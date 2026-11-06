# 🚀 Crypto Word Clock

*ESP32-S3 Blueprints No.01 — Word Clock, Crypto Word Clock face*

The market's mood in words: **MOON**, **DUMP**, **ATH**, **DIP**, **FOMO**, **FUD**. Your friend group can fire **TO THE MOON**, **BUY THE DIP** or **HODL** at each other's panels and answer with **LOL**, **RIP** or **SAME**.

Not financial advice. It's a lamp.

**→ [Install it from your browser](https://lonely-binary.github.io/WordClock/)** (Chrome or Edge, press *Install Crypto Word Clock*)

<p align="center"><img src="../docs/images/crypto.svg" width="440" alt="Crypto Word Clock faceplate lit: MOON, ATH, FOMO, TO THE MOON, LOL"></p>

## The faceplate

```
  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      Q  Z  M  O  O  N  D  U  M  P  K    MOON | DUMP        market
  r1      R  K  V  A  T  H  Q  Z  D  I  P    ATH  DIP           market
  r2      Q  F  O  M  O  Z  K  F  U  D  V    FOMO  FUD          market
  r3      Q  R  E  K  T  Z  W  H  A  L  E    REKT  WHALE        market feed
  r4      Q  Z  A  P  E  K  S  E  N  D  V    APE  SEND          friends
  r5      T  O  Q  T  H  E  Z  M  O  O  N    TO THE MOON        friends
  r6      B  U  Y  Q  T  H  E  Z  D  I  P    BUY THE DIP        friends
  r7      Q  Z  R  U  G  P  U  L  L  K  V    RUGPULL            friends
  r8      Q  L  O  L  R  I  P  S  A  M  E    LOL | RIP | SAME   replies
  r9      Q  H  O  D  L  Z  P  A  N  I  C    HODL | PANIC       friends
```

## What lights up

**Market words.** The panel fetches these itself, with no account or key:

| Words | When | Source |
|---|---|---|
| **MOON** / **DUMP** | 24-hour change +5% or more / −5% or less. Goes out once it's back inside ±3% | [CoinGecko](https://www.coingecko.com/), every 3 minutes |
| **ATH** / **DIP** | Within 0.5% of the all-time high / more than 10% below it | CoinGecko |
| **FOMO** / **FUD** | Fear & Greed index 75 or more / 25 or less | [alternative.me](https://alternative.me/crypto/fear-and-greed-index/), hourly |

It watches **bitcoin** by default. Any CoinGecko coin id works (`ethereum`, `solana` …); change it in Settings.

**REKT** and **WHALE** (mass liquidations, big transfers) need a service that watches the whole market, which the panel can't do on its own. They light when a message arrives on `lonelybinary/crypto/global/trigger/rekt` (or `.../whale`) on the MQTT broker, so they stay dark until someone runs such a feed.

**Friend words.** APE, SEND, TO THE MOON, BUY THE DIP, RUGPULL, HODL, PANIC and the replies LOL, RIP, SAME. Press one on any phone in your group and it lights on **every** panel in the group, yours included. It stays for 60 seconds; HODL and PANIC stay for 10 minutes.

In **Auto** colour, market words glow orange (MOON and ATH green, DUMP red), friend words blue, and the replies yellow, red and green.

## Setup

1. Install the firmware and power the unit from the base.
2. On your phone, join the open WiFi network **`CryptoClock-XXXX`**. A setup page opens (or go to `http://192.168.4.1`).
3. Pick your home WiFi. Optionally set the **coin**, and a **friend group code**. Everyone in the group types the same code; make it long and unguessable (for example `moonboys-7k2p9`).

Without a group code the market words still work; only the friend buttons are hidden.

## Control page

Open **`http://cryptoclock-xxxx.local`** (the same `XXXX` as the setup network) from a phone on the same WiFi. It shows the price, the 24-hour change, the distance from the all-time high and the Fear & Greed index, plus the buttons to send to your group and to reply.

If the market data can't be fetched, the first corner dot glows dim red. If the friend-message server can't be reached, the second does.

## Buttons

- **A**: change colour (Auto → White → Red → Blue → Green → Rainbow, remembered)
- **B**: play a light effect
- **Both for 3 seconds**: forget WiFi and settings, back to setup

## Privacy

Friend messages travel through the public MQTT broker **broker.emqx.io**, encrypted (TLS), on topics under `lonelybinary/crypto/group/<group code>/`. Anyone who knows the code can light your group's panels. Messages carry nothing but the panel's four-character id.

## Building from source

Open `CryptoClock.ino` in the Arduino IDE. You need the `libraries/LBPanel` folder from this repo copied into your Arduino `libraries` folder, plus FastLED, WiFiManager, PubSubClient and ArduinoJson from the Library Manager. Board **ESP32S3 Dev Module**, **Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)**. See the [root README](../README.md#building-from-source).

To trigger from something else, publish any payload to `lonelybinary/crypto/group/<group code>/manual/<ape|send|tothemoon|buythedip|rugpull|hodl|panic>` or `.../reply/<lol|rip|same>`.
