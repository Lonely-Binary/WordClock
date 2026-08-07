# Lesson 6 — NTP Time

**Goal:** once online (from Lesson 5), fetch the real current time over the internet using NTP, and print it to Serial every second. No panel rendering yet — this lesson is purely "can I get the correct time onto this board", which Lesson 7 then turns into words on the panel.

## What you'll learn

- **NTP** (Network Time Protocol): how the board gets accurate wall-clock time from a server, since the ESP32 has no battery-backed real-time clock of its own
- `configTzTime()` — configures both the timezone and the NTP servers to sync from
- `getLocalTime()` — reads the current time (already adjusted for your timezone) into a standard C `struct tm`
- POSIX `TZ` strings for timezone configuration

## Before you start

Fill in your WiFi credentials as in Lesson 5. The default `TZ_INFO` is US Eastern Time. If you're elsewhere, change `TZ_INFO` and the NTP servers to match your region — for example `"PST8PDT,M3.2.0,M11.1.0"` for US Pacific time, or any public NTP pool like `pool.ntp.org`.

## The code

See [`06_NTPTime.ino`](06_NTPTime.ino).

```cpp
void syncTime() {
  configTzTime(TZ_INFO, NTP1, NTP2, NTP3);
  struct tm t;
  while (!getLocalTime(&t, 500)) { /* keep waiting */ }
}
```

## Line by line

- `configTzTime(TZ_INFO, NTP1, NTP2, NTP3)` — starts an NTP sync in the background against up to three servers (tried in order if earlier ones don't respond), and tells the C library how to convert UTC into your local time using the POSIX `TZ` string (`"EST5EDT,M3.2.0,M11.1.0"` = US Eastern Time, UTC-5 standard / UTC-4 daylight, switching automatically on the 2nd Sunday of March and 1st Sunday of November).
- `getLocalTime(&t, 500)` — tries for up to 500ms to read the synced time into `t`; returns `false` if the sync hasn't completed yet, so we loop until it succeeds.
- `struct tm` — the standard C time structure: `tm_hour`, `tm_min`, `tm_sec`, `tm_year` (years since 1900, hence `+ 1900`), `tm_mon` (0-indexed, hence `+ 1`), `tm_mday`.
- In `loop()`, we call `getLocalTime(&t, 0)` every second just to read the already-synced clock (the `0` timeout means "don't wait, just tell me now") — the ESP32 keeps its own clock ticking between NTP syncs, and `configTzTime` re-syncs periodically in the background (~hourly) to correct drift.

## What you should see

In the Serial Monitor (115200 baud): a "WiFi connecting..." line, then "NTP syncing...", then the synced date/time, then a new `Now: HH:MM:SS` line every second showing the real current time.

## Troubleshooting

- **NTP sync never completes**: some networks block NTP's UDP port 123 — try a different network, or a different NTP server.
- **Time is right but off by a fixed number of hours**: your `TZ_INFO` doesn't match your actual timezone.

## Next

[Lesson 7 — Full Word Clock](../07_FullWordClock/README.md): the capstone — combine the grid/word rendering from Lesson 2 with the WiFi and NTP time from this lesson to build the simplest possible sketch that actually tells time in words.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
