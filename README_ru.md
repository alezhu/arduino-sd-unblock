[Read in English](README.md)

# SD Card Write Protection Tool (`sd-unblock`)

Скетч для Arduino и платформа для снятия / установки аппаратной защиты от записи на SD, MiniSD и MicroSD картах памяти любого объема и типа (SDSC, SDHC, SDXC).

Проект перенесен и расширен на основе консольной утилиты Linux [sdtool](https://github.com/BertoldVdb/sdtool) (автор Bertold Van den Bergh).

---

## 🚀 Особенности

- 🔍 **Проверка статуса защиты**: определение временной (Temporary) или постоянной (Permanent) блокировки карты от записи.
- 🔓 **Снятие защиты от записи**: снятие программных блокировок.
- 🔌 **Универсальность и автообнаружение**: поддержка работы как с аппаратным контактом присутствия карты, так и без него (через периодический опрос SPI).

---

## 🔌 Руководства по подключению плат

- 🟦 [Arduino Uno / Nano / Mega](docs/uno_ru.md)
- 🟧 [ESP8266 (NodeMCU / Wemos D1 Mini)](docs/esp8266_ru.md)
- 🟩 [ESP32 (Dev Module)](docs/esp32_ru.md)
- 🟦 [STM32 (Blue Pill F103C8)](docs/bluepill_ru.md)
- 🟪 [LGT8F328P (LogicGreen)](docs/lgt8f328p_ru.md)

---

## ⚙️ Аппаратное определение присутствия карты (Card Detect)

Подробное описание работы детектора вставки карты, особенностей линии `DAT1 (Пин 8)`, поведения разных типов SD-контроллеров и схемы подключения смотрите в отдельном руководстве:

👉 **[Аппаратное определение присутствия карты (Card Detect)](docs/card_detect_ru.md)**

---

## 🖥️ Использование

1. Загрузите скетч `sd-unblock.ino` в вашу плату.
2. Откройте **Serial Monitor** на скорости **9600 бод**.
3. Вставьте SD-карту. В консоли появится информация о статусе карты:

```text
--- SD Card Write Protection Tool (sd-unblock) ---
[+] SD Card connected!
[+] Card CSD: 005E00325B59A03B76DB7F800A4000EB.
[+] Write protection state: Temporary.

==============================================
SD Card Write Protection (CSD) Operations:
  1 or 'u' - Unlock card (disable write protection)
  2 or 'l' - Lock card (enable temporary write protection)
  3 or 'p' - Permlock card (enable permanent write protection)
  'r'      - Re-read CSD status
==============================================
Enter command:
```

4. Отправьте символ `1` или `u`, чтобы снять защиту от записи.

---

## 🛠️ Сборка через PlatformIO

Проект полностью настроен для сборки через PlatformIO. Скрипты сборки позволяют скомпилировать бинарный файл как для конкретной целевой платформы, так и для всех платформ одновременно.

* **Windows**:
  - Сборка для всех платформ: `build.cmd`
  - Сборка для конкретной платформы: `build.cmd uno` 
* **Linux / macOS**:
  - Сборка для всех платформ: `./build.sh`
  - Сборка для конкретной платформы: `./build.sh esp32`

Скомпилированные бинарные файлы прошивок автоматически сохраняются в папке `build_out/`.

Доступные варианты платформ дял сборки: `uno`, `esp8266`, `esp32`, `bluepill`, `lgt8f328p`

---

## ⚡ Загрузка готовых бинарников (Прошивка устройств)

Если у вас нет установленной среды разработки, вы можете прошить готовые бинарные файлы из папки `build_out/` или со страницы [GitHub Releases](../../releases).

### 1. Arduino Uno / Nano (`firmware_arduino_v*.hex`)
* **Через PlatformIO CLI**:
  ```bash
  pio run --environment uno --target upload
  ```
* **Через avrdude (командная строка)**:
  ```bash
  avrdude -c arduino -p m328p -P COM3 -b 115200 -U flash:w:build_out/firmware_arduino_v1.0.0.hex:i
  ```
* **Через графические утилиты**: [XLoader](http://www.russemotto.com/xloader/) (выберите плату Uno/Nano, COM-порт и файл `.hex`).

### 2. ESP8266 (NodeMCU / Wemos D1 Mini) (`firmware_esp8266_v*.bin`)
* **Через PlatformIO CLI**:
  ```bash
  pio run --environment esp8266 --target upload
  ```
* **Через esptool (Python)**:
  ```bash
  esptool.py --port COM3 --baud 460800 write_flash 0x00000 build_out/firmware_esp8266_v1.0.0.bin
  ```
* **Через графические утилиты**: NodeMCU PyFlasher или NodeMCU Flasher.

### 3. ESP32 Dev Module (`firmware_esp32_v*.bin`)
* **Через PlatformIO CLI**:
  ```bash
  pio run --environment esp32 --target upload
  ```
* **Через esptool (Python)**:
  ```bash
  esptool.py --port COM3 --baud 921600 write_flash 0x10000 build_out/firmware_esp32_v1.0.0.bin
  ```
* **Через графические утилиты**: Flash Download Tools (Espressif).

### 4. STM32 Blue Pill F103C8 (`firmware_stm32_v*.bin` / `.hex`)
* **Через PlatformIO CLI**:
  ```bash
  pio run --environment bluepill --target upload
  ```
* **Через ST-Link (SWD)**:
  Используйте программу **STM32CubeProgrammer** или **ST-LINK Utility**.
* **Через UART (USB-TTL программатор)**:
  Установите джампер `BOOT0 = 1` и прошейте через **stm32flash**:
  ```bash
  stm32flash -w build_out/firmware_stm32_v1.0.0.bin -v -g 0x0 COM3
  ```

### 5. LGT8F328P (`firmware_lgt8f328p_v*.hex`)
* **Через PlatformIO CLI**:
  ```bash
  pio run --environment lgt8f328p --target upload
  ```
* **Через avrdude**:
  ```bash
  avrdude -c arduino -p lgt8f328p -P COM3 -b 57600 -U flash:w:build_out/firmware_lgt8f328p_v1.0.0.hex:i
  ```

---

## 📜 Лицензия

Проект распространяется под лицензией MIT.
