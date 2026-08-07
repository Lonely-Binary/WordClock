/*
 * Lesson 7 — Full Word Clock (capstone)
 * --------------------------------------
 * Everything from Lessons 2, 5, and 6 combined: light words on the grid,
 * driven by the real time synced over WiFi/NTP.
 *
 * Deliberately the SIMPLEST version that tells time — one fixed colour
 * (white), no buttons, no light effects, no colour modes. For all of that,
 * see the full firmware at WordClock/WordClock.ino in the repo root.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
 *   r1      A  C  Q  U  A  R  T  E  R  D  C    A  QUARTER
 *   r2      T  W  E  N  T  Y  F  I  V  E  X    TWENTY  FIVE
 *   r3      H  A  L  F  S  T  E  N  F  T  O    HALF  TEN  TO
 *   r4      P  A  S  T  E  R  U  N  I  N  E    PAST  NINE
 *   r5      O  N  E  S  I  X  T  H  R  E  E    ONE  SIX  THREE
 *   r6      F  O  U  R  F  I  V  E  T  W  O    FOUR  FIVE  TWO
 *   r7      E  I  G  H  T  E  L  E  V  E  N    EIGHT  ELEVEN
 *   r8      S  E  V  E  N  T  W  E  L  V  E    SEVEN  TWELVE
 *   r9      T  E  N  S  E  O  C  L  O  C  K    TEN  O'CLOCK
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>
#include <WiFi.h>
#include <time.h>

// ----------------------------------------------------------------------------
// USER CONFIG — edit these
// ----------------------------------------------------------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* TZ_INFO   = "EST5EDT,M3.2.0,M11.1.0";  // US Eastern Time, change for your own timezone
const char* NTP1 = "us.pool.ntp.org";
const char* NTP2 = "time.google.com";
const char* NTP3 = "time.nist.gov";

#define LED_PIN     8
#define COLS        11
#define ROWS        10
#define NUM_WORD_LEDS (COLS * ROWS)              // 110
#define NUM_DOTS      4
#define NUM_LEDS      (NUM_WORD_LEDS + NUM_DOTS)  // 114
#define BRIGHTNESS  80

CRGB leds[NUM_LEDS];
int  lastMinute = -1;

// ----------------------------------------------------------------------------
// Word table: {row, col, length} — from Lesson 2, same layout as the full firmware.
// ----------------------------------------------------------------------------
#define W_IT        0, 0, 2
#define W_IS        0, 3, 2
#define W_AM        0, 7, 2
#define W_PM        0, 9, 2
#define W_A         1, 0, 1
#define W_QUARTER   1, 2, 7
#define W_HALF      3, 0, 4
#define W_PAST      4, 0, 4
#define W_TO        3, 9, 2
#define W_OCLOCK    9, 5, 6
#define M_TWENTY    2, 0, 6
#define M_FIVE      2, 6, 4
#define M_TEN       3, 5, 3
#define H_ONE       5, 0, 3
#define H_TWO       6, 8, 3
#define H_THREE     5, 6, 5
#define H_FOUR      6, 0, 4
#define H_FIVE      6, 4, 4
#define H_SIX       5, 3, 3
#define H_SEVEN     8, 0, 5
#define H_EIGHT     7, 0, 5
#define H_NINE      4, 7, 4
#define H_TEN       9, 0, 3
#define H_ELEVEN    7, 5, 6
#define H_TWELVE    8, 5, 6

uint16_t XY(uint8_t row, uint8_t col) {
  return (uint16_t)row * COLS + col;
}

void lightWord(uint8_t row, uint8_t col, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) leds[XY(row, col + i)] = CRGB::White;
}

void lightHour(uint8_t h) {   // h in 1..12
  switch (h) {
    case 1:  lightWord(H_ONE);    break;
    case 2:  lightWord(H_TWO);    break;
    case 3:  lightWord(H_THREE);  break;
    case 4:  lightWord(H_FOUR);   break;
    case 5:  lightWord(H_FIVE);   break;
    case 6:  lightWord(H_SIX);    break;
    case 7:  lightWord(H_SEVEN);  break;
    case 8:  lightWord(H_EIGHT);  break;
    case 9:  lightWord(H_NINE);   break;
    case 10: lightWord(H_TEN);    break;
    case 11: lightWord(H_ELEVEN); break;
    case 12: lightWord(H_TWELVE); break;
  }
}

void lightDots(uint8_t n) {   // n = 0..4 leftover minutes
  for (uint8_t i = 0; i < n && i < NUM_DOTS; i++) leds[NUM_WORD_LEDS + i] = CRGB::White;
}

// ----------------------------------------------------------------------------
// Render the time onto the panel — same 5-minute-bucket logic as the full firmware.
// ----------------------------------------------------------------------------
void showTime(int hour24, int minute) {
  FastLED.clear();

  lightWord(W_IT);
  lightWord(W_IS);
  if (hour24 < 12) lightWord(W_AM); else lightWord(W_PM);

  int bucket = minute / 5;    // which 5-minute block
  int rem    = minute % 5;    // leftover minutes -> corner dots

  int h12   = hour24 % 12; if (h12   == 0) h12   = 12;
  int hnext = (hour24 + 1) % 12; if (hnext == 0) hnext = 12;
  int dispHour;

  if (bucket == 0) {
    dispHour = h12;
    lightWord(W_OCLOCK);
  } else {
    bool past = (bucket <= 6);
    dispHour  = past ? h12 : hnext;

    switch (bucket) {
      case 1:  lightWord(M_FIVE);                       break;
      case 2:  lightWord(M_TEN);                        break;
      case 3:  lightWord(W_A); lightWord(W_QUARTER);    break;
      case 4:  lightWord(M_TWENTY);                     break;
      case 5:  lightWord(M_TWENTY); lightWord(M_FIVE);  break;
      case 6:  lightWord(W_HALF);                       break;
      case 7:  lightWord(M_TWENTY); lightWord(M_FIVE);  break;
      case 8:  lightWord(M_TWENTY);                     break;
      case 9:  lightWord(W_A); lightWord(W_QUARTER);    break;
      case 10: lightWord(M_TEN);                        break;
      case 11: lightWord(M_FIVE);                       break;
    }
    if (past) lightWord(W_PAST); else lightWord(W_TO);
  }

  lightHour(dispHour);
  lightDots(rem);
  FastLED.show();
}

// ----------------------------------------------------------------------------
// WiFi + NTP — from Lessons 5 and 6.
// ----------------------------------------------------------------------------
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(250);
  }
  Serial.printf("\nWiFi OK, IP=%s\n", WiFi.localIP().toString().c_str());
}

void syncTime() {
  configTzTime(TZ_INFO, NTP1, NTP2, NTP3);
  Serial.print("NTP syncing");
  struct tm t;
  while (!getLocalTime(&t, 500)) Serial.print('.');
  Serial.println("\nTime synced.");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  connectWiFi();
  syncTime();
}

void loop() {
  struct tm t;
  if (getLocalTime(&t, 0)) {
    if (t.tm_min != lastMinute) {   // redraw only when the minute changes
      lastMinute = t.tm_min;
      showTime(t.tm_hour, t.tm_min);
      Serial.printf("Display %02d:%02d\n", t.tm_hour, t.tm_min);
    }
  }
  delay(10);
}
