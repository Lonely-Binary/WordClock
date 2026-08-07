# Lesson 3 — Colors and Grid (MicroPython)

**Goal:** light up the entire panel (all 114 LEDs) and cycle through solid colours and a rainbow.

## What you'll learn

- Filling the whole `np` buffer instead of individual words
- Plain `(r, g, b)` tuples — MicroPython's `neopixel` module doesn't have a built-in HSV type like FastLED's `CHSV`, so we write a small `hsv_to_rgb()` helper ourselves
- Why HSV makes rainbows easy: a rainbow is just hue sweeping from 0 to 255 while saturation and brightness stay maxed out
- An infinite `while True:` animation loop with `time.sleep()`

## The code

See [`main.py`](main.py). The two functions worth focusing on:

```python
def fill_solid(color):
    np.fill(color)
    np.write()

def fill_rainbow():
    for i in range(NUM_LEDS):
        hue = (i * 256 // NUM_LEDS) % 256
        np[i] = hsv_to_rgb(hue, 255, 255)
    np.write()
```

## Line by line

- `fill_solid(color)` — sets every pixel in the buffer to the same `(r, g, b)` tuple in one call, then writes once (not once per pixel — that would be slow and would flicker).
- `hsv_to_rgb(h, s, v)` — a standard integer HSV→RGB conversion (hue 0–255 instead of the usual 0–360°, to fit in a byte, matching FastLED's convention so the two tutorial tracks stay conceptually identical). You don't need to memorize this function — treat it as a library helper.
- `fill_rainbow()` — gives pixel `i` the hue `(i * 256 // NUM_LEDS) % 256`, so hue advances smoothly across the whole strip: pixel 0 is red-ish, and by the last pixel you've swept almost all the way around the colour wheel back to red.
- The `while True:` loop cycles through `COLORS` (white, red, blue, green), 2 seconds each, then shows the rainbow for 2 seconds, forever.

## What you should see

The whole panel flashing through white, red, blue, green, then a rainbow gradient, 2 seconds each, repeating forever.

## Next

[Lesson 4 — Buttons](../04_Buttons/README.md): let the physical buttons on the board control the colour instead of a fixed timer.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
