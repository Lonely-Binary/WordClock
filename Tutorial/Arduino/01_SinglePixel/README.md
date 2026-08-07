# Lesson 1 — Single Pixel

**Goal:** light up exactly one LED on the panel. This is the "does my hardware work" check — if this lesson lights up, your wiring, power, and FastLED install are all correct, and you're ready for everything else in this series.

## What you'll learn

- Installing and including the FastLED library
- The three things every FastLED sketch needs: an `addLeds` call, a `leds[]` array, and `FastLED.show()`
- Why nothing appears on the panel until you call `show()`

## Before you start

- Install **FastLED 3.6.0+** in the Arduino IDE: Tools → Manage Libraries → search "FastLED" → Install.
- Board: Tools → Board → **ESP32S3 Dev Module**.
- **Flash over the dev board's own Type-C port.** To actually *see* the LED light up afterwards, power the unit from the **base's Type-C port** instead — the dev board's USB port alone can't supply enough current for the panel. This is expected, not a fault (see the [root README](../../../README.md) for the full explanation).

## The code

See [`01_SinglePixel.ino`](01_SinglePixel.ino).

```cpp
#include <FastLED.h>

#define LED_PIN     8
#define NUM_LEDS    114
#define BRIGHTNESS  80

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.clear();
  leds[0] = CRGB(255, 255, 255);
  FastLED.show();
}
```

## Line by line

- `#define LED_PIN 8` — the panel's WS2812 data line is wired to GPIO 8 on this board. Every lesson in this series uses this same pin.
- `#define NUM_LEDS 114` — the panel has 110 letter LEDs (11 columns × 10 rows) plus 4 corner "minute dot" LEDs, wired in one single chain: 114 total.
- `CRGB leds[NUM_LEDS]` — this array is your entire panel. Index 0 is the first LED in the chain (physically the top-left letter, "I"), and so on down the wire.
- `FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS)` — tells FastLED which LED chip (`WS2812B`), which pin, which colour order (`GRB` — WS2812 chips send Green-Red-Blue, not RGB), and which array to drive.
- `FastLED.clear()` — sets every entry in `leds[]` to black (off), but doesn't push it to the panel yet.
- `leds[0] = CRGB(255, 255, 255)` — sets just LED 0 to white. This is only a change in memory so far.
- `FastLED.show()` — this is the line that actually pushes the `leds[]` array out over GPIO 8 to the physical LEDs. **Nothing lights up without this call.**

## What you should see

The top-left letter cell ("I") lit up white. Everything else on the panel stays dark.

## Troubleshooting

- **Nothing lights up at all**: are you powering from the *base's* Type-C port, not the dev board's? The dev board's own USB port can't drive the panel.
- **Wrong colours** (e.g. you expect white but see green/red tints): double check the colour order argument — it should be `GRB` for WS2812B.
- **Compile error about `FastLED.h` not found**: the library isn't installed — see "Before you start" above.

## Next

[Lesson 2 — Light a Word](../02_LightAWord/README.md): stop thinking in single LED indices and start thinking in rows/columns/words, the way the rest of this panel's software does.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
