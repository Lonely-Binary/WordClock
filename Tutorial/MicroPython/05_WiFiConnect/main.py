"""
Lesson 5 - WiFi Connect (MicroPython)
----------------------------------------
Connect the ESP32-S3 to your WiFi network, with a small LED sweep
animation on the panel while it connects.
"""

from machine import Pin
import neopixel
import network
import time

# ---------------------------------------------------------------------------
# USER CONFIG - edit these
# ---------------------------------------------------------------------------
WIFI_SSID = "YOUR_WIFI_SSID"
WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"

LED_PIN = 8
COLS = 11
NUM_LEDS = 114

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)


def xy(row, col):
    return row * COLS + col


def status_sweep(color, position):
    """A small progress sweep across row 0 while we wait for something."""
    np.fill((0, 0, 0))
    np[xy(0, position % COLS)] = color
    np.write()


def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    print("WiFi connecting", end="")
    p = 0
    while not wlan.isconnected():
        status_sweep((0, 40, 80), p)   # blue-ish sweep = joining WiFi
        p += 1
        print(".", end="")
        time.sleep_ms(250)
    print("\nWiFi OK, IP=", wlan.ifconfig()[0])
    return wlan


connect_wifi()

# Show solid green once connected.
np.fill((0, 255, 0))
np.write()
