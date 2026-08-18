/*
  SD Card Write Protection Detector & Unblocker (CSD Register Tool)

  Detects if write protection (Permanent or Temporary) is set in the CSD register
  of a connected SD / MicroSD card and provides an interactive Serial Monitor interface
  to unlock or lock the card.
  
  Logic adapted from Linux 'sdtool' (by Bertold Van den Bergh) for Arduino
  using standard SPI.h library without filesystem dependencies.
*/

#include <SPI.h>

// CS Pin for SD Card module depending on board architecture
#if defined(ESP8266)
const uint8_t chipSelect = D8;           // GPIO 15 (D8 on NodeMCU / Wemos D1 Mini)
const uint8_t HWCD_ANALOG_CARD_PIN = A0; // ESP8266: pin A0 (0..1.0V / 0..3.3V)
#elif defined(ESP32)
const uint8_t chipSelect = 5;            // GPIO 5 (VSPI CS on ESP32)
const uint8_t HWCD_ANALOG_CARD_PIN = 34; // ESP32: GPIO34 (Analog input ADC1)
#elif defined(ARDUINO_ARCH_STM32)
const uint8_t chipSelect = PA4;           // PA4 on STM32 (Blue Pill)
const uint8_t HWCD_ANALOG_CARD_PIN = PA0; // STM32: analog pin PA0
#else
const uint8_t chipSelect = SS;           // Pin 10 (SS/CS) for Arduino Uno / Nano / Mega
const uint8_t HWCD_ANALOG_CARD_PIN = A0;
#endif

// SD Card Command Opcodes
const uint8_t CMD0   = 0;   // GO_IDLE_STATE
const uint8_t CMD8   = 8;   // SEND_IF_COND
const uint8_t CMD9   = 9;   // SEND_CSD
const uint8_t CMD27  = 27;  // PROGRAM_CSD
const uint8_t CMD55  = 55;  // APP_CMD
const uint8_t ACMD41 = 41;  // SD_SEND_OP_COND

// SPI clock speed settings
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_STM32)
SPISettings spiSettingsInit(250000, MSBFIRST, SPI_MODE0);
SPISettings spiSettingsOp(4000000, MSBFIRST, SPI_MODE0);
#else
SPISettings spiSettingsInit(250000, MSBFIRST, SPI_MODE0);
SPISettings spiSettingsOp(4000000, MSBFIRST, SPI_MODE0);
#endif

// Hardware Card Detector State Machine
enum HardwareCardDetectorUsing
{
    HWCD_UNDEFINED, // Unknown if analog pin is connected (auto-calibrating)
    HWCD_USE,       // Analog pin physically connected, tested and active
    HWCD_DONT_USE   // Analog pin disconnected or unstable (SPI polling fallback)
};

typedef struct
{
    HardwareCardDetectorUsing use;
    int adcLast = 0;
    int adcWithCard = 0;
    int adcWithoutCard = 63;
} HardwareCardDetector;

HardwareCardDetector hardwareCardDetector;
unsigned long lastCheckTime = 0;
bool cardWasConnected = false;
bool cardCSDLoaded = false;
uint8_t currentCSD[16];

// Low-level SPI helper functions
void sdSelect()
{
    digitalWrite(chipSelect, LOW);
}

void sdDeselect()
{
    digitalWrite(chipSelect, HIGH);
    SPI.transfer(0xFF);
}

// CRC7 calculation (adapted from sdtool crc.c)
uint8_t crc7AddWord(uint8_t crc, uint8_t word, uint8_t len)
{
    uint8_t i;
    uint8_t bit;
    for (i = 0; i < len; i++)
    {
        bit = ((word & 0x80) > 0);
        bit ^= (crc >> 6);
        if (bit)
        {
            crc ^= 0x04;
        }
        crc <<= 1;
        crc &= 0x7F;
        crc |= bit;
        word <<= 1;
    }
    return crc;
}

void sdCSDSetCRC(uint8_t *csd)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < 15; i++)
    {
        crc = crc7AddWord(crc, csd[i], 8);
    }
    csd[15] = (crc << 1) | 1;
}

