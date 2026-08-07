# Lesson 8 — OTA Updates (bonus, Arduino/C++ only)

**Goal:** upload new firmware to the ESP32-S3 over WiFi instead of USB. This is the last piece behind the main [`WordClock/`](../../../WordClock/README.md) firmware's "just leave it flashed once and update it from your couch" workflow.

This lesson is Arduino/C++-only and isn't part of the core 7-lesson progression — it's a standalone bonus once you're comfortable with WiFi (Lesson 5) and want one more convenience.

## What you'll learn

- What **OTA** (over-the-air) firmware updates are, and why `ArduinoOTA.handle()` has to run often
- Setting a **hostname** so the device is identifiable in the Arduino IDE's port list
- The OTA lifecycle callbacks: `onStart`, `onProgress`, `onEnd`, `onError`
- Why this lesson ships with **no OTA password**, and the trade-off that implies

## Before you start

Fill in your WiFi credentials, same as Lesson 5. Upload this sketch **once over USB** first — OTA can only update a device that's already running OTA-capable firmware; it can't turn a blank chip into one wirelessly.

## The code

See [`08_OTAUpdates.ino`](08_OTAUpdates.ino).

```cpp
void setupOTA() {
  ArduinoOTA.setHostname("wordclock-tutorial");
  ArduinoOTA.onStart([]() { Serial.println("OTA update starting..."); });
  ArduinoOTA.onEnd([]() { Serial.println("OTA update complete, rebooting."); });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    Serial.printf("OTA progress: %u%%\n", (done * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("OTA error [%u]\n", error); });
  ArduinoOTA.begin();
}
```

## Line by line

- `ArduinoOTA.setHostname("wordclock-tutorial")` — the name this device advertises on the network (via mDNS). Without a hostname, you'd have to identify the device by IP address in the Arduino IDE's port list instead of by name — annoying once you have more than one ESP32 on your network.
- `ArduinoOTA.onStart / onProgress / onEnd / onError` — optional callbacks, useful for feedback while an update is in flight. None of these are required for OTA to work — you could call `ArduinoOTA.begin()` alone — but without them you'd have no visibility into what's happening during an upload.
- `ArduinoOTA.begin()` — starts listening for incoming OTA upload requests on the network. Must be called once, after WiFi is connected (an ESP32 with no IP address has nothing to advertise or listen on).
- `ArduinoOTA.handle()` in `loop()` — this is what actually services an incoming upload once one starts. **It has to be called frequently and can't be blocked for long** — if your `loop()` gets stuck in a long `delay()` or a blocking animation while an OTA upload is in progress, the upload will stall and likely fail partway through. This is exactly why the [full firmware](../../../WordClock/WordClock.ino) had to become non-blocking once OTA (and the captive portal) were added.

## No OTA password — is that safe?

This sketch calls `ArduinoOTA.begin()` with no password, meaning **anyone on the same WiFi network could push new firmware to this device**. For a lesson, and for a home device on a home network, that's an acceptable trade-off — it keeps the setup simple, and your home WiFi password is already the real gate. If you want to lock it down further, add a line before `ArduinoOTA.begin()`:

```cpp
ArduinoOTA.setPassword("your-ota-password");
```

## What you should see

The panel lights up solid blue. In the Arduino IDE, **Tools → Port** should now list a network port named `wordclock-tutorial` alongside your usual USB ports (you may need to wait a few seconds after boot, and make sure your computer is on the same WiFi network as the board).

## Try it yourself

1. Change `DEMO_COLOR` from `CRGB::Blue` to `CRGB::Red` (or anything else).
2. In the Arduino IDE, select the `wordclock-tutorial` **network port** instead of the USB port.
3. Click Upload. No USB cable involved this time — watch the Serial Monitor (if you're still connected over USB for logging) or just watch the panel change colour once the upload finishes and the board reboots.

## Troubleshooting

- **No network port shows up**: confirm the board actually connected to WiFi (check Serial output over USB first), and that your computer is on the same WiFi network/subnet as the board — OTA discovery relies on local network multicast (mDNS), which doesn't cross most guest networks or VLANs.
- **Upload starts but fails partway through**: something in your sketch is blocking `loop()` for too long between `ArduinoOTA.handle()` calls — keep any long-running code (long `delay()`s, blocking animations) out of the loop while you expect to be doing OTA updates.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
