# Lesson 3 — Full Word Clock (MicroPython capstone)

**Goal:** wire everything from this MicroPython series together into the smallest script that actually tells the time in words — no buttons, no colour cycling, no light effects. Just the grid rendering from Lesson 1, plus the WiFi and NTP time you covered in the foundations, driving real "IT IS ... O'CLOCK"-style sentences.

This is deliberately the *simple* end state, matching the Arduino/C++ track's [Lesson 3](../../Arduino/03_FullWordClock/README.md). The full-featured firmware with COLOR/EFFECT buttons, rainbow mode, and animations lives at [`WordClock/WordClock.ino`](../../../WordClock/WordClock.ino) — that one is Arduino/C++ only; this MicroPython track stops here.

## What you'll learn

- How the pieces compose: grid/word rendering (Lesson 1) + [WiFi](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/wifi/station-mode) + [NTP time](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/time/ntp-and-timezones) = a working clock
- The 5-minute "bucket" logic that turns a `minute` value into words like "TEN PAST" or "A QUARTER TO"
- Only redrawing the panel when the minute actually changes, instead of every loop
- `h12 = hour24 % 12 or 12` — a small Python idiom: since `0` is falsy in Python, `x or 12` gives you `x` unless it's `0`, in which case you get `12`. (The Arduino track needs an explicit `if (h12 == 0) h12 = 12;` for the same result.)

## Before you start

Fill in your WiFi credentials and `UTC_OFFSET_SEC`, exactly as in [station-mode](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/wifi/station-mode) and [ntp-and-timezones](https://github.com/Lonely-Binary/ESP32-MicroPython-Tutorials/tree/main/time/ntp-and-timezones).

## The code

See [`main.py`](main.py). The two ideas worth walking through:

### 1. Minute buckets

```python
bucket = minute // 5   # which 5-minute block (0..11)
rem = minute % 5        # leftover minutes (0..4) -> corner dots
```

English time-telling only has words for 5-minute increments ("five past", "ten past", ...), so we divide the minute into which 5-minute block it falls in (`bucket`) and whatever's left over (`rem`, shown as 1–4 lit corner dots). `bucket == 0` means "on the hour" → `"O'CLOCK"`. Buckets 1–6 (minutes 5–30) read as `"<word> PAST <hour>"`; buckets 7–11 (minutes 35–55) read as `"<word> TO <next hour>"` — which is why `hnext` (the *next* hour) gets computed too.

### 2. Redraw only on minute change

```python
if minute != last_minute:
    last_minute = minute
    show_time(hour, minute)
```

The loop runs many times a second, but the display only needs to change once a minute. Tracking `last_minute` and comparing avoids needlessly redrawing (and flickering) the panel 100 times a second for no visible change.

## What you should see

The panel connects to WiFi, syncs time, then displays the current time as a lit sentence. For example, at 9:47 it reads **"IT IS A QUARTER TO TEN"** (47 minutes falls in the "to the next hour, quarter" bucket), with 2 corner dots lit for the leftover 2 minutes. It updates automatically every time the minute changes.

## Where to go from here

- Add back the COLOR/EFFECT buttons from Lesson 2 — you already know the debounce pattern.
- Add automatic US daylight-saving switching to `UTC_OFFSET_SEC` — a good exercise now that you understand `local_time()`.
- Compare this script with the [Arduino/C++ version](../../Arduino/03_FullWordClock/03_FullWordClock.ino) — same logic, two different languages.
- Try designing your own panel layout/vocabulary — the [root README](../../../README.md) describes four other panel concepts (Weather Station, Status Board, Couple Message Board, Crypto/DevOps Word Clock) built on this same hardware and grid-rendering approach.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