uint8_t sdSendCommand(uint8_t cmd, uint32_t arg)
{
    sdSelect();

    uint8_t buf[5];
    buf[0] = 0x40 | cmd;
    buf[1] = (uint8_t)(arg >> 24);
    buf[2] = (uint8_t)(arg >> 16);
    buf[3] = (uint8_t)(arg >> 8);
    buf[4] = (uint8_t)arg;

    uint8_t crc = 0;
    for (int i = 0; i < 5; i++)
    {
        crc = crc7AddWord(crc, buf[i], 8);
    }
    uint8_t crcByte = (crc << 1) | 1;

    for (int i = 0; i < 5; i++)
    {
        SPI.transfer(buf[i]);
    }
    SPI.transfer(crcByte);

    uint8_t r1 = 0xFF;
    for (int i = 0; i < 10; i++)
    {
        r1 = SPI.transfer(0xFF);
        if ((r1 & 0x80) == 0)
        {
            break;
        }
    }
    return r1;
}

bool sdInitCardSPI()
{
    SPI.beginTransaction(spiSettingsInit);

    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
    delay(10);

    for (int i = 0; i < 10; i++)
    {
        SPI.transfer(0xFF);
    }

    uint8_t r1 = 0xFF;
    for (int i = 0; i < 10; i++)
    {
        r1 = sdSendCommand(CMD0, 0);
        sdDeselect();
        if (r1 == 0x01)
        {
            break;
        }
        delay(10);
    }

    if (r1 != 0x01)
    {
        SPI.endTransaction();
        return false;
    }

    r1 = sdSendCommand(CMD8, 0x1AA);
    if (r1 == 0x01)
    {
        uint32_t r7 = 0;
        for (int i = 0; i < 4; i++)
        {
            r7 = (r7 << 8) | SPI.transfer(0xFF);
        }
        sdDeselect();
        if ((r7 & 0xFFF) != 0x1AA)
        {
            SPI.endTransaction();
            return false;
        }
    }
    else
    {
        sdDeselect();
    }

    unsigned long startTime = millis();
    bool ready = false;
    while (millis() - startTime < 2000)
    {
        sdSendCommand(CMD55, 0);
        sdDeselect();
        r1 = sdSendCommand(ACMD41, 0x40000000);
        sdDeselect();
        if (r1 == 0x00)
        {
            ready = true;
            break;
        }
        delay(20);
    }

    SPI.endTransaction();
    return ready;
}

bool sdReadCSD(uint8_t *csd)
{
    SPI.beginTransaction(spiSettingsOp);

    uint8_t r1 = sdSendCommand(CMD9, 0);
    if (r1 != 0x00)
    {
        sdDeselect();
        SPI.endTransaction();
        return false;
    }

    unsigned long startTime = millis();
    uint8_t token = 0xFF;
    while (millis() - startTime < 500)
    {
        token = SPI.transfer(0xFF);
        if (token == 0xFE)
        {
            break;
        }
    }

    if (token != 0xFE)
    {
        sdDeselect();
        SPI.endTransaction();
        return false;
    }

    for (int i = 0; i < 16; i++)
    {
        csd[i] = SPI.transfer(0xFF);
    }

    SPI.transfer(0xFF);
    SPI.transfer(0xFF);

    sdDeselect();
    SPI.endTransaction();
    return true;
}

bool sdWriteCSD(const uint8_t *csd)
{
    SPI.beginTransaction(spiSettingsOp);

    uint8_t r1 = sdSendCommand(CMD27, 0);
    if (r1 != 0x00)
    {
        sdDeselect();
        SPI.endTransaction();
        return false;
    }

    SPI.transfer(0xFE);

    for (int i = 0; i < 16; i++)
    {
        SPI.transfer(csd[i]);
    }

    SPI.transfer(0xFF);
    SPI.transfer(0xFF);

    uint8_t dataResponse = SPI.transfer(0xFF);
    bool accepted = ((dataResponse & 0x1F) == 0x05);

    if (accepted)
    {
        unsigned long startTime = millis();
        while (SPI.transfer(0xFF) == 0x00)
        {
            if (millis() - startTime > 2000)
            {
                accepted = false;
                break;
            }
            yield();
        }
    }

    sdDeselect();
    SPI.endTransaction();
    return accepted;
}

