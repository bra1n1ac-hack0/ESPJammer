# ESPJAMMER v1.0

ESP32 pentesting tool firmware with OLED menu UI, WiFi scanning, packet sniffing, and BLE modules.

## Web Flasher
Flash directly from your browser — no installs needed:
> https://YOURUSERNAME.github.io/ESPJammer

## Features
- Scrollable OLED menu system
- WiFi AP scanning
- Promiscuous packet sniffing
- Beacon spam
- BLE device scanning
- Apple BLE spam

## Hardware
| Component | Details |
|---|---|
| Board | ESP32 WROOM-32 |
| Display | 0.96" OLED SSD1306 |
| Buttons | 4x tactile (UP/DOWN/SEL/BACK) |

## Wiring
| OLED | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

| Button | GPIO |
|---|---|
| UP | 12 |
| DOWN | 13 |
| SELECT | 14 |
| BACK | 27 |

## Build & Flash
1. Install [PlatformIO](https://platformio.org/)
2. Clone this repo
3. Open in VS Code with PlatformIO extension
4. Run `pio run --target upload`

## Legal
For authorized security research and educational use only.
Only use on networks and devices you own or have explicit written permission to test.

## License
MIT License
