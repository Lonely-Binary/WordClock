# Lesson 2 — Light a Word

**Goal:** stop thinking in raw LED indices and start thinking in the panel's real shape: an 11×10 grid of letters, where every word is a horizontal run of letters on some row. Light up "IT IS" in the top-left corner.

## What you'll learn

- Why the panel is a grid, not just a strip, and how row-major wiring maps a `(row, col)` position to an LED index
- The `XY(row, col)` helper, used throughout the rest of this series (and in the full firmware) to address the grid
- A reusable `lightWord(row, col, len)` helper — since every word on this panel is a contiguous horizontal run

## The grid

```
   col ->  0  1  2  3  4  5  6  7  8  9 10
   r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
```

Row 0 holds the letters for "IT", "IS", and "AM"/"PM". The wiring runs left to right along row 0 first (LEDs 0–10), then continues along row 1 (LEDs 11–21), and so on — this is called **row-major** order.

## The code

See [`02_LightAWord.ino`](02_LightAWord.ino).

```cpp
uint16_t XY(uint8_t row, uint8_t col) {
  return (uint16_t)row * COLS + col;
}

void lightWord(uint8_t row, uint8_t col, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    leds[XY(row, col + i)] = CRGB(255, 255, 255);
  }
}
```

## Line by line

- `XY(row, col)` — converts a grid position into a `leds[]` index. Since each row has `COLS` (11) LEDs, the formula `row * COLS + col` skips over every full row before landing on the right column. This is the same trick used to address any 2D grid stored in a flat 1D array.
- `lightWord(row, col, len)` — walks `len` letters to the right of `(row, col)`, lighting each one. Every word on this panel — "IT", "IS", "QUARTER", "O'CLOCK", the hour names — is defined exactly this way: a starting cell and a length.
- `lightWord(0, 0, 2)` lights columns 0–1 of row 0 → **I T**. `lightWord(0, 3, 2)` lights columns 3–4 of row 0 → **I S**.

## What you should see

"IT IS" lit up white in the top-left of the panel — the other letters in row 0 ("L", "A", "S", "A", "M", "P", "M") stay dark, along with the rest of the grid.

## Try it yourself

Using the layout map in the header comment of [`WordClock/WordClock.ino`](../../../WordClock/WordClock.ino), try lighting up "O'CLOCK" (row 9, starting col 5, length 6) instead.

## Next

[Lesson 3 — Colors and Grid](../03_ColorsAndGrid/README.md): light up the *entire* grid and start playing with colour.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
