# LBPanel

The code the five newer Word Clock faces share: Weather Station, Status Board, Couple Message Board, Crypto Word Clock and DevOps Word Clock. Each panel sketch only supplies its letter grid and where its words come from. LBPanel does the rest, the same way the Word Clock does:

- the factory LED self-test on a never-configured unit
- the `<Panel>-XXXX` setup hotspot and captive portal, with the panel's own settings fields
- the blue "joining WiFi" snake, and confetti while it waits for setup
- buttons: A changes the colour (remembered), B plays an effect, unless the panel takes them over
- holding both buttons for 3 seconds to forget everything and go back to setup
- OTA updates from the Arduino IDE

It also adds:

- a control page on your network at `http://<panel>-xxxx.local`, with a built-in `/settings` page
- HTTPS JSON fetches, for panels that read public APIs
- MQTT over TLS to `broker.emqx.io`, for panels that talk to each other

## Install

Copy this `LBPanel` folder into your Arduino `libraries` folder (Documents/Arduino/libraries), or pass `--libraries libraries` to `arduino-cli` from the repo root. It needs FastLED, WiFiManager, PubSubClient and ArduinoJson from the Library Manager.

## A minimal panel

```cpp
#include <LBPanel.h>

const LBWord HELLO = {0, 0, 5};     // row 0, column 0, 5 letters
const LBPanelConfig PANEL = { "Hello Panel", "Hello", nullptr, 0 };

void setup() {
  lb::begin(PANEL);                 // setup portal, OTA, buttons...
}

void loop() {
  if (!lb::loop()) return;          // not on WiFi yet: LBPanel owns the LEDs
  if (lb::takeRedraw()) {           // first connect, colour change, after an effect
    lb::clear();
    lb::light(HELLO, lb::color());
    lb::show();
  }
}
```

The full API is documented in [`src/LBPanel.h`](src/LBPanel.h), and the five panel sketches in this repo are worked examples.
