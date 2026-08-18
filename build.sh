#!/usr/bin/env bash
set -e

if [ ! -f version.txt ]; then
    echo "Error: version.txt not found!"
    exit 1
fi

VERSION=$(cat version.txt | tr -d '\r\n ')
TARGET="${1:-all}"

echo "==================================================="
echo "Building SD Card Write Protection Tool (sd-unblock) v${VERSION}"
echo "Target platform: ${TARGET}"
echo "==================================================="

mkdir -p build_out

PIO_CMD="pio"
if ! command -v pio &> /dev/null; then
    if [ -f "$HOME/.platformio/penv/bin/pio" ]; then
        PIO_CMD="$HOME/.platformio/penv/bin/pio"
    fi
fi

build_uno() {
    echo -e "\nBuilding for Arduino (Uno)..."
    $PIO_CMD run --environment uno
    cp .pio/build/uno/firmware.hex "build_out/firmware_arduino_v${VERSION}.hex"
}

build_esp8266() {
    echo -e "\nBuilding for ESP8266..."
    $PIO_CMD run --environment esp8266
    cp .pio/build/esp8266/firmware.bin "build_out/firmware_esp8266_v${VERSION}.bin"
}

build_esp32() {
    echo -e "\nBuilding for ESP32..."
    $PIO_CMD run --environment esp32
    cp .pio/build/esp32/firmware.bin "build_out/firmware_esp32_v${VERSION}.bin"
}

build_bluepill() {
    echo -e "\nBuilding for STM32 (Blue Pill)..."
    $PIO_CMD run --environment bluepill
    if [ -f ".pio/build/bluepill/firmware.bin" ]; then
        cp .pio/build/bluepill/firmware.bin "build_out/firmware_stm32_v${VERSION}.bin"
    elif [ -f ".pio/build/bluepill/firmware.hex" ]; then
        cp .pio/build/bluepill/firmware.hex "build_out/firmware_stm32_v${VERSION}.hex"
    fi
}

build_lgt8f328p() {
    echo -e "\nBuilding for LGT8F328P..."
    $PIO_CMD run --environment lgt8f328p
    cp .pio/build/lgt8f328p/firmware.hex "build_out/firmware_lgt8f328p_v${VERSION}.hex"
}

case "$TARGET" in
    uno)
        build_uno
        ;;
    esp8266)
        build_esp8266
        ;;
    esp32)
        build_esp32
        ;;
    bluepill)
        build_bluepill
        ;;
    lgt8f328p)
        build_lgt8f328p
        ;;
    all)
        build_uno
        build_esp8266
        build_esp32
        build_bluepill
        build_lgt8f328p
        ;;
    *)
        echo "Error: Unknown platform '$TARGET'!"
        echo "Supported platforms: uno, esp8266, esp32, bluepill, lgt8f328p, all"
        exit 1
        ;;
esac

echo -e "\n==================================================="
echo "Build completed successfully! Output files in build_out/:"
ls -lh build_out/firmware_*
echo "==================================================="
