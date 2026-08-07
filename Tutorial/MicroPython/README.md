# Word Clock Tutorial — MicroPython Track

The same 7-lesson progression as the [Arduino/C++ track](../Arduino/README.md), rewritten for **MicroPython** — for readers who'd rather write Python than C++ on this hardware.

## Prerequisites

- **MicroPython firmware flashed onto the ESP32-S3.** Download the ESP32-S3 build from [micropython.org/download/](https://micropython.org/download/) and flash it with `esptool`:
  ```bash
  pip install esptool
  esptool.py --chip esp32s3 --port /dev/tty.usbmodemXXXX erase_flash
  esptool.py --chip esp32s3 --port /dev/tty.usbmodemXXXX write_flash -z 0x0 ESP32_GENERIC_S3-*.bin
  ```
- **[Thonny](https://thonny.org/)** (or `mpremote`/`rshell`) to run scripts on the board and use its Python REPL. In Thonny: Tools → Options → Interpreter → "MicroPython (ESP32)", then pick the board's serial port.
- The word clock hardware: 11×10 WS2812 letter grid + base, snapped onto the Lonely Binary ESP32-S3 dev board.

Each lesson's `main.py` can either be run directly from Thonny ("Run current script"), or copied onto the board's filesystem as `main.py` so it runs automatically on boot.

## Hardware recap

- **GPIO 8** — WS2812 data line for the whole panel (110 letter LEDs + 4 corner "minute" dot LEDs = 114 total), driven by the built-in `neopixel` module.
- **GPIO 42** — COLOR button (active-high).
- **GPIO 41** — EFFECT button (active-high).
- **Two Type-C ports, two different jobs**: the port on the **dev board itself** is for flashing firmware and running scripts. The port on the **base** powers everything, including the LED grid — flash/run over the dev board's port, then power from the base's port to actually see LEDs light up. See the [root README](../../README.md#️-power--flashing--please-read) for the full explanation.

## Lessons

| # | Lesson | What you'll learn |
|---|---|---|
| 1 | [Single Pixel](01_SinglePixel/README.md) | `neopixel` module basics — light exactly one LED, confirm your hardware works |
| 2 | [Light a Word](02_LightAWord/README.md) | Row/column grid mapping, lighting a whole word |
| 3 | [Colors and Grid](03_ColorsAndGrid/README.md) | Fill the whole panel, RGB tuples, a hand-rolled HSV rainbow |
| 4 | [Buttons](04_Buttons/README.md) | Reading the onboard buttons, debouncing, edge detection |
| 5 | [WiFi Connect](05_WiFiConnect/README.md) | Getting the board online in station mode with `network` |
| 6 | [NTP Time](06_NTPTime/README.md) | Syncing real time over the internet with `ntptime` |
| 7 | [Full Word Clock](07_FullWordClock/README.md) | Capstone — combine it all into the simplest script that tells time |

Work through them in order — each one builds on ideas from the last.

## How this differs from the Arduino/C++ track

- FastLED's `CRGB`/`CHSV` become plain `(r, g, b)` tuples plus a small hand-written `hsv_to_rgb()` helper (Lesson 3) — MicroPython's `neopixel` module doesn't have a built-in HSV type.
- `configTzTime()`'s automatic timezone/DST handling doesn't exist in MicroPython; `ntptime` only gives you UTC, so the clock applies a fixed `UTC_OFFSET_SEC` by hand (Lessons 6–7).
- The full-featured firmware (button-driven colour modes, light effects) at [`WordClock/WordClock.ino`](../../WordClock/README.md) is Arduino/C++ only — this track's capstone (Lesson 7) is intentionally the simple end state and isn't ported further.

## After the tutorial

Once you've built Lesson 7, you have everything you need to design your own panel in MicroPython — new vocabulary, new triggers, new data source — on the same 11×10 grid.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
