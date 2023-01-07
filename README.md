
# ESP32 Internet Radio

This is an internet radio based on esp32-wroom, max98357 DAC and 128x160 tft.


## Features

- Playing any mp3 stream
- Connecting to wifi by creating a hotspot and hosting a www panel
- Menus navigated by a rotary encoder
- Saving last station and volume to eeprom


## Installation

List of needed libraries:
- https://github.com/madhephaestus/ESP32Encoder
- https://github.com/tzapu/WiFiManager
- https://github.com/adafruit/Adafruit-GFX-Library
- https://github.com/adafruit/Adafruit-ST7735-Library
- https://github.com/schreibfaul1/ESP32-audioI2S
- https://github.com/arduino-libraries/NTPClient
- https://github.com/PaulStoffregen/Time

You will also need to add these into additional boards in preferences:
```bash
http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
    