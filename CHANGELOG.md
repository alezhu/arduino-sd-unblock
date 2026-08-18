# Changelog

All notable changes to the SD Card Write Protection Tool (`sd-unblock`) project will be documented in this file.

## [1.0.0] - 2026-08-18

### Added
- Initial release of `sd-unblock` firmware for Arduino (Uno/Nano), ESP8266, ESP32, STM32 (Blue Pill), and LGT8F328P.
- Low-level SD card SPI protocol implementation using standard `<SPI.h>` library without filesystem dependencies.
- CSD register reading (CMD9) and Write Protection status detection (`PERM_WRITE_PROTECT` and `TMP_WRITE_PROTECT`).
- CSD register writing (CMD27) with CRC7 calculation adapted from `sdtool`.
- Interactive Serial Monitor interface (9600 baud) for unlocking, locking, and status re-reading.
- Multi-platform PlatformIO configuration (`platformio.ini`).
- Automated build scripts (`build.cmd` and `build.sh`) for generating release binaries.
- GitHub Actions workflow (`.github/workflows/release.yml`) for automated release builds on tag push.
- Comprehensive connection documentation with SVG wiring diagrams for all supported boards.
