# Lesson 5 — WiFi Connect (MicroPython)

**Goal:** get the ESP32-S3 onto your WiFi network — the last building block before the panel can fetch real time over the internet.

## What you'll learn

- Connecting to WiFi in station mode with MicroPython's built-in `network` module
- Giving visual feedback on the panel (a sweeping dot) while waiting on something slow, instead of leaving the panel blank
- `wlan.isconnected()` and blocking until it returns `True`

## Before you start

Edit the top of the script with your own network name and password:

```python
WIFI_SSID = "YOUR_WIFI_SSID"
WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"
```

**Never commit real WiFi credentials to a public repository** — if you're keeping your own copy of this code on GitHub, leave these as local edits only, or load them from a separate file you `.gitignore`.

## The code

See [`main.py`](main.py).

```python
def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    while not wlan.isconnected():
        status_sweep((0, 40, 80), p)
        time.sleep_ms(250)
```

## Line by line

- `network.WLAN(network.STA_IF)` — creates a WiFi interface handle in **station** mode, i.e. the ESP32-S3 joins an existing network as a client (as opposed to `network.AP_IF`, which would make it broadcast its own network).
- `wlan.active(True)` — powers on the WiFi radio.
- `wlan.connect(WIFI_SSID, WIFI_PASSWORD)` — starts the connection attempt in the background; it doesn't block.
- `while not wlan.isconnected():` — so we poll in a loop until it reports connected, calling `status_sweep()` each time to advance a moving dot across row 0 of the panel — useful, low-effort feedback that something is happening instead of a dark, seemingly-frozen panel.
- `wlan.ifconfig()[0]` — once connected, this is the IP address your router assigned the board; printing it is handy for confirming the connection or looking the board up on your network later.

## What you should see

A blue dot sweeping left to right across the top row while connecting (check the REPL output for connection progress too), then the whole panel turns solid green once WiFi connects.

## Troubleshooting

- **Sweep never stops**: double-check `WIFI_SSID`/`WIFI_PASSWORD`, and that you're on a 2.4GHz network — the ESP32-S3 doesn't support 5GHz WiFi.
- **Connects then panel goes blank**: make sure you're powering the panel from the base's Type-C port, not just the dev board's.

## Next

[Lesson 6 — NTP Time](../06_NTPTime/README.md): once online, fetch the actual current time over the internet.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