int displayWpState(const uint8_t *csd)
{
    int retVal;
    Serial.print(F("[+] Write protection state: "));
    if (csd[14] & 0x20)
    {
        Serial.println(F("Permanent"));
        retVal = -1;
    }
    else if (csd[14] & 0x10)
    {
        Serial.println(F("Temporary"));
        retVal = -2;
    }
    else
    {
        Serial.println(F("Off"));
        retVal = -3;
    }
    return retVal;
}

void permlockWarning(const uint8_t *csd)
{
    if (csd[14] & 0x20)
    {
        Serial.println(F("[?] Card is permanently locked. I will try to clear the flag, but it will likely fail."));
    }
}

void printMenu()
{
    Serial.println(F("\n=============================================="));
    Serial.println(F("SD Card Write Protection (CSD) Operations:"));
    Serial.println(F("  1 or 'u' - Unlock card (disable write protection)"));
    Serial.println(F("  2 or 'l' - Lock card (enable temporary write protection)"));
    Serial.println(F("  3 or 'p' - Permlock card (enable permanent write protection)"));
    Serial.println(F("  'r'      - Re-read CSD status"));
    Serial.println(F("=============================================="));
    Serial.print(F("Enter command: "));
}

void processCard()
{
    if (!sdReadCSD(currentCSD))
    {
        Serial.println(F("Error: Could not read SD Card CSD register!"));
        cardCSDLoaded = false;
        return;
    }

    cardCSDLoaded = true;

    Serial.print(F("[+] Card CSD: "));
    for (int i = 0; i < 16; i++)
    {
        if (currentCSD[i] < 0x10)
        {
            Serial.print('0');
        }
        Serial.print(currentCSD[i], HEX);
    }
    Serial.println(F("."));

    displayWpState(currentCSD);
    printMenu();
}

bool checkAdcCardPresent(int adcVal)
{
    return (abs(adcVal - hardwareCardDetector.adcWithCard) < abs(adcVal - hardwareCardDetector.adcWithoutCard));
}

bool isSpiCardConnected()
{
    SPI.begin();
    return sdInitCardSPI();
}

void enableHardwareCardDetector()
{
    hardwareCardDetector.use = HWCD_USE;
    Serial.println(F("Use hardware card detector"));
}

void disableHardwareCardDetector()
{
    hardwareCardDetector.use = HWCD_DONT_USE;
    Serial.println(F("Don't use hardware card detector"));
}

bool isCardConnected()
{
    unsigned long currentMillis = millis();
    auto needRecheck = (currentMillis - lastCheckTime >= 1000);
    if (needRecheck)
    {
        lastCheckTime = currentMillis;
    }

    switch (hardwareCardDetector.use)
    {
    case HWCD_UNDEFINED:
        if (needRecheck)
        {
            bool spiConnected = isSpiCardConnected();
            if (spiConnected != cardWasConnected)
            {
                int currentAdc = analogRead(HWCD_ANALOG_CARD_PIN);
                int maxVal = max(hardwareCardDetector.adcLast, currentAdc);
                int adcDelta = abs(currentAdc - hardwareCardDetector.adcLast);
                bool isSignificantChange = (maxVal >= 20) && (adcDelta >= (maxVal / 2));
                if (isSignificantChange)
                {
                    if (spiConnected)
                    {
                        hardwareCardDetector.adcWithCard = currentAdc;
                        hardwareCardDetector.adcWithoutCard = hardwareCardDetector.adcLast;
                    }
                    else
                    {
                        hardwareCardDetector.adcWithCard = hardwareCardDetector.adcLast;
                        hardwareCardDetector.adcWithoutCard = currentAdc;
                    }
                    hardwareCardDetector.adcLast = currentAdc;
                    enableHardwareCardDetector();
                }
                else
                {
                    disableHardwareCardDetector();
                }
            }
            return spiConnected;
        }
        break;

    case HWCD_USE:
    {
        int currentAdc = analogRead(HWCD_ANALOG_CARD_PIN);
        return checkAdcCardPresent(currentAdc);
    }
    break;

    case HWCD_DONT_USE:
        if (needRecheck)
        {
            return isSpiCardConnected();
        }
        break;

    default:
        break;
    }

    return cardWasConnected;
}

