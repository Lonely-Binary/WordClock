# Lesson 3 — Full Word Clock (capstone)

**Goal:** wire everything from this series together into the smallest sketch that actually tells the time in words — no buttons, no colour cycling, no light effects. Just the grid rendering from Lesson 1, plus the WiFi and NTP time you covered in the foundations, driving real "IT IS ... O'CLOCK"-style sentences.

This is deliberately the *simple* end state. For the full-featured version with the COLOR/EFFECT buttons, rainbow mode, and animations, see [`WordClock/WordClock.ino`](../../../WordClock/WordClock.ino) at the repo root — everything in this lesson is a subset of it.

## What you'll learn

- How the pieces compose: grid/word rendering (Lesson 1) + [WiFi](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/wifi/station-mode) + [NTP time](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/time/ntp-and-timezones) = a working clock
- The 5-minute "bucket" logic that turns a `minute` value into words like "TEN PAST" or "A QUARTER TO"
- Only redrawing the panel when the minute actually changes, instead of every loop

## Before you start

Fill in your WiFi credentials and timezone, exactly as in [station-mode](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/wifi/station-mode) and [ntp-and-timezones](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/time/ntp-and-timezones).

## The code

See [`03_FullWordClock.ino`](03_FullWordClock.ino). The two ideas worth walking through:

### 1. Minute buckets

```cpp
int bucket = minute / 5;   // which 5-minute block (0..11)
int rem    = minute % 5;   // leftover minutes (0..4) -> corner dots
```

English time-telling only has words for 5-minute increments ("five past", "ten past", ...), so we divide the minute into which 5-minute block it falls in (`bucket`) and whatever's left over (`rem`, shown as 1–4 lit corner dots). `bucket == 0` means "on the hour" → `"O'CLOCK"`. Buckets 1–6 (minutes 5–30) read as `"<word> PAST <hour>"`; buckets 7–11 (minutes 35–55) read as `"<word> TO <next hour>"` — which is why `hnext` (the *next* hour) gets computed too.

### 2. Redraw only on minute change

```cpp
if (t.tm_min != lastMinute) {
  lastMinute = t.tm_min;
  showTime(t.tm_hour, t.tm_min);
}
```

`loop()` runs many times a second, but the display only needs to change once a minute. Tracking `lastMinute` and comparing avoids needlessly redrawing (and flickering) the panel 100 times a second for no visible change.

## What you should see

The panel connects to WiFi, syncs time, then displays the current time as a lit sentence. For example, at 9:47 it reads **"IT IS A QUARTER TO TEN"** (47 minutes falls in the "to the next hour, quarter" bucket), with 2 corner dots lit for the leftover 2 minutes. It updates automatically every time the minute changes.

## Where to go from here

- Add back the COLOR/EFFECT buttons from Lesson 2 — you already know the debounce pattern.
- Compare your sketch line-by-line with [`WordClock/WordClock.ino`](../../../WordClock/WordClock.ino) to see exactly what the full firmware adds on top of this.
- Try designing your own panel layout/vocabulary — the [root README](../../../README.md) describes four other panel concepts (Weather Station, Status Board, Couple Message Board, Crypto/DevOps Word Clock) built on this same hardware and grid-rendering approach.

## Next

Wireless updates now live in the shared foundations: [ota-updates](https://github.com/Lonely-Binary/ESP32-Arduino-Tutorials/tree/main/wifi/ota-updates).

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
