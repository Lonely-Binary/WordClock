# AGENTS.md — notes for AI coding assistants

This file is for AI assistants working on this repository (Cursor, Copilot, Codex, ChatGPT and others). It holds the facts you can't guess from the code alone and the mistakes that are easy to make on this hardware. Read it all before changing anything. Humans: the [README](README.md) is the place to start.

## What this repository is

Firmware for the **Lonely Binary Word Clock**: an 11×10 grid of WS2812 LEDs, one behind each letter of a printed faceplate, on a Lonely Binary ESP32-S3 dev board. Six faceplates exist, and each has its own Arduino sketch:

| Folder | Panel | Built on |
|---|---|---|
| `WordClock/` | Word Clock (tells the time) | standalone sketch, no LBPanel |
| `WeatherStation/` | Weather Station | `libraries/LBPanel` |
| `StatusBoard/` | Status Board | `libraries/LBPanel` |
| `CoupleBoard/` | Couple Message Board | `libraries/LBPanel` + MQTT |
| `CryptoClock/` | Crypto Word Clock | `libraries/LBPanel` + MQTT |
| `DevOpsClock/` | DevOps Word Clock | `libraries/LBPanel` + MQTT |

Each sketch starts with a comment holding its letter grid and the rules for what lights up. Each folder's `README.md` is the user-facing description of that panel.

## Hardware facts (do not change these)

| Thing | Value |
|---|---|
| MCU | ESP32-S3 (Arduino board **ESP32S3 Dev Module**, FQBN `esp32:esp32:esp32s3`) |
| LED data pin | **GPIO 8**, WS2812B, colour order **GRB** |
| LED count | **114**: 110 letters (11 columns × 10 rows), then 4 corner dots |
| LED order | Row-major, not serpentine. LED 0 = top-left letter, LED 10 = top-right, LED 11 = start of row 1 … LED 109 = bottom-right. LEDs 110–113 are the corner dots |
| Button A | **GPIO 42**, active **high** (reads HIGH when pressed). Use `INPUT`, not `INPUT_PULLUP` |
| Button B | **GPIO 41**, active **high** |
| Brightness | Default `80` out of 255. Don't raise it or light the whole panel white for long: 114 LEDs at full white can draw several amps, more than the supply is meant for |

### The two USB-C ports

- The **dev board's** port is for flashing and the serial log. It **cannot power the LEDs**.
- The **base's** port powers everything.

So "I flashed it and the LEDs stay dark" is normal while it's powered from the dev board port. It is not a firmware bug. Don't "fix" it in code; tell the user to power it from the base. Likewise, OTA updates and brownouts (reset reason 9) mean "power it from the base".

## The letter grid

A word is `{row, col, len}`: row 0–9 from the top, col 0–10 from the left, all **0-based**, horizontal only. In the LBPanel sketches, words are `const LBWord NAME = {row, col, len};`. The design documents count columns from 1, so subtract 1.

The faceplate is printed. **A sketch can only light letters that are physically on its faceplate.** You cannot add a new word unless its letters already sit in a row, in order, on that grid. If a user asks for a word that isn't there, say so and offer the nearest words that are; don't invent coordinates.

After adding or moving a word, run:

```bash
python3 tools/check-grid.py WeatherStation/WeatherStation.ino
```

It reads the grid from the sketch's header comment and prints the letters under every `LBWord`, flagging any that don't spell the word's name.

Avoid word names that clash with Arduino or system macros. These have bitten this repo: `HIGH`, `BUSY`, `R_OK`, `W_OK`; `LOW`, `INPUT`, `OUTPUT` would too. The compiler error is usually "expected unqualified-id before numeric constant". Prefix the name (`W_HIGH`, `RP_OK`).

## Building

Libraries (Arduino Library Manager names, versions used for releases):

```bash
arduino-cli core install esp32:esp32@3.3.10
arduino-cli lib install FastLED@3.10.3 WiFiManager@2.0.17 PubSubClient@2.8 ArduinoJson@7.4.3
```

Compile commands, from the repo root:

```bash
# Word Clock: default partitions, no LBPanel
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc WordClock/WordClock.ino

# The other five: MUST use Minimal SPIFFS, and the local library folder
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=min_spiffs \
  --libraries libraries WeatherStation/WeatherStation.ino
```

