[Читать на русском языке](lgt8f328p_ru.md)

# Connecting SD / MicroSD Module to LGT8F328P

The board based on the **LGT8F328P** (LogicGreen) microcontroller is **completely identical to Arduino Nano / Uno** in pinout, SPI hardware, and 5V logic levels.

## Comparison and Differences from Arduino Uno / Nano

| Feature | Arduino Uno / Nano (ATmega328P) | LGT8F328P |
| :--- | :--- | :--- |
| **SPI Pinout (CS, MOSI, MISO, SCK)** | D10, D11, D12, D13 (1:1 identical) | D10, D11, D12, D13 (1:1 identical) |
| **Default Logic Level** | 5 V | 5 V (when powered from 5 V) |
| **Max Clock Frequency** | 16 MHz | 32 MHz |
| **ADC Resolution** | 10-bit (0..1023) | 10-bit / 12-bit (10-bit mode used in sketch) |
| **3.3V Operation** | Requires separate 3.3V board | Supports onboard 3.3V switching |

> [!NOTE]
> Wiring diagrams, resistor values ($1\text{ k}\Omega / 2\text{ k}\Omega$), and level conversion options using the **74HC125N** chip for the LGT8F328P are **100% identical to Arduino Uno / Nano boards**.
> Detailed diagrams are provided in the guide: 👉 **[Connecting to Arduino Uno / Nano](uno.md)**.
