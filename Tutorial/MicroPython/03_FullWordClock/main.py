"""
Lesson 7 - Full Word Clock (MicroPython capstone)
-----------------------------------------------------
Everything from Lessons 2, 5, and 6 combined: light words on the grid,
driven by the real time synced over WiFi/NTP.

Deliberately the SIMPLEST version that tells time - one fixed colour
(white), no buttons, no light effects, no colour modes. For all of that,
see the full firmware at WordClock/WordClock.ino in the repo root
(Arduino/C++ - the full firmware is not ported to MicroPython).

  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
  r1      A  C  Q  U  A  R  T  E  R  D  C    A  QUARTER
  r2      T  W  E  N  T  Y  F  I  V  E  X    TWENTY  FIVE
  r3      H  A  L  F  S  T  E  N  F  T  O    HALF  TEN  TO
  r4      P  A  S  T  E  R  U  N  I  N  E    PAST  NINE
  r5      O  N  E  S  I  X  T  H  R  E  E    ONE  SIX  THREE
  r6      F  O  U  R  F  I  V  E  T  W  O    FOUR  FIVE  TWO
  r7      E  I  G  H  T  E  L  E  V  E  N    EIGHT  ELEVEN
  r8      S  E  V  E  N  T  W  E  L  V  E    SEVEN  TWELVE
  r9      T  E  N  S  E  O  C  L  O  C  K    TEN  O'CLOCK
"""

from machine import Pin
import neopixel
import network
import ntptime
import time

# ---------------------------------------------------------------------------
# USER CONFIG - edit these
# ---------------------------------------------------------------------------
WIFI_SSID = "YOUR_WIFI_SSID"
WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"
UTC_OFFSET_SEC = -5 * 3600     # US Eastern Standard Time; use -4*3600 during DST
NTP_HOST = "us.pool.ntp.org"

LED_PIN = 8
COLS = 11
ROWS = 10
NUM_WORD_LEDS = COLS * ROWS          # 110
NUM_DOTS = 4
NUM_LEDS = NUM_WORD_LEDS + NUM_DOTS  # 114
WHITE = (255, 255, 255)

np = neopixel.NeoPixel(Pin(LED_PIN), NUM_LEDS)

# ---------------------------------------------------------------------------
# Word table: (row, col, length) - same layout as the Arduino track / full firmware.
# ---------------------------------------------------------------------------
W_IT = (0, 0, 2)
W_IS = (0, 3, 2)
W_AM = (0, 7, 2)
W_PM = (0, 9, 2)
W_A = (1, 0, 1)
W_QUARTER = (1, 2, 7)
W_HALF = (3, 0, 4)
W_PAST = (4, 0, 4)
W_TO = (3, 9, 2)
W_OCLOCK = (9, 5, 6)
M_TWENTY = (2, 0, 6)
M_FIVE = (2, 6, 4)
M_TEN = (3, 5, 3)
HOURS = {
    1: (5, 0, 3), 2: (6, 8, 3), 3: (5, 6, 5), 4: (6, 0, 4),
    5: (6, 4, 4), 6: (5, 3, 3), 7: (8, 0, 5), 8: (7, 0, 5),
    9: (4, 7, 4), 10: (9, 0, 3), 11: (7, 5, 6), 12: (8, 5, 6),
}


def xy(row, col):
    return row * COLS + col


def light_word(word):
    row, col, length = word
    for i in range(length):
        np[xy(row, col + i)] = WHITE


def light_hour(h):
    light_word(HOURS[h])


def light_dots(n):
    for i in range(min(n, NUM_DOTS)):
        np[NUM_WORD_LEDS + i] = WHITE


def show_time(hour24, minute):
    np.fill((0, 0, 0))

    light_word(W_IT)
    light_word(W_IS)
    light_word(W_AM if hour24 < 12 else W_PM)

    bucket = minute // 5    # which 5-minute block
    rem = minute % 5        # leftover minutes -> corner dots

    h12 = hour24 % 12 or 12
    hnext = (hour24 + 1) % 12 or 12

    if bucket == 0:
        disp_hour = h12
        light_word(W_OCLOCK)
    else:
        past = bucket <= 6
        disp_hour = h12 if past else hnext

        if bucket == 1:
            light_word(M_FIVE)
        elif bucket == 2:
            light_word(M_TEN)
        elif bucket == 3:
            light_word(W_A)
            light_word(W_QUARTER)
        elif bucket == 4:
            light_word(M_TWENTY)
        elif bucket == 5:
            light_word(M_TWENTY)
            light_word(M_FIVE)
        elif bucket == 6:
            light_word(W_HALF)
        elif bucket == 7:
            light_word(M_TWENTY)
            light_word(M_FIVE)
        elif bucket == 8:
            light_word(M_TWENTY)
        elif bucket == 9:
            light_word(W_A)
            light_word(W_QUARTER)
        elif bucket == 10:
            light_word(M_TEN)
        elif bucket == 11:
            light_word(M_FIVE)

        light_word(W_PAST if past else W_TO)

    light_hour(disp_hour)
    light_dots(rem)
    np.write()


# ---------------------------------------------------------------------------
# WiFi + NTP - from Lessons 5 and 6.
# ---------------------------------------------------------------------------
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
    ntptime.host = NTP_HOST
    print("NTP syncing...")
    ntptime.settime()
    print("Time synced.")


def local_time():
    return time.localtime(time.time() + UTC_OFFSET_SEC)


connect_wifi()
sync_time()

last_minute = -1
while True:
    year, month, mday, hour, minute, second, weekday, yearday = local_time()
    if minute != last_minute:   # redraw only when the minute changes
        last_minute = minute
        show_time(hour, minute)
        print("Display {:02d}:{:02d}".format(hour, minute))
    time.sleep_ms(10)