- **Use `PartitionScheme=min_spiffs` for the five LBPanel sketches.** They are about 1.25 MB, which is 95–98% of the default 1.25 MB app slot: they barely compile on the default scheme, and almost any addition pushes them over. The released images use Minimal SPIFFS (1.9 MB app, still with OTA), so a board flashed from the web installer has that partition table. In the Arduino IDE: Tools → Partition Scheme → *Minimal SPIFFS (1.9MB APP with OTA)*. An OTA update must use the same scheme the board was flashed with.
- `CDCOnBoot=cdc` sends `Serial` to the USB port, so `Serial.printf` output shows up in the serial monitor at 115200.
- Arduino IDE users need `libraries/LBPanel` copied into their Arduino `libraries` folder.
- Release images: `tools/build-firmware.sh <version> [panel…]` builds full-flash images into `docs/firmware/<panel>/`. CI runs it when a `v*` tag is pushed. Don't hand-edit `docs/firmware/`.

## How an LBPanel sketch works

```cpp
#include <LBPanel.h>

const LBWord HELLO = {0, 0, 5};
const LBParam PARAMS[] = { { "city", "City", "Sydney", 40 } };      // optional settings
const LBPanelConfig PANEL = { "My Panel", "MyPanel", PARAMS, 1 };   // hotspot "MyPanel-XXXX"

void setup() {
  lb::begin(PANEL);                    // LEDs, self-test, WiFi portal, OTA. Blocks while joining WiFi
  lb::web().on("/", handleRoot);       // control page routes: register after begin()
}

void loop() {
  if (!lb::loop()) return;             // false = not on WiFi; LBPanel is animating, draw nothing
  // ... fetch data / react to events, then call lb::requestRedraw() ...
  if (lb::takeRedraw()) render();      // render(): lb::clear(); lb::light(...); lb::show();
}
```

Rules that keep it working:

- **`loop()` must stay non-blocking.** Buttons, the control page, OTA and MQTT are all serviced from `lb::loop()`. Don't `delay()` for long; pace things with `millis()`. Network calls (`lb::getJson`) block for up to 8 s, so do them every few minutes, not every loop.
- **Draw only in `render()`, only when `lb::takeRedraw()` says so.** Set `lb::requestRedraw()` whenever your state changes. LBPanel also requests a redraw after a colour change, an effect, or reconnecting.
- **Colours:** `lb::color(CRGB meaning)` returns the meaning colour in Auto mode and the user's chosen colour otherwise. Always go through it, never hard-code `CRGB::Red` straight into `lb::light`.
- **Settings** go in `PARAMS` (shown in the setup portal and at `/settings`, saved to flash). Read them with `lb::param("key")`. Keys max 15 characters. Saving settings restarts the panel.
- **Buttons:** `lb::onButtons(handlerA, handlerB)`. Return `true` if you handled the press, `false` to fall back to the default (A = next colour, B = random effect).
- **MQTT:** `lb::mqttBegin(handler)`, then `lb::mqttSubscribe(topic)` in `setup()`. Topics live under `lonelybinary/<panel>/...` on the public broker `broker.emqx.io`. Anything secret (a pair code, a channel key) goes in the topic, because the broker is public.
- **Arduino auto-generates function prototypes** at the top of the sketch. A function that takes or returns a struct you define must have that struct declared before the first function in the file, or the build fails with "'X' was not declared in this scope".

The full API, with comments, is in `libraries/LBPanel/src/LBPanel.h`. The simplest complete example is `StatusBoard/StatusBoard.ino`; for fetching from an API, copy `WeatherStation/`; for MQTT, copy `CoupleBoard/`.

## Behaviour users rely on (don't remove)

- **Factory self-test** on a never-configured unit: one LED at a time through all 114, then the whole panel, in red, green and blue.
- **Setup hotspot** `<Prefix>-XXXX`, open, with a captive portal at `192.168.4.1`. `XXXX` comes from `ESP.getEfuseMac()` (not `WiFi.macAddress()`, which is wrong this early in boot).
- **Status animations:** blue snake = joining WiFi; confetti = waiting for setup or WiFi lost; red snake = Word Clock syncing time.
- **Reset gesture:** both buttons held 3 s forgets WiFi and settings, from any state.
- **OTA** from the Arduino IDE, hostname = the hotspot name, no password. `WiFi.setSleep(false)` is deliberate: modem sleep made OTA fail.
- The Word Clock's long comments explain several WiFiManager workarounds (`setCleanConnect`, releasing the STA after a failed portal save). Keep them when editing nearby code.

## Checking your work

There are no unit tests and usually no board attached. Before saying a change is done:

1. It compiles with the exact command above for that panel, with no new warnings from your code.
2. `python3 tools/check-grid.py <sketch>` shows no mismatches, if you touched words.
3. If you changed what a panel shows or how it's controlled, update that panel's `README.md`, and the header comment in the sketch, to match.
4. Say plainly that it hasn't been tested on hardware, if it hasn't.
