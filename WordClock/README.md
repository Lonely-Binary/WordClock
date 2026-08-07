# WordClock Firmware

*ESP32-S3 Blueprints No.01 — Word Clock*

The finished "tells the time in English sentences" panel — `IT IS <hour> PAST/TO <hour>` style, QLOCKTWO-inspired, running on the Lonely Binary ESP32-S3 word clock hardware.

This is the exact firmware flashed onto every unit before it ships, so it doesn't hardcode any one buyer's WiFi network or timezone — instead it walks you through a one-time setup the first time it boots.

## Just want it installed?

You don't need any of the below. **[Open the web installer](https://lonely-binary.github.io/WordClock/)** in Chrome or Edge, plug in the board, press Install. The rest of this page is for building it yourself from source.

## What you need

- **Hardware**: Lonely Binary 11×10 WS2812 letter grid + ESP32-S3 dev board (see the [root README](../README.md) for the full hardware rundown and the two-Type-C-port power/flashing warning — read that before you plug anything in).
- **Arduino IDE** with the ESP32 board package installed, board set to **`ESP32S3 Dev Module`** (board FQBN `esp32:esp32:esp32s3`).
- **[FastLED](https://github.com/FastLED/FastLED) 3.6.0 or newer** — older versions lack solid ESP32-S3 RMT (LED driver) support.
- **[WiFiManager](https://github.com/tzapu/WiFiManager) 2.0.16 or newer** (by tzapu) — powers the WiFi/timezone setup flow below.
- Both installed via Arduino IDE → Tools → Manage Libraries → search the name. `ArduinoOTA` and `Preferences` ship with the ESP32 board package — nothing extra to install for those.

## First-boot setup (no app required)

Flash `WordClock.ino` once over the **dev board's own Type-C port**, then power the unit from the **base's Type-C port** (the dev board's USB port alone can't drive the whole panel — expected, not a fault).

1. **On a genuinely unconfigured unit, a one-time factory QC self-test runs first**: the whole panel lights solid RED, then GREEN, then BLUE, 1 second each at half brightness. This is meant for spotting a dead or miswired WS2812 pixel before a unit ships — it only ever runs when there's no saved WiFi, never on a buyer's already-configured clock.
2. **A blue snake crawls across the panel while it tries to join WiFi**, then **a red snake while it waits for the time to sync over NTP**. On an already-configured clock that's all you'll see — blue, red, then the time. (Both steps take a few seconds; the animations are there so the panel never looks dead while it works.)
3. **If there's no saved WiFi, the panel switches to a scattered confetti/sparkle animation** (random single LEDs, fading — not a full-grid rainbow, to keep current draw low) — that means it's waiting to be set up.
4. **On your phone, look for a WiFi network called `WordClock-XXXX`** (the last 4 characters are unique to your unit — handy if you have more than one). It's an open network, no password.
5. Connect to it. A setup page should pop up automatically (a "captive portal"); if it doesn't, open a browser and go to `http://192.168.4.1`.
6. The page is branded **Lonely Binary** and asks for your home WiFi name/password, plus a **timezone dropdown** covering the whole world (Americas, Europe, Africa/Middle East, South/Southeast/East Asia, Oceania, plus a plain UTC fallback). Fill it in and submit.
7. The panel reboots, connects to your WiFi, syncs time over NTP, and starts displaying the time — the confetti animation stops as soon as it's connected.

If you ever move the clock to a new house, change your WiFi password, or want to switch timezone, you don't need to reflash — see **Reset / re-setup** below.

## Reset / re-setup

**Hold both buttons (COLOR + EFFECT) down together for 3 seconds, at any time** — even while the clock is running normally, not just at boot. The panel blinks white a few times to confirm, forgets the saved WiFi network and timezone, and restarts straight into setup mode (step 1 above).

## OTA (wireless) updates

Once connected to WiFi, the panel appears in the Arduino IDE as a **network port** (Tools → Port → look for the same `WordClock-XXXX` hostname), so you can upload new firmware over WiFi instead of unplugging it from its base to reach the dev board's USB port.

**Power the unit from the base's Type-C port when doing an OTA update.** An OTA writes flash and transmits over WiFi at the same time, which is the peak current moment for the board. Running it off the dev board's USB port alone was enough to make updates stall partway through and abort here; on base power the same update completes reliably. (The firmware also disables WiFi modem-sleep — measured LAN latency dropped from ~95 ms to ~15 ms — which was the other half of this problem.)

There's **no OTA password** — anyone on the same WiFi network could push new firmware to the panel. That's an acceptable trade-off for a home device, but if you want to lock it down, add a password via `ArduinoOTA.setPassword(...)` before `ArduinoOTA.begin()` in `onNewlyConnected()`.

## What it displays

- `IT IS <hour> O'CLOCK`, `IT IS FIVE PAST <hour>`, `IT IS A QUARTER TO <hour>`, etc. — full 5-minute-resolution English time-telling.
- The 4 corner dots show the leftover 1–4 minutes inside each 5-minute block.
- AM/PM indicator in the top row (toggle with `SHOW_AMPM`).

### Status animations

The panel is never left blank while it's busy, so it can't be mistaken for broken hardware:

| Animation | Meaning |
|---|---|
| 🔵 Blue snake | Joining WiFi |
| 🔴 Red snake | Connected, waiting for the NTP time sync |
| ✨ Confetti | Not set up yet — the `WordClock-XXXX` setup hotspot is open |
| 🔴 Red flashes ×3 | The WiFi details you submitted didn't connect; pick again |
| ⚪ White flashes ×3 | Factory reset accepted (both buttons held 3s) |

## Controls

| Button | GPIO | Action |
|---|---|---|
| COLOR  | 42 | Cycles White → Red → Blue → Green → Rainbow |
| EFFECT | 41 | Plays a random light animation, then returns to showing the time |
| COLOR + EFFECT held 3s | 42 + 41 | Factory reset — forgets WiFi/timezone, returns to setup mode |

## Tuning

- `BRIGHTNESS` (0–255) — panel brightness.
- `SERPENTINE` — set to `1` if alternate rows appear mirrored on your wiring.
- `RESET_HOLD_MS` — how long (ms) the two buttons must be held together to trigger the reset gesture; defaults to 3000.
- Timezone is no longer a compile-time constant — it's picked once, from a worldwide dropdown, in the setup portal and stored on-device (`Preferences`, namespace `wordclock`). `NTP1-3` (the default NTP servers) are still compile-time constants: `pool.ntp.org`, `time.google.com`, `time.cloudflare.com` — reachable from almost anywhere. **China is the one exception**: selecting the China timezone entry automatically switches to `ntp.aliyun.com` / `cn.pool.ntp.org` / `ntp.ntsc.ac.cn` instead, since the global servers are unreliable from inside mainland China. See `TZ_OPTIONS` in the sketch to add more per-region NTP overrides the same way.

## New to this hardware?

If you'd rather build up to this from scratch — lighting a single pixel, then a word, then adding buttons, WiFi, and OTA one step at a time — start with the [Tutorial series](../Tutorial/README.md) instead. Its final core lesson arrives at a simplified version of this same sketch (the Arduino/C++ track also has a bonus lesson on OTA updates). Note: the tutorial's WiFi lessons use plain hardcoded credentials to teach the underlying concepts one at a time — the WiFiManager-based setup flow described above is specific to this finished firmware.

---

Thank you for bringing a **Lonely Binary** word clock onto your desk — this firmware exists because of buyers like you. We hope watching your own words light up the room is worth every minute you spend setting it up and tuning it.

**— Lonely Binary**
