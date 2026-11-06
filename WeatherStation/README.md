# 🌤 Weather Station

*ESP32-S3 Blueprints No.01 — Word Clock, Weather Station face*

The weather outside, as one sentence: **IT IS CLOUDY … NOW … FEELS LIKE COOL**. It updates every 10 minutes from [Open-Meteo](https://open-meteo.com/), which is free and needs no account or API key.

**→ [Install it from your browser](https://lonely-binary.github.io/WordClock/)** (Chrome or Edge, press *Install Weather Station*)

## The faceplate

```
  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      I  T  Q  I  S  V  Z  K  D  M  R    IT  IS
  r1      S  U  N  N  Y  C  L  O  U  D  Y    SUNNY | CLOUDY
  r2      R  A  I  N  Y  S  N  O  W  Y  B    RAINY | SNOWY
  r3      W  I  N  D  Y  F  O  G  G  Y  K    WINDY | FOGGY
  r4      S  T  O  R  M  Y  C  L  E  A  R    STORMY | CLEAR
  r5      H  O  T  C  O  L  D  M  I  L  D    HOT | COLD | MILD
  r6      W  A  R  M  C  O  O  L  D  R  Y    WARM | COOL | DRY
  r7      B  R  E  E  Z  Y  C  A  L  M  T    BREEZY | CALM
  r8      T  O  N  I  G  H  T  N  O  W  D    TONIGHT | NOW
  r9      F  E  E  L  S  V  L  I  K  E  R    FEELS LIKE
```

## What lights up

| Words | When |
|---|---|
| **IT IS** | Always |
| **SUNNY** / **CLEAR** | Clear sky, by day / at night |
| **CLOUDY** | Partly cloudy or overcast |
| **RAINY** | Drizzle, rain or showers |
| **SNOWY** | Snow |
| **FOGGY** | Fog |
| **STORMY** | Thunderstorm |
| **WINDY** | Wind 40 km/h or more (replaces a clear or cloudy sky) |
| **NOW** / **TONIGHT** | Day / night at your location |
| **FEELS LIKE** + **HOT** / **WARM** / **MILD** / **COOL** / **COLD** | "Feels like" temperature: 30 °C+, 24+, 16+, 8+, below 8 |
| **DRY** | Humidity under 30% (only next to HOT, COLD or MILD, since WARM and COOL share its row) |
| **BREEZY** / **CALM** | Wind 15–40 km/h / under 6 km/h |

In **Auto** colour, SUNNY glows warm yellow, RAINY and STORMY blue, and the temperature word runs from red (HOT) to blue (COLD). If an update fails, the first corner dot glows dim red until the next one succeeds.

## Setup

1. Install the firmware and power the unit from the base.
2. On your phone, join the open WiFi network **`Weather-XXXX`**. A setup page opens (or go to `http://192.168.4.1`).
3. Pick your home WiFi and type your **city**, for example `Sydney` or `London`. For a precise spot, type `latitude,longitude` instead, like `-33.87,151.21`.
4. The panel restarts once, joins your WiFi and shows the weather.

## Control page

From any phone or computer on the same WiFi, open **`http://weather-xxxx.local`** (the same `XXXX` as the setup network). It shows the place it found, the feels-like temperature, humidity and wind. It also has an **Update now** button, and a **Settings** page where you can change the city.

## Buttons

- **A**: change colour (Auto → White → Red → Blue → Green → Rainbow, remembered)
- **B**: play a light effect
- **Both for 3 seconds**: forget WiFi and settings, back to setup

## Building from source

Open `WeatherStation.ino` in the Arduino IDE. You need the `libraries/LBPanel` folder from this repo copied into your Arduino `libraries` folder, plus FastLED, WiFiManager and ArduinoJson from the Library Manager. Board **ESP32S3 Dev Module**, **Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)**. See the [root README](../README.md#building-from-source) for the full list.

The thresholds above are plain numbers in `render()` (temperature, humidity, wind) and `skyWord()` (sky), if you'd like your own idea of "warm".
