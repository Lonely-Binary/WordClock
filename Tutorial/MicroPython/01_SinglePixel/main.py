"""
Lesson 1 - Single Pixel (MicroPython)
--------------------------------------
The "hello world" of the word clock panel: light up exactly one LED.
If this works, your wiring, power, and MicroPython install are all good.

Board: ESP32-S3 running MicroPython
Power: flash via the dev board's Type-C port, then power the panel from
       the BASE's Type-C port to actually see the LED - the dev board's
       own USB port can't supply enough current for the panel.
"""

from machine import Pin
import neopixel

LED_PIN = 8        # WS2812 data line
NUM_LEDS = 114      # 110 letters + 4 corner dots

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)

np.fill((0, 0, 0))            # turn every LED off first
np[0] = (255, 255, 255)       # LED index 0 = top-left letter, "I"
np.write()                    # nothing changes on the panel until you call write()
