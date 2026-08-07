# Word Clock Tutorial — MicroPython Track

*The blueprint behind ESP32-S3 Blueprints No.01.*

Three lessons covering what's specific to **this** panel: its 11×10 letter grid, its two buttons, and the logic that turns a time into an English sentence.

Prefer C++? See the [Arduino/C++ track](../Arduino/README.md).

## Do this first

These lessons assume you can already get an ESP32-S3 online and telling the time. That groundwork isn't repeated here — it lives in **[ESP32-MicroPython-Tutorials](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials)**, shared by every Blueprint in the series.

Work through these four (about 45 minutes), then come back:

| | Lesson | Why you need it |
|---|---|---|
| 1 | [hello-world](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/getting-started/hello-world) | Flashing MicroPython, the REPL, `neopixel`, and `np.write()` |
| 2 | [colour-and-brightness](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/leds/colour-and-brightness) | RGB tuples and the `hsv_to_rgb()` helper reused below |
| 3 | [station-mode](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/wifi/station-mode) | Getting on WiFi |
| 4 | [ntp-and-timezones](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/time/ntp-and-timezones) | Real local time — including the UTC offset MicroPython makes you apply yourself |

Already comfortable with those? Skip straight to Lesson 1 below.

## Prerequisites

- **MicroPython** flashed onto the board (ESP32_GENERIC_S3 build)
- **[Thonny](https://thonny.org/)** or `mpremote`
- Your assembled Word Clock: the 11×10 WS2812 grid on its base

## Hardware recap

- **GPIO 8** — data line for the whole panel: 110 letter LEDs + 4 corner "minute" dots = 114
- **GPIO 42** — COLOR button (active-high)
- **GPIO 41** — EFFECT button (active-high)
- **Two Type-C ports, two jobs.** The dev board's port talks to your computer; the **base's** port powers the LED grid. See the [root README](../../README.md#️-power--flashing--please-read).

## Lessons

| # | Lesson | What you'll learn |
|---|---|---|
| 1 | [Light a Word](01_LightAWord/README.md) | The panel as an 11×10 grid, `xy()` mapping, lighting whole words |
| 2 | [Buttons](02_Buttons/README.md) | Reading the two buttons, debouncing, edge detection |
| 3 | [Full Word Clock](03_FullWordClock/README.md) | Capstone — the 5-minute bucket logic that turns a time into a sentence |

## What this track doesn't cover

The shipped firmware — self-serve WiFi setup, colour modes, light effects, OTA — is **Arduino/C++ only**, and lives at [`WordClock/WordClock.ino`](../../WordClock/WordClock.ino). This track deliberately stops at a working clock.

That's not a limitation of MicroPython so much as a choice: the C++ version is what ships on the hardware, so that's where the extra features live.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
