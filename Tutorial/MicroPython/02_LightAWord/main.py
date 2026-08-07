"""
Lesson 2 - Light a Word (MicroPython)
--------------------------------------
The panel is really an 11x10 grid of letters, not just a flat strip.
This lesson introduces row/column mapping and lights up one whole word:
"IT IS" in the top-left corner.

  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
"""

from machine import Pin
import neopixel

LED_PIN = 8
COLS = 11
ROWS = 10
NUM_LEDS = 114

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)


def xy(row, col):
    """Convert a (row, col) grid position into an index in np[]."""
    return row * COLS + col


def light_word(row, col, length, color=(255, 255, 255)):
    """Light `length` letters starting at (row, col) - every word on this
    panel is horizontal and contiguous, so this one helper draws all of them."""
    for i in range(length):
        np[xy(row, col + i)] = color


np.fill((0, 0, 0))
light_word(0, 0, 2)   # row 0, starting at col 0, 2 letters -> "IT"
light_word(0, 3, 2)   # row 0, starting at col 3, 2 letters -> "IS"
np.write()
