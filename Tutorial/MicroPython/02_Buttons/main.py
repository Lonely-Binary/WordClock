"""
Lesson 4 - Buttons (MicroPython)
-----------------------------------
Read the two onboard buttons and use one of them to cycle the panel's
colour on each press.

BUTTONS (both ACTIVE-HIGH -> reads 1 when pressed; internal pull-down on):
  GPIO 42 = COLOR button (used in this lesson)
  GPIO 41 = EFFECT button (wired up, but unused until Lesson 7 / the full firmware)
"""

from machine import Pin
import neopixel
import time

LED_PIN = 8
NUM_LEDS = 114

BTN_COLOR = 42
BTN_EFFECT = 41
DEBOUNCE_MS = 40

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)
btn_color = Pin(BTN_COLOR, Pin.IN)
btn_effect = Pin(BTN_EFFECT, Pin.IN)

COLORS = [(255, 255, 255), (255, 0, 0), (0, 0, 255), (0, 255, 0)]
color_mode = 0


def fill_current_color():
    np.fill(COLORS[color_mode])
    np.write()


class Debouncer:
    """Fires once per physical press, ignoring contact bounce."""

    def __init__(self, pin, debounce_ms=DEBOUNCE_MS):
        self.pin = pin
        self.debounce_ms = debounce_ms
        self.prev = False
        self.last_t = 0

    def pressed(self):
        now = self.pin.value() == 1
        fired = False
        if now != self.prev and time.ticks_diff(time.ticks_ms(), self.last_t) >= self.debounce_ms:
            self.last_t = time.ticks_ms()
            if now:
                fired = True   # rising edge = press
            self.prev = now
        return fired


color_button = Debouncer(btn_color)

fill_current_color()

while True:
    if color_button.pressed():
        color_mode = (color_mode + 1) % len(COLORS)
        print("Color mode ->", color_mode)
        fill_current_color()

    time.sleep_ms(10)   # poll roughly every 10ms - responsive without hogging the CPU
