# Lesson 3 — Colors and Grid

**Goal:** light up the entire panel (all 114 LEDs) and cycle through solid colours and a rainbow, using FastLED's HSV colour space.

## What you'll learn

- Filling the whole `leds[]` array instead of individual words
- `CRGB` named colours (`CRGB::Red`, `CRGB::Blue`, ...) vs. `CRGB(r, g, b)` from Lesson 1
- `CHSV(hue, saturation, value)` — a colour space that makes rainbows easy, because a rainbow is just hue sweeping from 0 to 255
- Using `loop()` with `delay()` to animate over time, instead of a static `setup()`-only sketch

## The code

See [`03_ColorsAndGrid.ino`](03_ColorsAndGrid.ino).

```cpp
void fillSolid(CRGB c) {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
  FastLED.show();
}

void fillRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * 256 / NUM_LEDS), 255, 255);
  }
  FastLED.show();
}
```

## Line by line

- `fillSolid(CRGB c)` — loops over every index in `leds[]` and sets it to the same colour, then calls `show()` once at the end (not once per pixel — that would be slow and would flicker).
- `CHSV(hue, saturation, value)` — hue is a position around a colour wheel (0–255 instead of 0–360°, to fit in a byte), saturation is how vivid the colour is (255 = fully saturated), and value is brightness. FastLED converts `CHSV` to `CRGB` automatically when you assign it into a `CRGB` array.
- `fillRainbow()` — gives LED `i` the hue `i * 256 / NUM_LEDS`, so hue advances smoothly across the whole strip: LED 0 is red-ish, and by the last LED you've swept almost all the way around the colour wheel back to red.
- `loop()` cycles `mode` through white → red → blue → green → rainbow, holding each for `delay(2000)` (2 seconds) before moving to the next.

## What you should see

The whole panel flashing through white, red, blue, green, then a rainbow gradient, 2 seconds each, repeating forever.

## Next

[Lesson 4 — Buttons](../04_Buttons/README.md): let the physical buttons on the board control the colour instead of a fixed timer.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
