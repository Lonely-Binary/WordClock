# Word Clock Tutorial — Arduino/C++ Track

*The blueprint behind ESP32-S3 Blueprints No.01.*

Three lessons covering what's specific to **this** panel: its 11×10 letter grid, its two buttons, and the logic that turns a time into an English sentence.

Prefer Python? See the [MicroPython track](../MicroPython/README.md).

## Do this first

These lessons assume you can already get an ESP32-S3 online and telling the time. That groundwork isn't repeated here — it lives in **[ESP32-Arduino-Tutorials](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials)**, shared by every Blueprint in the series.

Work through these four (about 45 minutes), then come back:

| | Lesson | Why you need it |
|---|---|---|
| 1 | [hello-world](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/getting-started/hello-world) | Uploading, Serial, and the USB CDC setting that otherwise leaves your Serial Monitor blank |
| 2 | [colour-and-brightness](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/leds/colour-and-brightness) | FastLED, `CRGB` vs `CHSV` — the panel is 114 of these |
| 3 | [station-mode](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/wifi/station-mode) | Getting on WiFi |
| 4 | [ntp-and-timezones](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/time/ntp-and-timezones) | Real local time — a clock is not much use without it |

Already comfortable with those? Skip straight to Lesson 1 below.

## Prerequisites

- **Arduino IDE** with the ESP32 board package, board set to **ESP32S3 Dev Module**
- **Tools → USB CDC On Boot → Enabled** (or the Serial Monitor stays blank)
- **[FastLED](https://github.com/FastLED/FastLED) 3.6.0+**
- Your assembled Word Clock: the 11×10 WS2812 grid on its base

## Hardware recap

- **GPIO 8** — data line for the whole panel: 110 letter LEDs + 4 corner "minute" dots = 114
- **GPIO 42** — COLOR button (active-high)
- **GPIO 41** — EFFECT button (active-high)
- **Two Type-C ports, two jobs.** The dev board's port flashes; the **base's** port powers the LED grid. A dark panel when powered from the dev board is expected — see the [root README](../../README.md#️-power--flashing--please-read).

## Lessons

| # | Lesson | What you'll learn |
|---|---|---|
| 1 | [Light a Word](01_LightAWord/README.md) | The panel as an 11×10 grid, `XY()` mapping, lighting whole words |
| 2 | [Buttons](02_Buttons/README.md) | Reading the two buttons, debouncing, edge detection |
| 3 | [Full Word Clock](03_FullWordClock/README.md) | Capstone — the 5-minute bucket logic that turns a time into a sentence |

## After the tutorial

Compare your Lesson 3 with [`WordClock/WordClock.ino`](../../WordClock/WordClock.ino) to see what the shipped firmware adds: self-serve WiFi/timezone setup, colour modes, light effects, a factory self-test and OTA.

From there you have everything you need to design your own panel — new vocabulary, new triggers, new data source — on the same 11×10 grid.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
