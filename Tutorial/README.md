# Word Clock Tutorial — Learn the Hardware, Step by Step

A beginner-friendly path through the Lonely Binary ESP32-S3 word clock hardware: one small concept per lesson, each in its own folder with a full sketch/script and a line-by-line explanation. By the end you'll understand the panel well enough to build your own display logic, not just this one — the [root README](../README.md) describes four other panel concepts (Weather Station, Status Board, Couple Message Board, Crypto/DevOps Word Clock) that all run on this exact same hardware.

If you just want the finished, full-featured clock, skip straight to [`WordClock/`](../WordClock/README.md). This series is for understanding *how* it's built.

## Pick your language

| Track | Best for |
|---|---|
| **[Arduino/C++](Arduino/README.md)** | Arduino IDE users, and anyone who wants the same language as the full [`WordClock/`](../WordClock/README.md) firmware |
| **[MicroPython](MicroPython/README.md)** | Readers who'd rather write Python, or are already comfortable with MicroPython on other boards |

Both tracks teach the exact same 7 core concepts, in the exact same order, on the exact same hardware — pick whichever language you'd rather learn in. (The MicroPython track's capstone lesson stops at the simple "just tells time" sketch; the button-driven colour modes and light effects in the full firmware are Arduino/C++ only.) The Arduino/C++ track also has a bonus 8th lesson on OTA (wireless) updates, not present in the MicroPython track.

## Hardware recap (applies to both tracks)

- **GPIO 8** — WS2812 data line for the whole panel (110 letter LEDs + 4 corner "minute" dot LEDs = 114 total).
- **GPIO 42** — COLOR button (active-high).
- **GPIO 41** — EFFECT button (active-high).
- **Two Type-C ports, two different jobs**: the port on the **dev board itself** is for flashing firmware/scripts only. The port on the **base** powers everything, including the LED grid — flash over the dev board's port, then power from the base's port to actually see LEDs light up. See the [root README](../README.md#️-power--flashing--please-read) for the full explanation; this trips up almost everyone on their first flash.

## The lessons

| # | Lesson | What you'll learn |
|---|---|---|
| 1 | Single Pixel | Light exactly one LED, confirm your hardware works |
| 2 | Light a Word | Row/column grid mapping, lighting a whole word |
| 3 | Colors and Grid | Fill the whole panel, colour modes, simple animation |
| 4 | Buttons | Reading the onboard buttons, debouncing, edge detection |
| 5 | WiFi Connect | Getting the board online in station mode |
| 6 | NTP Time | Syncing real time over the internet |
| 7 | Full Word Clock | Capstone — combine it all into the simplest program that tells time |
| 8 | OTA Updates *(Arduino/C++ only, bonus)* | Upload new firmware over WiFi instead of USB |

Open the [Arduino/C++ track](Arduino/README.md) or the [MicroPython track](MicroPython/README.md) for the full write-up of each lesson.

## After the tutorial

Once you've built Lesson 7 in either track, you have everything you need to design your own panel — new vocabulary, new triggers, new data source — on the same 11×10 grid.

---

Thank you for bringing a **Lonely Binary** word clock onto your desk. Every lesson in this series exists because of buyers like you — we hope it makes the hardware feel like yours, not just ours.

**— Lonely Binary**