void onCardConnected()
{
    Serial.println(F("\n[+] SD Card connected!"));
    SPI.begin();
    if (!sdInitCardSPI())
    {
        Serial.println(F("Error: Could not initialize SD Card over SPI!"));
        cardCSDLoaded = false;
        return;
    }
    processCard();
}

void onCardDisconnected()
{
    Serial.println(F("\n[-] SD Card removed."));
    cardCSDLoaded = false;
    Serial.println(F("Waiting for SD Card to be connected..."));
}

void printADC(int value)
{
    Serial.print(F("ADC A0: "));
    Serial.println(value);
}

void handleSerialInput()
{
    if (!Serial.available() || !cardWasConnected || !cardCSDLoaded)
    {
        return;
    }

    char cmd = Serial.read();
    if (cmd == '\r' || cmd == '\n' || cmd == ' ')
    {
        return;
    }

    Serial.println(cmd);

    uint8_t oldCSD14 = currentCSD[14];

    if (cmd == '1' || cmd == 'u' || cmd == 'U')
    {
        Serial.println(F("[+] Action: Unlock (Disable write protection)"));
        permlockWarning(currentCSD);
        currentCSD[14] &= ~0x30;
    }
    else if (cmd == '2' || cmd == 'l' || cmd == 'L')
    {
        Serial.println(F("[+] Action: Lock (Enable temporary write protection)"));
        permlockWarning(currentCSD);
        currentCSD[14] &= ~0x30;
        currentCSD[14] |= 0x10;
    }
    else if (cmd == '3' || cmd == 'p' || cmd == 'P')
    {
        Serial.println(F("[+] Action: Permlock (Enable permanent write protection)"));
        Serial.println(F("WARNING: Permanent lock CANNOT be undone on hardware level!"));
        currentCSD[14] &= ~0x30;
        currentCSD[14] |= 0x20;
    }
    else if (cmd == 'r' || cmd == 'R')
    {
        Serial.println(F("[+] Action: Re-reading CSD..."));
        processCard();
        return;
    }
    else
    {
        Serial.println(F("Invalid command!"));
        printMenu();
        return;
    }

    if (currentCSD[14] != oldCSD14)
    {
        Serial.println(F("[+] Recalculating CRC7 and writing CSD..."));
        sdCSDSetCRC(currentCSD);
        if (sdWriteCSD(currentCSD))
        {
            Serial.println(F("[+] Write CSD command accepted!"));
        }
        else
        {
            Serial.println(F("[-] Failed to write CSD!"));
        }
        delay(100);
        processCard();
    }
    else
    {
        Serial.println(F("[+] CSD unchanged."));
        displayWpState(currentCSD);
        printMenu();
    }
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        yield();
    }

    Serial.println(F("--- SD Card Write Protection Tool (sd-unblock) ---"));

#if defined(__AVR__)
    pinMode(SS, OUTPUT);
    digitalWrite(SS, HIGH);
#endif

    pinMode(HWCD_ANALOG_CARD_PIN, INPUT);
    bool spiConnected = isSpiCardConnected();
    int initialAdc = hardwareCardDetector.adcLast = analogRead(HWCD_ANALOG_CARD_PIN);

    if (spiConnected)
    {
        cardWasConnected = true;
        hardwareCardDetector.adcWithCard = initialAdc;
        printADC(initialAdc);
        onCardConnected();
    }
    else
    {
        cardWasConnected = false;
        hardwareCardDetector.adcWithoutCard = initialAdc;
        printADC(initialAdc);
        onCardDisconnected();
    }

    Serial.flush();
}

void loop()
{
    bool connected = isCardConnected();
    if (connected != cardWasConnected)
    {
        if (connected)
        {
            onCardConnected();
        }
        else
        {
            onCardDisconnected();
        }
        cardWasConnected = connected;
    }

    handleSerialInput();
}
