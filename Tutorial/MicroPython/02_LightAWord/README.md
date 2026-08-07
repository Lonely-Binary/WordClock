# Lesson 2 — Light a Word (MicroPython)

**Goal:** stop thinking in raw LED indices and start thinking in the panel's real shape: an 11×10 grid of letters, where every word is a horizontal run of letters on some row. Light up "IT IS" in the top-left corner.

## What you'll learn

- Why the panel is a grid, not just a strip, and how row-major wiring maps a `(row, col)` position to an LED index
- The `xy(row, col)` helper, used throughout the rest of this series to address the grid
- A reusable `light_word(row, col, length)` helper — since every word on this panel is a contiguous horizontal run

## The grid

```
   col ->  0  1  2  3  4  5  6  7  8  9 10
   r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
```

Row 0 holds the letters for "IT", "IS", and "AM"/"PM". The wiring runs left to right along row 0 first (LEDs 0–10), then continues along row 1 (LEDs 11–21), and so on — **row-major** order.

## The code

See [`main.py`](main.py).

```python
def xy(row, col):
    return row * COLS + col

def light_word(row, col, length, color=(255, 255, 255)):
    for i in range(length):
        np[xy(row, col + i)] = color
```

## Line by line

- `xy(row, col)` — converts a grid position into a `np[]` index. Since each row has `COLS` (11) LEDs, `row * COLS + col` skips over every full row before landing on the right column — the standard trick for addressing a 2D grid stored in a flat 1D array.
- `light_word(row, col, length, color=...)` — walks `length` letters to the right of `(row, col)`, lighting each one. Every word on this panel — "IT", "IS", "QUARTER", "O'CLOCK", the hour names — is defined exactly this way: a starting cell and a length. The `color` keyword argument defaults to white, so you can call `light_word(0, 0, 2)` without thinking about colour yet, but override it later.
- `light_word(0, 0, 2)` lights columns 0–1 of row 0 → **I T**. `light_word(0, 3, 2)` lights columns 3–4 of row 0 → **I S**.

## What you should see

"IT IS" lit up white in the top-left of the panel — the other letters in row 0 ("L", "A", "S", "A", "M", "P", "M") stay dark, along with the rest of the grid.

## Try it yourself

Using the layout map in the header comment of [`WordClock/WordClock.ino`](../../../WordClock/WordClock.ino), try lighting up "O'CLOCK" (row 9, starting col 5, length 6) instead.

## Next

[Lesson 3 — Colors and Grid](../03_ColorsAndGrid/README.md): light up the *entire* grid and start playing with colour.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
