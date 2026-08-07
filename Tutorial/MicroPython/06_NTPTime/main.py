"""
Lesson 6 - NTP Time (MicroPython)
------------------------------------
Once online, fetch the real current time over the internet (NTP) and
print it every second. No panel rendering yet - that's Lesson 7.
"""

import network
import ntptime
import time

# ---------------------------------------------------------------------------
# USER CONFIG - edit these
# ---------------------------------------------------------------------------
WIFI_SSID = "YOUR_WIFI_SSID"
WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"

# US Eastern Time offset from UTC, in seconds. ntptime only gives you UTC -
# MicroPython has no built-in timezone/DST database, so we apply a fixed
# offset by hand. -5h = Eastern Standard Time, -4h = Eastern Daylight Time.
# Update this by hand around early March / early November, or extend this
# script yourself with the US DST rule if you want it automatic.
UTC_OFFSET_SEC = -5 * 3600

ntptime.host = "us.pool.ntp.org"


def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    print("WiFi connecting", end="")
    while not wlan.isconnected():
        print(".", end="")
        time.sleep_ms(250)
    print("\nWiFi OK, IP=", wlan.ifconfig()[0])


def sync_time():
    print("NTP syncing...")
    ntptime.settime()   # sets the board's RTC to UTC
    print("Time synced (UTC).")


def local_time():
    """Returns a MicroPython time tuple adjusted to US Eastern Time."""
    return time.localtime(time.time() + UTC_OFFSET_SEC)


connect_wifi()
sync_time()

while True:
    year, month, mday, hour, minute, second, weekday, yearday = local_time()
    print("Now: {:02d}:{:02d}:{:02d}".format(hour, minute, second))
    time.sleep(1)
