# Lesson 6 — NTP Time (MicroPython)

**Goal:** once online (from Lesson 5), fetch the real current time over the internet using NTP, and print it every second. No panel rendering yet — this lesson is purely "can I get the correct time onto this board", which Lesson 7 then turns into words on the panel.

## What you'll learn

- **NTP** (Network Time Protocol): how the board gets accurate wall-clock time from a server, since the ESP32 has no battery-backed real-time clock of its own
- MicroPython's built-in `ntptime` module — `ntptime.settime()` syncs the board's clock to **UTC**
- Why MicroPython needs a manual UTC offset for local time, unlike the Arduino/C++ track's `configTzTime()`
- `time.localtime()` and MicroPython time tuples

## Before you start

Fill in your WiFi credentials as in Lesson 5. The default offset is US Eastern Time. If you're elsewhere, change `UTC_OFFSET_SEC` and `ntptime.host` to match your region — for example `-8 * 3600` for US Pacific Standard Time, or any public NTP pool like `pool.ntp.org`.

## The code

See [`main.py`](main.py).

```python
UTC_OFFSET_SEC = -5 * 3600
ntptime.host = "us.pool.ntp.org"

def sync_time():
    ntptime.settime()

def local_time():
    return time.localtime(time.time() + UTC_OFFSET_SEC)
```

## Line by line

- `ntptime.host = "us.pool.ntp.org"` — `ntptime` only supports a single server (unlike the Arduino track's 3-server fallback), so point it at a reliable US pool.
- `ntptime.settime()` — reaches out over NTP and sets the board's internal RTC directly to **UTC**. There's no timezone concept here at all — MicroPython's standard library doesn't ship a timezone/DST database the way the ESP32 Arduino core's `configTzTime()` does.
- `time.time() + UTC_OFFSET_SEC` — since everything is stored as UTC, we manually add the offset (in seconds) whenever we want local time. `UTC_OFFSET_SEC = -5 * 3600` means "5 hours behind UTC" — US Eastern Standard Time.
- **Daylight saving isn't automatic here.** The Arduino/C++ track's POSIX `TZ` string handles US DST switching for you; in MicroPython you'd need to either update `UTC_OFFSET_SEC` by hand twice a year, or write your own date-based DST check (a good exercise once you're comfortable with this lesson).
- `time.localtime(...)` — converts a Unix timestamp into a tuple: `(year, month, mday, hour, minute, second, weekday, yearday)`.

## What you should see

In the REPL: a "WiFi connecting..." line, then "NTP syncing...", then "Time synced (UTC).", then a new `Now: HH:MM:SS` line every second showing the real current time in US Eastern Time.

## Troubleshooting

- **NTP sync never completes / `OSError` from `ntptime.settime()`**: some networks block NTP's UDP port 123 — try a different network, or a different NTP server.
- **Time is right but off by a fixed number of hours**: your `UTC_OFFSET_SEC` doesn't match your actual timezone, or you're in/out of daylight saving and haven't updated it.

## Next

[Lesson 7 — Full Word Clock](../07_FullWordClock/README.md): the capstone — combine the grid/word rendering from Lesson 2 with the WiFi and NTP time from this lesson to build the simplest possible sketch that actually tells time in words.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
