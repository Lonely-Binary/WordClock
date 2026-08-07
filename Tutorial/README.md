# Word Clock Tutorial — Build It Yourself

*The blueprint behind ESP32-S3 Blueprints No.01.*

How this panel actually works, one small piece at a time — so that by the end you could design your own face for the same hardware, not just reflash ours.

If you only want the finished clock installed, you don't need any of this: use the [web installer](https://lonely-binary.github.io/WordClock/).

## Start with the foundations

The ESP32-S3 groundwork — uploading code, driving LEDs, WiFi, NTP — isn't repeated here. It's shared by every Blueprint in the series and lives in its own repo, so learning it once carries over to everything else we make:

| | For |
|---|---|
| **[ESP32-Arduino-Tutorials](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials)** | Arduino IDE / C++ |
| **[ESP32-MicroPython-Tutorials](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials)** | Python |

Four lessons from either (about 45 minutes) and you're ready: *hello-world → colour-and-brightness → station-mode → ntp-and-timezones*.

Already know that material? Go straight to the lessons below.

## Then the Word Clock itself

Three lessons, in whichever language you started with:

| Track | |
|---|---|
| **[Arduino/C++](Arduino/README.md)** | Same language as the shipped firmware |
| **[MicroPython](MicroPython/README.md)** | Quicker to experiment in |

| # | Lesson | What it covers |
|---|---|---|
| 1 | Light a Word | The panel as an 11×10 grid, `XY()` mapping, lighting whole words |
| 2 | Buttons | The two buttons, debouncing, edge detection |
| 3 | Full Word Clock | Capstone — turning a time into an English sentence |

## Hardware recap

- **GPIO 8** — data line for all 114 LEDs (110 letters + 4 corner minute dots)
- **GPIO 42** — COLOR button · **GPIO 41** — EFFECT button (both active-high)
- **Two Type-C ports, two jobs.** The dev board's port flashes; the **base's** port powers the LED grid. A dark panel when powered from the dev board is expected, not a fault — see the [root README](../README.md#️-power--flashing--please-read).

## After the tutorial

Compare Lesson 3 with [`WordClock/WordClock.ino`](../WordClock/WordClock.ino) — the shipped firmware adds self-serve WiFi/timezone setup, colour modes, light effects, a factory self-test and OTA.

The [root README](../README.md) also describes four other panel faces (Weather Station, Status Board, Couple Message Board, Crypto/DevOps) that run on this exact same hardware. Once you've done Lesson 3, building one of those is mostly a matter of new vocabulary and a new data source.

---

Thank you for bringing a **Lonely Binary** word clock onto your desk. Every lesson here exists because of buyers like you — we hope it makes the hardware feel like yours, not just ours.

**— Lonely Binary**
