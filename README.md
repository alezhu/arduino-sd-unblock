[Читать на русском языке](README_ru.md)

# SD Card Write Protection Tool (`sd-unblock`)

Arduino sketch and platform for clearing / setting hardware write protection on SD, MiniSD, and MicroSD memory cards of any capacity and type (SDSC, SDHC, SDXC).

Ported and expanded from the Linux [sdtool](https://github.com/BertoldVdb/sdtool) console utility (by Bertold Van den Bergh).

---

## 🚀 Features

- 🔍 **Write Protection Status Check**: detection of Temporary or Permanent write protection flags.
- 🔓 **Unlock Write Protection**: clearing software lock flags.
- 🔌 **Versatility & Auto-Detection**: support for working both with a hardware card presence contact and without it (via periodic SPI polling).

---

## 🔌 Board Connection Guides

- 🟦 [Arduino Uno / Nano / Mega](docs/uno.md)
- 🟧 [ESP8266 (NodeMCU / Wemos D1 Mini)](docs/esp8266.md)
- 🟩 [ESP32 (Dev Module)](docs/esp32.md)
- 🟦 [STM32 (Blue Pill F103C8)](docs/bluepill.md)
- 🟪 [LGT8F328P (LogicGreen)](docs/lgt8f328p.md)

---

## ⚙️ Hardware Card Detection (Card Detect)

For a detailed description of card insertion detector operation, `DAT1 (Pin 8)` line features, card controller variations, and connection diagrams, see the dedicated guide:

👉 **[Hardware Card Detection (Card Detect)](docs/card_detect.md)**

---

## 🖥️ Usage

1. Upload the `sd-unblock.ino` sketch to your board.
2. Open **Serial Monitor** at **9600 baud**.
3. Insert the SD card. Console will display card status information:

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

4. Send character `1` or `u` to remove write protection.

---

## 🛠️ Building via PlatformIO

The project is fully configured for building via PlatformIO. Build scripts allow compiling a binary file both for a specific target platform and for all platforms simultaneously.

* **Windows**:
  - Build for all platforms: `build.cmd`
  - Build for a specific platform: `build.cmd uno`
* **Linux / macOS**:
  - Build for all platforms: `./build.sh`
  - Build for a specific platform: `./build.sh esp32`

Compiled firmware binaries are automatically saved in the `build_out/` folder.

Available build target platforms: `uno`, `esp8266`, `esp32`, `bluepill`, `lgt8f328p`

---

## ⚡ Flashing Precompiled Binaries (Device Flashing)

If you do not have an installed development environment, you can flash ready-made binary files from the `build_out/` folder or from the [GitHub Releases](../../releases) page.

### 1. Arduino Uno / Nano (`firmware_arduino_v*.hex`)
* **Via PlatformIO CLI**:
  ```bash
  pio run --environment uno --target upload
  ```
* **Via avrdude (command line)**:
  ```bash
  avrdude -c arduino -p m328p -P COM3 -b 115200 -U flash:w:build_out/firmware_arduino_v1.0.0.hex:i
  ```
* **Via GUI utilities**: [XLoader](http://www.russemotto.com/xloader/) (select Uno/Nano board, COM port, and `.hex` file).

### 2. ESP8266 (NodeMCU / Wemos D1 Mini) (`firmware_esp8266_v*.bin`)
* **Via PlatformIO CLI**:
  ```bash
  pio run --environment esp8266 --target upload
  ```
* **Via esptool (Python)**:
  ```bash
  esptool.py --port COM3 --baud 460800 write_flash 0x00000 build_out/firmware_esp8266_v1.0.0.bin
  ```
* **Via GUI utilities**: NodeMCU PyFlasher or NodeMCU Flasher.

### 3. ESP32 Dev Module (`firmware_esp32_v*.bin`)
* **Via PlatformIO CLI**:
  ```bash
  pio run --environment esp32 --target upload
  ```
* **Via esptool (Python)**:
  ```bash
  esptool.py --port COM3 --baud 921600 write_flash 0x10000 build_out/firmware_esp32_v1.0.0.bin
  ```
* **Via GUI utilities**: Flash Download Tools (Espressif).

### 4. STM32 Blue Pill F103C8 (`firmware_stm32_v*.bin` / `.hex`)
* **Via PlatformIO CLI**:
  ```bash
  pio run --environment bluepill --target upload
  ```
* **Via ST-Link (SWD)**:
  Use **STM32CubeProgrammer** or **ST-LINK Utility**.
* **Via UART (USB-TTL programmer)**:
  Set jumper `BOOT0 = 1` and flash via **stm32flash**:
  ```bash
  stm32flash -w build_out/firmware_stm32_v1.0.0.bin -v -g 0x0 COM3
  ```

### 5. LGT8F328P (`firmware_lgt8f328p_v*.hex`)
* **Via PlatformIO CLI**:
  ```bash
  pio run --environment lgt8f328p --target upload
  ```
* **Via avrdude**:
  ```bash
  avrdude -c arduino -p lgt8f328p -P COM3 -b 57600 -U flash:w:build_out/firmware_lgt8f328p_v1.0.0.hex:i
  ```

---

## 📜 License

Distributed under the MIT License.
