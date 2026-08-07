# Lesson 5 — WiFi Connect

**Goal:** get the ESP32-S3 onto your WiFi network — the last building block before the panel can fetch real time over the internet.

## What you'll learn

- Connecting to WiFi in station mode with the `WiFi` library
- Giving visual feedback on the panel (a sweeping dot) while waiting on something slow, instead of leaving the panel blank
- `WiFi.status()` and blocking until `WL_CONNECTED`

## Before you start

Edit the top of the sketch with your own network name and password:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
```

**Never commit real WiFi credentials to a public repository** — if you're keeping your own copy of this code on GitHub, leave these as local edits only, or move them into a separate file you `.gitignore`.

## The code

See [`05_WiFiConnect.ino`](05_WiFiConnect.ino).

```cpp
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    statusSweep(CRGB(0, 40, 80));
    delay(250);
  }
}
```

## Line by line

- `WiFi.mode(WIFI_STA)` — puts the ESP32-S3 in **station** mode, i.e. it joins an existing network as a client (as opposed to `WIFI_AP`, which would make it broadcast its own network).
- `WiFi.begin(WIFI_SSID, WIFI_PASS)` — starts the connection attempt in the background; it doesn't block.
- `while (WiFi.status() != WL_CONNECTED)` — so we poll in a loop until it reports connected, calling `statusSweep()` each time to advance a moving dot across row 0 of the panel — useful, low-effort feedback that something is happening instead of a dark, seemingly-frozen panel.
- `WiFi.localIP()` — once connected, this is the IP address your router assigned the board; printing it to Serial is handy for confirming the connection or looking the board up on your network later.

## What you should see

A blue dot sweeping left to right across the top row while connecting (check the Serial Monitor at 115200 baud for connection progress too), then the whole panel turns solid green once WiFi connects.

## Troubleshooting

- **Sweep never stops**: double-check `WIFI_SSID`/`WIFI_PASS`, and that you're on a 2.4GHz network — the ESP32-S3 doesn't support 5GHz WiFi.
- **Connects then panel goes blank**: make sure you're powering the panel from the base's Type-C port, not just the dev board's.

## Next

[Lesson 6 — NTP Time](../06_NTPTime/README.md): once online, fetch the actual current time over the internet.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
