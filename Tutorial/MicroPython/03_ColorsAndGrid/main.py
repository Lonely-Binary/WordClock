"""
Lesson 3 - Colors and Grid (MicroPython)
------------------------------------------
Light up the ENTIRE panel and cycle through colours, including a
per-pixel rainbow using a hand-rolled HSV-to-RGB conversion (MicroPython's
neopixel module only speaks RGB, unlike FastLED's built-in CHSV).
"""

from machine import Pin
import neopixel
import time

LED_PIN = 8
NUM_LEDS = 114

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)

COLORS = [
    (255, 255, 255),   # white
    (255, 0, 0),       # red
    (0, 0, 255),        # blue
    (0, 255, 0),        # green
]


def fill_solid(color):
    np.fill(color)
    np.write()


def hsv_to_rgb(h, s, v):
    """h, s, v all 0-255 -> returns an (r, g, b) tuple, 0-255 each."""
    if s == 0:
        return (v, v, v)
    region = h // 43
    remainder = (h - (region * 43)) * 6
    p = (v * (255 - s)) >> 8
    q = (v * (255 - ((s * remainder) >> 8))) >> 8
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8
    if region == 0:
        return (v, t, p)
    if region == 1:
        return (q, v, p)
    if region == 2:
        return (p, v, t)
    if region == 3:
        return (p, q, v)
    if region == 4:
        return (t, p, v)
    return (v, p, q)


def fill_rainbow():
    for i in range(NUM_LEDS):
        hue = (i * 256 // NUM_LEDS) % 256
        np[i] = hsv_to_rgb(hue, 255, 255)
    np.write()


while True:
    for color in COLORS:
        fill_solid(color)
        time.sleep(2)
    fill_rainbow()
    time.sleep(2)
