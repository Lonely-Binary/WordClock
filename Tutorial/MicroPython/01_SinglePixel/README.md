# Lesson 1 — Single Pixel (MicroPython)

**Goal:** light up exactly one LED on the panel. This is the "does my hardware work" check — if this lesson lights up, your wiring, power, and MicroPython setup are all correct, and you're ready for everything else in this series.

## What you'll learn

- Flashing MicroPython firmware onto the ESP32-S3 and running a script on it
- The built-in `neopixel` module: the three things every WS2812 script needs — a `Pin`, a `NeoPixel` object, and `.write()`
- Why nothing appears on the panel until you call `.write()`

## Before you start

1. **Flash MicroPython firmware** onto the board (one-time setup, if it isn't already running MicroPython):
   - Download the ESP32-S3 `.bin` firmware from [micropython.org/download/](https://micropython.org/download/).
   - Install `esptool`: `pip install esptool`
   - Erase and flash (adjust the port for your machine):
     ```bash
     esptool.py --chip esp32s3 --port /dev/tty.usbmodemXXXX erase_flash
     esptool.py --chip esp32s3 --port /dev/tty.usbmodemXXXX write_flash -z 0x0 ESP32_GENERIC_S3-*.bin
     ```
2. **Install [Thonny](https://thonny.org/)** (or use `mpremote`/`rshell`) — the easiest way to run scripts on the board and see a Python REPL.
3. In Thonny, set the interpreter to "MicroPython (ESP32)" and select the board's serial port.
4. **Flash over the dev board's own Type-C port.** To actually *see* the LED light up, power the unit from the **base's Type-C port** instead — the dev board's USB port alone can't supply enough current for the panel. This is expected, not a fault (see the [root README](../../../README.md) for the full explanation).

## The code

See [`main.py`](main.py). Open it in Thonny and click "Run" (or copy it onto the board as `main.py` so it runs automatically on boot).

```python
from machine import Pin
import neopixel

LED_PIN = 8
NUM_LEDS = 114

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)

np.fill((0, 0, 0))
np[0] = (255, 255, 255)
np.write()
```

## Line by line

- `LED_PIN = 8` — the panel's WS2812 data line is wired to GPIO 8 on this board. Every lesson in this series uses this same pin.
- `NUM_LEDS = 114` — the panel has 110 letter LEDs (11 columns × 10 rows) plus 4 corner "minute dot" LEDs, wired in one single chain: 114 total.
- `neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)` — MicroPython's built-in driver for WS2812/NeoPixel-style LEDs. It manages a buffer the same size as your LED count.
- `np.fill((0, 0, 0))` — sets every pixel to black (off) in the buffer, but doesn't push it to the panel yet.
- `np[0] = (255, 255, 255)` — sets just pixel 0 to white (as an `(r, g, b)` tuple, 0–255 each). Still only a change in memory.
- `np.write()` — this is the line that actually pushes the buffer out over GPIO 8 to the physical LEDs. **Nothing lights up without this call.**

## What you should see

The top-left letter cell ("I") lit up white. Everything else on the panel stays dark.

## Troubleshooting

- **`ImportError: no module named 'neopixel'`**: your firmware build is missing it — reflash with the standard ESP32-S3 firmware from micropython.org, which includes it.
- **Nothing lights up at all**: are you powering from the *base's* Type-C port, not the dev board's? The dev board's own USB port can't drive the panel.
- **Wrong colours**: MicroPython's `neopixel` module already handles the WS2812 GRB byte order internally — just pass colours as `(r, g, b)` and it takes care of the rest.

## Next

[Lesson 2 — Light a Word](../02_LightAWord/README.md): stop thinking in single LED indices and start thinking in rows/columns/words.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
