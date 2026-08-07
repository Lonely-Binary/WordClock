# Word Clock Tutorial — Arduino/C++ Track

A beginner-friendly path through the Lonely Binary ESP32-S3 word clock hardware, written in Arduino/C++ with [FastLED](https://github.com/FastLED/FastLED). One small concept per lesson, each in its own folder with a full sketch and a line-by-line explanation.

Prefer Python? See the [MicroPython track](../MicroPython/README.md) instead — same 7 lessons, same hardware, different language.

## Prerequisites

- **Arduino IDE** with the ESP32 board package installed (Boards Manager → search "esp32", install the Espressif package).
- Board selection: **ESP32S3 Dev Module** (`esp32:esp32:esp32s3`).
- **[FastLED](https://github.com/FastLED/FastLED) 3.6.0+** installed via Library Manager — older versions lack solid ESP32-S3 support.
- The word clock hardware: 11×10 WS2812 letter grid + base, snapped onto the Lonely Binary ESP32-S3 dev board.

## Hardware recap

- **GPIO 8** — WS2812 data line for the whole panel (110 letter LEDs + 4 corner "minute" dot LEDs = 114 total).
- **GPIO 42** — COLOR button (active-high).
- **GPIO 41** — EFFECT button (active-high).
- **Two Type-C ports, two different jobs**: the port on the **dev board itself** is for flashing firmware only. The port on the **base** powers everything, including the LED grid — flash over the dev board's port, then power from the base's port to actually see LEDs light up. See the [root README](../../README.md#️-power--flashing--please-read) for the full explanation; this trips up almost everyone on their first flash.

## Lessons

| # | Lesson | What you'll learn |
|---|---|---|
| 1 | [Single Pixel](01_SinglePixel/README.md) | FastLED basics — light exactly one LED, confirm your hardware works |
| 2 | [Light a Word](02_LightAWord/README.md) | Row/column grid mapping, lighting a whole word |
| 3 | [Colors and Grid](03_ColorsAndGrid/README.md) | Fill the whole panel, `CRGB` vs `CHSV`, simple animation with `loop()` |
| 4 | [Buttons](04_Buttons/README.md) | Reading the onboard buttons, debouncing, edge detection |
| 5 | [WiFi Connect](05_WiFiConnect/README.md) | Getting the board online in station mode |
| 6 | [NTP Time](06_NTPTime/README.md) | Syncing real time over the internet |
| 7 | [Full Word Clock](07_FullWordClock/README.md) | Capstone — combine it all into the simplest sketch that tells time |
| 8 | [OTA Updates](08_OTAUpdates/README.md) *(bonus)* | Upload new firmware over WiFi instead of USB |

Work through 1–7 in order — each one builds on ideas from the last. Lesson 8 is a standalone bonus once you're comfortable with WiFi (Lesson 5).

## After the tutorial

Once you've built Lesson 7, compare it against [`WordClock/WordClock.ino`](../../WordClock/WordClock.ino) to see what the full firmware adds (button-driven colour modes, random light effects, self-serve WiFi/timezone setup, OTA). From there, you have everything you need to design your own panel — new vocabulary, new triggers, new data source — on the same 11×10 grid.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
