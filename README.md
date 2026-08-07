# Word Clock — ESP32-S3 Blueprints No.01

> Part of **[ESP32-S3 Blueprints](#about-esp32-s3-blueprints)**, a Lonely Binary series: finished things you'd actually want on your desk, each shipped with the complete blueprint behind it.

**A word that lights up — instead of a number.**

Most smart displays shout numbers, graphs, and notifications at you. **Lonely Binary Word Clock** whispers a word.

It's an 11×10 letter grid that looks like a plain wall of type until the words you actually need light up — the weather outside, whether you're free or in a meeting, a quiet "HOME SAFE" from someone you love, or a red "DOWN" from a server you're watching at 2am. One piece of hardware, five very different lives — pick the face that matches yours.

---

## What It Is

- **11×10 WS2812 letter grid** (110 individually addressable RGB LEDs, one per letter)
- **+4 corner WS2812 LEDs** for ambient/accent lighting (114 LEDs total)
- Snaps onto a **standard Lonely Binary ESP32-S3** dev board via onboard socket — no soldering to get started
- **2 physical buttons**, wired to `IO42` and `IO41` (Active High)
- **USB Type-C power input on the base** — powers both the ESP32-S3 and the entire LED grid

---

## ⚠️ Power & Flashing — Please Read

This product has **two Type-C ports**, and they do different jobs:

| Port | Location | Purpose |
|---|---|---|
| **Base Type-C** | On the base | Powers *everything* — ESP32-S3 + WS2812 grid |
| **Dev board Type-C** | On the ESP32-S3 itself | Used **only** for flashing firmware |

**To flash your firmware, connect via the Type-C port on the Lonely Binary ESP32-S3 dev board** (as you would with any ESP32-S3 board).

**Do not be alarmed if the LED grid stays dark after flashing.** The dev board's own Type-C port does **not** power the WS2812 panel — the panel's current draw exceeds what the ESP32-S3's USB port can safely supply. This is expected behavior, not a fault.

To see your LEDs light up, power the unit from the **Type-C port on the base** instead.

---

## Recommended: Use Arduino OTA

Once your firmware is flashed once over USB, we strongly recommend switching to **Arduino OTA** for all future updates. Just connect to Wi-Fi and push new firmware wirelessly — no more unplugging the panel from its base and hunting for a cable.

**Power the unit from the base's Type-C port while updating over OTA.** Writing flash and transmitting over Wi-Fi at the same time is the board's peak-current moment; running it from the dev board's USB port alone is enough to make an update stall partway through.

---

## Five Panels, One Platform

All five designs below share the same 11×10 hardware and the same panel design rules — only the word layout and data logic differ, so you can flash a different personality onto the same physical unit.

### 🌤 Weather Station
A desktop weather display built around one sentence: **"IT IS [weather]"**, with **"FEELS LIKE [warm/cool/dry]"** underneath, and an optional **TONIGHT / NOW** modifier for time of day.

### 💼 Status Board
Your at-a-glance office status — busy, free, in a meeting, or on a call — plus a do-not-disturb level (**KNOCK / OK / ASK**) and a return-time hint (**SOON / LATER / BACK AT**).

### 💌 Couple Message Board
A two-way message board for couples living apart during the day. One partner taps a message (**ON THE WAY**, **HOME SAFE**, **CALL ME NOW**...), and it lights up instantly on the other's panel — with a quick **OK / NO / MAYBE** reply built in.

### 🚀 Crypto Word Clock
Your portfolio's mood, spelled out in real time — **MOON / DUMP / ATH / DIP / REKT / WHALE** — plus meme-worthy friend-to-friend triggers like **TO THE MOON** and **BUY THE DIP**, with **LOL / RIP / SAME** replies.

### 🖥 DevOps Word Clock
CI/CD results and server health on your desk — deploys, rollbacks, uptime, and resource alerts (**CPU / MEM / LOAD / DISK**), sharing common words like **HIGH / OK / DOWN** across multiple signals to keep the grid compact.

---

## Install the firmware (no tools needed)

**→ [Open the web installer](https://lonely-binary.github.io/WordClock/)**

Plug the ESP32-S3 board into your computer with a USB-C data cable, open that page in **Chrome or Edge**, and press Install. No Arduino IDE, no drivers, no downloads — the ESP32-S3's USB is natively supported by Windows 10+ and macOS.

(Safari and Firefox don't support Web Serial; that page also has a plain `esptool` fallback, and the raw image lives in [`docs/firmware/`](docs/firmware/).)

## Firmware & Libraries

The Word Clock panel is the first of the five to ship. It's built with the [Arduino ESP32 core](https://github.com/espressif/arduino-esp32) (board: `esp32:esp32:esp32s3`), [FastLED](https://github.com/FastLED/FastLED) for the WS2812 grid, and [WiFiManager](https://github.com/tzapu/WiFiManager) for the on-device Wi-Fi/timezone setup portal. `ArduinoOTA` and `Preferences` ship with the ESP32 core.

- Want the finished, ready-to-flash sketch? See [`WordClock/`](WordClock/).
- New to this hardware and want to learn it one small step at a time? Start with the [`Tutorial/`](Tutorial/) series.

## Repository

- **[`WordClock/`](WordClock/)** — the finished Word Clock panel firmware: time-telling logic, self-serve Wi-Fi/timezone setup, colour modes, light effects and OTA.
- **[`Tutorial/`](Tutorial/)** — a step-by-step, beginner-friendly series (one lesson per folder) that builds up from lighting a single LED to a working word clock, in both **Arduino/C++** and **MicroPython**, so you can design your own panels on this same hardware.
- **[`docs/`](docs/)** — the browser-based firmware installer published to GitHub Pages, plus the prebuilt image it serves.
- **[`tools/`](tools/)** — `build-firmware.sh`, which produces that image. CI runs it automatically on every tagged release.

---

## About ESP32-S3 Blueprints

Most beginner electronics kits hand you a bag of sensors and leave you to invent a reason to use them. That's the wrong way round: the hard part for a beginner was never *how*, it was *what for*. A pile of assorted Lego bricks is intimidating; a Lego set is not — and you still walk away knowing techniques you can reuse on anything.

**ESP32-S3 Blueprints** works the same way. Every entry in the series is:

- **A finished object you'd actually want.** Not a demo, not a breadboard — something that earns its place on your desk.
- **Built on the ESP32-S3**, so the skills compound instead of resetting each time.
- **Shipped with the whole blueprint.** Full source, and a lesson-by-lesson tutorial that takes you from lighting a single LED to the finished firmware — in both Arduino/C++ and MicroPython.

The thing you buy is the reason to start. What you keep is knowing how to build the next one yourself.

| No. | Blueprint | Status |
|---|---|---|
| **01** | **Word Clock** — tells the time in English words | ✅ This repo |
| 02 | — | In development |

---

## A Note of Thanks

Every one of these five panels started as a "what if it just showed a word instead of a number?" idea, and became real hardware because of the people who support **Lonely Binary**. Thank you for being part of that — we can't wait to see which face you put on yours.

**— Lonely Binary**
