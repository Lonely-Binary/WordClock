# Lonely Binary Word Clock

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

## Firmware & Libraries

Coming soon.

## Repository

Coming soon.

---

## A Note of Thanks

Every one of these five panels started as a "what if it just showed a word instead of a number?" idea, and became real hardware because of the people who support **Lonely Binary**. Thank you for being part of that — we can't wait to see which face you put on yours.

**— Lonely Binary**
