/*
 * WordClock — QLOCKTWO-style English WS2812 word clock
 * ----------------------------------------------------
 * Board  : ESP32-S3 (GPIO 8 data, GPIO 41/42 buttons — these pins only exist
 *          on the S3, so build for esp32:esp32:esp32s3).
 * LEDs   : WS2812B on GPIO 8, driven by FastLED.
 *
 * WiFi & timezone setup (this firmware is flashed identically to every unit,
 * so WiFi/timezone can't be compile-time constants — they're set per-device
 * at first boot):
 *   - No saved WiFi -> a one-time factory QC self-test runs first (solid
 *     RED -> GREEN -> BLUE across the whole panel, 1s each at half
 *     brightness, to catch a dead/miswired WS2812 pixel before it ships —
 *     see runFactoryLedTest()), then the panel shows a low-power
 *     confetti/sparkle animation (random single LEDs, fading — not a
 *     full-grid rainbow, to keep current draw down) and broadcasts its own
 *     hotspot, "WordClock-XXXX" (XXXX = 4 hex digits from the chip's efuse
 *     ID), open/no password. Connect with a phone, a captive portal page
 *     opens (or visit http://192.168.4.1) branded with Lonely Binary, where
 *     you enter your WiFi and pick a timezone from a worldwide dropdown
 *     (China automatically syncs from China-reachable NTP servers instead
 *     of the global default — see TZ_OPTIONS). Powered by the WiFiManager
 *     library (tzapu) — see WordClock/README.md for the full first-boot flow.
 *   - Time syncs from NTP once connected (global default servers, or a
 *     region-specific override — currently just China — from TZ_OPTIONS).
 *
 * STARTUP INDICATORS (so the panel is never blank while it's busy):
 *   BLUE snake = joining WiFi
 *   RED  snake = connected, waiting for the NTP time sync to land
 *   confetti   = unconfigured, captive portal is open
 *   - RESET GESTURE: hold BOTH buttons (GPIO 41 + 42) for 3 seconds, at any
 *     time, to forget the saved WiFi/timezone and return to setup mode.
 *   - OTA: once connected, the device appears as a network port in the
 *     Arduino IDE (hostname = the same "WordClock-XXXX"), no OTA password.
 *
 * BUTTONS (both ACTIVE-HIGH -> reads HIGH when pressed; internal pull-down on):
 *   GPIO 42  = COLOR : cycles White -> Red -> Blue -> Green -> Rainbow
 *                      (Rainbow = a random colour per lit LED)
 *   GPIO 41  = EFFECT: plays a random animation, then returns to the time
 *   BOTH held 3s = factory reset (see above)
 *
 * PANEL LAYOUT (11 columns x 10 rows = 110 letter LEDs, then 4 dot LEDs = 114)
 * Wiring order is row-major: LED0 = top-left "I", LED10 = top-right "M",
 * LED11 = start of row 1 ("A"), ... LED109 = bottom-right "K".
 * The last 4 LEDs (110..113) are the corner dots = +1,+2,+3,+4 minutes.
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
 * Reading (classic English):
 *   :00  IT IS <hour> O'CLOCK
 *   :05  IT IS FIVE PAST <hour>          :35  IT IS TWENTY FIVE TO <hour+1>
 *   :10  IT IS TEN PAST <hour>           :40  IT IS TWENTY TO <hour+1>
 *   :15  IT IS A QUARTER PAST <hour>     :45  IT IS A QUARTER TO <hour+1>
 *   :20  IT IS TWENTY PAST <hour>        :50  IT IS TEN TO <hour+1>
 *   :25  IT IS TWENTY FIVE PAST <hour>   :55  IT IS FIVE TO <hour+1>
 *   :30  IT IS HALF PAST <hour>
 * The 4 corner dots show the leftover 1-4 minutes inside the 5-minute block.
 *
 * If alternate rows look mirrored on your hardware, set SERPENTINE to 1.
 *
 * Libraries: FastLED 3.6.0+, WiFiManager (tzapu) 2.0.16+
 *            (ArduinoOTA / Preferences ship with the ESP32 Arduino core)
 */

#include <FastLED.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <time.h>

// ----------------------------------------------------------------------------
// USER CONFIG — edit these
// ----------------------------------------------------------------------------
#define LED_PIN     8            // WS2812 data pin
#define BTN_COLOR   42           // active-high: cycles colour
#define BTN_EFFECT  41           // active-high: random effect
#define BRIGHTNESS  80           // 0-255
#define SHOW_AMPM   1            // 1 = light the AM/PM cells in the top row

// Set to 1 if every ODD row is wired right-to-left (boustrophedon / serpentine).
#define SERPENTINE  0

// How long (ms) both buttons must be held together to trigger a factory reset.
#define RESET_HOLD_MS 3000

// How long (ms) to wait after a config-portal "Save" before deciding the
// connection attempt failed. Must exceed wm.setConnectTimeout() below (15s),
// with margin for the retry the library does internally.
#define SAVE_CONNECT_TIMEOUT_MS 25000

// If the buyer submits a failing network and then walks away, don't sit with a
// quiesced STA forever — reboot after this long so a unit that DOES have good
// saved credentials (router was merely rebooting) resumes retrying them.
#define PORTAL_IDLE_RESTART_MS  300000   // 5 minutes

// Default NTP servers — globally reachable almost everywhere. China is the
// known exception (see TZ_OPTIONS' China entry, which overrides these).
const char* NTP1 = "pool.ntp.org";
const char* NTP2 = "time.google.com";
const char* NTP3 = "time.cloudflare.com";

// ----------------------------------------------------------------------------
// LED geometry
// ----------------------------------------------------------------------------
#define COLS          11
#define ROWS          10
#define NUM_WORD_LEDS (COLS * ROWS)               // 110
#define NUM_DOTS      4
#define NUM_LEDS      (NUM_WORD_LEDS + NUM_DOTS)   // 114

CRGB leds[NUM_LEDS];

// ----------------------------------------------------------------------------
// WiFiManager / device identity / timezone globals — declared early because
// checkResetGesture() (further down) needs `wm` and `prefs` before the
// "WiFi + timezone setup" section where they'd otherwise naturally live.
// Arduino auto-generates function prototypes for you, but NOT variable
// ordering, so these must appear before any function body that uses them.
// ----------------------------------------------------------------------------
WiFiManager wm;
Preferences prefs;
String apName;

// ntp1/ntp2/ntp3 let a specific timezone override the global default NTP
// servers below — nullptr means "use the global default". Only China needs
// this today: pool.ntp.org / time.google.com / time.cloudflare.com are
// globally reachable almost everywhere EXCEPT mainland China, where they're
// unreliable or blocked outright, so that entry points at Chinese NTP
// servers instead. Add more per-region overrides here later the same way.
struct TzOption {
  const char* label;
  const char* posix;
  const char* ntp1;
  const char* ntp2;
  const char* ntp3;
};

const TzOption TZ_OPTIONS[] = {
  // Americas
  { "US Pacific (Los Angeles, UTC-8)",  "PST8PDT,M3.2.0,M11.1.0", nullptr, nullptr, nullptr },
  { "US Mountain (Denver, UTC-7)",      "MST7MDT,M3.2.0,M11.1.0", nullptr, nullptr, nullptr },
  { "US Central (Chicago, UTC-6)",      "CST6CDT,M3.2.0,M11.1.0", nullptr, nullptr, nullptr },
  { "US Eastern (New York, UTC-5)",     "EST5EDT,M3.2.0,M11.1.0", nullptr, nullptr, nullptr },
  { "Brazil (Sao Paulo, UTC-3)",        "BRT3",                   nullptr, nullptr, nullptr },
  // Europe / Africa
  { "UK (London, UTC+0)",               "GMT0BST,M3.5.0/1,M10.5.0",   nullptr, nullptr, nullptr },
  { "Central Europe (Berlin, UTC+1)",   "CET-1CEST,M3.5.0,M10.5.0/3", nullptr, nullptr, nullptr },
  { "Eastern Europe (Athens, UTC+2)",   "EET-2EEST,M3.5.0/3,M10.5.0/4", nullptr, nullptr, nullptr },
  { "Russia (Moscow, UTC+3)",           "MSK-3",                   nullptr, nullptr, nullptr },
  { "Gulf (Dubai, UTC+4)",              "GST-4",                   nullptr, nullptr, nullptr },
  // South / Southeast Asia
  { "India (New Delhi, UTC+5:30)",      "IST-5:30", nullptr, nullptr, nullptr },
  { "Indochina (Bangkok/Jakarta, UTC+7)", "ICT-7",  nullptr, nullptr, nullptr },
  { "Singapore / Malaysia (UTC+8)",     "SGT-8",    nullptr, nullptr, nullptr },
  // East Asia — China gets its own NTP servers, everything else here doesn't
  { "China (Beijing/Shanghai, UTC+8)",  "CST-8", "ntp.aliyun.com", "cn.pool.ntp.org", "ntp.ntsc.ac.cn" },
  { "Japan / Korea (Tokyo/Seoul, UTC+9)", "JST-9", nullptr, nullptr, nullptr },
  // Oceania
  { "Australia East (Sydney, UTC+10)",  "AEST-10AEDT,M10.1.0,M4.1.0/3", nullptr, nullptr, nullptr },
  { "New Zealand (Auckland, UTC+12)",   "NZST-12NZDT,M9.5.0,M4.1.0/3",  nullptr, nullptr, nullptr },
  // Fallback
  { "UTC (no offset)",                  "UTC0", nullptr, nullptr, nullptr },
};
const uint8_t NUM_TZ_OPTIONS = sizeof(TZ_OPTIONS) / sizeof(TZ_OPTIONS[0]);

char   currentTzInfo[40] = "UTC0";  // default before the buyer picks a real one (see TZ_OPTIONS)
String tzSelectHtml;                 // holds the <select> markup for the portal

// Config-portal provisioning state (see handleProvisioningFailure()).
// 0 = nothing pending; otherwise the millis() timestamp of the event.
uint32_t saveAttemptAt   = 0;   // buyer hit "Save"; waiting to see if it connects
uint32_t failedAttemptAt = 0;   // that attempt failed; STA quiesced, awaiting a retry

// ----------------------------------------------------------------------------
// Colour modes (cycled by the COLOR button)
// ----------------------------------------------------------------------------
enum { COL_WHITE = 0, COL_RED, COL_BLUE, COL_GREEN, COL_RAINBOW, NUM_COLOR_MODES };
uint8_t colorMode = COL_WHITE;

// State shared between loop() and the button/effect handlers.
int  lastMinute  = -1;
bool wasConnected = false;
// False until SNTP has actually set the clock. Drives the red "syncing"
// snake, and deliberately NOT reset when WiFi drops later: the ESP32 keeps
// its own clock running, so a mid-life outage shouldn't blank the display.
bool timeSynced   = false;

// Return the colour for one LED index under the current mode.
// Rainbow mode gives every lit LED its own random colour.
CRGB pixelColor() {
  switch (colorMode) {
    case COL_WHITE:   return CRGB(255, 255, 255);
    case COL_RED:     return CRGB::Red;
    case COL_BLUE:    return CRGB::Blue;
    case COL_GREEN:   return CRGB::Green;
    case COL_RAINBOW: return (CRGB)CHSV(random8(), 255, 255);
  }
  return CRGB::White;
}

void fillAll(CRGB c) {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
}

// ----------------------------------------------------------------------------
// Word table: {row, col, length}. Every word is horizontal and contiguous.
// ----------------------------------------------------------------------------
#define W_IT        0, 0, 2
#define W_IS        0, 3, 2
#define W_AM        0, 7, 2      // A M
#define W_PM        0, 9, 2      // P M

#define W_A         1, 0, 1      // "A" (for "A QUARTER")
#define W_QUARTER   1, 2, 7
#define W_HALF      3, 0, 4
#define W_PAST      4, 0, 4
#define W_TO        3, 9, 2
#define W_OCLOCK    9, 5, 6

// minute words
#define M_TWENTY    2, 0, 6
#define M_FIVE      2, 6, 4      // FIVE inside "TWENTY FIVE" (also 5 past/to)
#define M_TEN       3, 5, 3

// hour words (1..12)
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

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
uint16_t XY(uint8_t row, uint8_t col) {
#if SERPENTINE
  if (row & 0x01) col = COLS - 1 - col;
#endif
  return (uint16_t)row * COLS + col;
}

void lightWord(uint8_t row, uint8_t col, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    leds[XY(row, col + i)] = pixelColor();
  }
}

void lightHour(uint8_t h) {            // h in 1..12
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

void lightDots(uint8_t n) {            // light n (0..4) corner dots
  for (uint8_t i = 0; i < n && i < NUM_DOTS; i++) {
    leds[NUM_WORD_LEDS + i] = pixelColor();
  }
}

// ----------------------------------------------------------------------------
// Render the time onto the panel
// ----------------------------------------------------------------------------
void showTime(int hour24, int minute) {
  FastLED.clear();

  lightWord(W_IT);
  lightWord(W_IS);
#if SHOW_AMPM
  if (hour24 < 12) lightWord(W_AM); else lightWord(W_PM);
#endif

  int bucket = minute / 5;             // 0..11  (which 5-minute block)
  int rem    = minute % 5;             // 0..4   (leftover -> corner dots)

  int h12   = hour24 % 12; if (h12   == 0) h12   = 12;
  int hnext = (hour24 + 1) % 12; if (hnext == 0) hnext = 12;
  int dispHour;

  if (bucket == 0) {
    dispHour = h12;
    lightWord(W_OCLOCK);
  } else {
    bool past = (bucket <= 6);         // 5..30 -> PAST, 35..55 -> TO
    dispHour  = past ? h12 : hnext;

    switch (bucket) {
      case 1:  lightWord(M_FIVE);                       break; // 5  past
      case 2:  lightWord(M_TEN);                        break; // 10 past
      case 3:  lightWord(W_A); lightWord(W_QUARTER);    break; // 15 past
      case 4:  lightWord(M_TWENTY);                     break; // 20 past
      case 5:  lightWord(M_TWENTY); lightWord(M_FIVE);  break; // 25 past
      case 6:  lightWord(W_HALF);                       break; // 30 past
      case 7:  lightWord(M_TWENTY); lightWord(M_FIVE);  break; // 25 to
      case 8:  lightWord(M_TWENTY);                     break; // 20 to
      case 9:  lightWord(W_A); lightWord(W_QUARTER);    break; // 15 to
      case 10: lightWord(M_TEN);                        break; // 10 to
      case 11: lightWord(M_FIVE);                       break; // 5  to
    }
    if (past) lightWord(W_PAST); else lightWord(W_TO);
  }

  lightHour(dispHour);
  lightDots(rem);
  FastLED.show();
}

// Redraw the current time immediately (e.g. after a colour change / effect).
void refreshTime() {
  struct tm t;
  if (getLocalTime(&t, 10)) showTime(t.tm_hour, t.tm_min);
}

// ----------------------------------------------------------------------------
// Random effects (EFFECT button) — each runs a few seconds, then time returns
// ----------------------------------------------------------------------------
void fxConfetti(uint16_t ms) {
  uint32_t end = millis() + ms;
  while (millis() < end) {
    fadeToBlackBy(leds, NUM_LEDS, 18);
    leds[random16(NUM_LEDS)] += CHSV(random8(), 200, 255);
    FastLED.show();
    delay(18);
  }
}

// A short rainbow-coloured comet chasing around the strip, fading tail
// behind it. Only ~10 LEDs are ever lit at once (vs. every LED for a
// full-grid rainbow fill), so it draws far less current for the same
// "colourful and moving" effect.
void fxRainbowComet(uint16_t ms) {
  const uint8_t cometLen = 10;
  uint32_t end = millis() + ms;
  uint8_t  hueBase = 0;
  uint16_t pos = 0;
  while (millis() < end) {
    fadeToBlackBy(leds, NUM_LEDS, 40);
    for (uint8_t i = 0; i < cometLen; i++) {
      leds[(pos + i) % NUM_LEDS] = CHSV(hueBase + i * 8, 255, 255);
    }
    FastLED.show();
    delay(25);
    pos = (pos + 1) % NUM_LEDS;
    hueBase += 4;
  }
}

void fxColorWipe(uint16_t ms) {
  (void)ms;
  CRGB c = (CRGB)CHSV(random8(), 255, 255);
  FastLED.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = c;
    FastLED.show();
    delay(10);
  }
  delay(300);
}

void fxTheaterChase(uint16_t ms) {
  uint32_t end = millis() + ms;
  uint8_t hue = 0, q = 0;
  while (millis() < end) {
    FastLED.clear();
    for (int i = q; i < NUM_LEDS; i += 3) leds[i] = CHSV(hue, 255, 255);
    FastLED.show();
    delay(55);
    q = (q + 1) % 3;
    hue += 8;
  }
}

void fxSparkle(uint16_t ms) {
  uint32_t end = millis() + ms;
  while (millis() < end) {
    fadeToBlackBy(leds, NUM_LEDS, 40);
    leds[random16(NUM_LEDS)] = CRGB::White;
    FastLED.show();
    delay(28);
  }
}

void playRandomEffect() {
  switch (random8(5)) {
    case 0: fxConfetti(3000);     break;
    case 1: fxRainbowComet(3000); break;
    case 2: fxColorWipe(0);       break;
    case 3: fxTheaterChase(3000); break;
    case 4: fxSparkle(3000);      break;
  }
  FastLED.clear();
  FastLED.show();
  refreshTime();                 // straight back to normal time
}

// A non-blocking, single-frame version of fxConfetti()'s animation — same
// fadeToBlackBy()+random-pixel body, just paced with millis() instead of a
// blocking while-loop, so it can run every loop() iteration while WiFi is
// unconfigured without freezing button polling or the captive portal's web
// server. Deliberately confetti, not a full-grid rainbow fill: lighting all
// 114 LEDs at once draws a lot more current for no extra visual information
// ("still needs setup"), and a full-panel wash doesn't read as an animation
// at a glance the way scattered sparkles do.
void setupIndicatorFrame() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  if (now - lastFrame < 18) return;    // same ~18ms pacing as fxConfetti()
  lastFrame = now;
  fadeToBlackBy(leds, NUM_LEDS, 18);
  leds[random16(NUM_LEDS)] += CHSV(random8(), 200, 255);
  FastLED.show();
}

// ----------------------------------------------------------------------------
// Startup progress indicators — a crawling snake, so the panel is never dark
// while the clock is busy doing something the buyer can't see:
//   BLUE snake = joining WiFi
//   RED  snake = connected, waiting for the NTP time sync to land
// Without these the panel sits blank for seconds at a time and reads as
// broken hardware.
//
// The blue one has to run on its own FreeRTOS task, because wm.autoConnect()
// BLOCKS while it tries the saved network (up to setConnectTimeout seconds)
// and that happens inside setup(), before loop() ever runs. The red one is
// the opposite: by then we're in loop(), which must stay non-blocking for
// OTA and the buttons, so it's a millis()-paced single frame instead.
// ----------------------------------------------------------------------------
#define SNAKE_LEN       8
#define SNAKE_FRAME_MS  45
#define SNAKE_BRIGHT    128    // 50% of full (before FastLED's global BRIGHTNESS)

TaskHandle_t connectAnimHandle = NULL;

// One frame of the snake, in whatever colour. It crawls a boustrophedon path
// over the grid (left-to-right, then right-to-left on the next row) so it
// reads as one continuous crawl rather than jumping back at every row end.
// Note this is the VISUAL path — XY() separately handles however the panel
// is physically wired.
void drawSnakeFrame(const CRGB& base) {
  static uint16_t head = 0;
  FastLED.clear();
  for (uint8_t i = 0; i < SNAKE_LEN; i++) {
    uint16_t p   = (head + NUM_WORD_LEDS - i) % NUM_WORD_LEDS;
    uint8_t  row = p / COLS;
    uint8_t  col = p % COLS;
    if (row & 0x01) col = COLS - 1 - col;         // crawl back along odd rows
    uint8_t bright = SNAKE_BRIGHT - (i * (SNAKE_BRIGHT / SNAKE_LEN));  // fading tail
    CRGB c = base;
    c.nscale8_video(bright);
    leds[XY(row, col)] = c;
  }
  FastLED.show();
  head = (head + 1) % NUM_WORD_LEDS;
}

// Red snake while NTP is still syncing. Called from loop(), so it's paced
// with millis() rather than blocking.
void ntpSyncFrame() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  if (now - lastFrame < SNAKE_FRAME_MS) return;
  lastFrame = now;
  drawSnakeFrame(CRGB::Red);
}

void connectAnimTask(void* pv) {
  (void)pv;
  for (;;) {
    drawSnakeFrame(CRGB::Blue);
    vTaskDelay(pdMS_TO_TICKS(SNAKE_FRAME_MS));
  }
}

void startConnectingAnimation() {
  if (connectAnimHandle) return;
  xTaskCreate(connectAnimTask, "connAnim", 4096, NULL, 1, &connectAnimHandle);
}

// Must be called before loop() starts drawing, so only one thing ever owns
// the LEDs at a time.
void stopConnectingAnimation() {
  if (!connectAnimHandle) return;
  vTaskDelete(connectAnimHandle);
  connectAnimHandle = NULL;
  delay(SNAKE_FRAME_MS * 2);   // let any in-flight FastLED.show() finish
  FastLED.clear(true);
}

// ----------------------------------------------------------------------------
// Buttons (active-high, internal pull-down). React on the press edge, then
// lock out for DEBOUNCE_MS to swallow contact bounce.
// ----------------------------------------------------------------------------
#define DEBOUNCE_MS 40
bool     btnColorPrev  = false, btnEffectPrev = false;
uint32_t btnColorT     = 0,     btnEffectT    = 0;

bool pressedEdge(uint8_t pin, bool &prev, uint32_t &tLast) {
  bool now   = (digitalRead(pin) == HIGH);
  bool fired = false;
  if (now != prev && (millis() - tLast) >= DEBOUNCE_MS) {
    tLast = millis();
    if (now) fired = true;       // rising edge = press
    prev = now;
  }
  return fired;
}

bool comboActive() {
  return (digitalRead(BTN_COLOR) == HIGH) && (digitalRead(BTN_EFFECT) == HIGH);
}

void pollButtons() {
  // Compute both edges unconditionally so the debounce state stays correct,
  // but suppress the single-button action while both buttons are held —
  // that's very likely the start of the 3s reset gesture, not a colour/effect
  // request. Best-effort: if the two presses land more than ~10ms apart this
  // can still let one action slip through, which is harmless.
  bool combo         = comboActive();
  bool colorPressed  = pressedEdge(BTN_COLOR,  btnColorPrev,  btnColorT);
  bool effectPressed = pressedEdge(BTN_EFFECT, btnEffectPrev, btnEffectT);

  if (colorPressed && !combo) {
    colorMode = (colorMode + 1) % NUM_COLOR_MODES;
    Serial.printf("Color mode -> %d\n", colorMode);
    refreshTime();               // redraw immediately in the new colour
  }
  if (effectPressed && !combo) {
    Serial.println("Random effect!");
    playRandomEffect();
  }
}

// ----------------------------------------------------------------------------
// Factory reset gesture: hold both buttons for RESET_HOLD_MS, from any state.
// ----------------------------------------------------------------------------
uint32_t comboHeldSince = 0;
bool     resetTriggered = false;

// Brief red blink: the WiFi credentials just submitted in the portal didn't
// connect. Blocking for ~1s is fine here — it's a one-shot failure path, and
// the portal's web server tolerates the short stall.
void flashConnectFailed() {
  for (uint8_t i = 0; i < 3; i++) {
    fillAll(CRGB::Red);   FastLED.show(); delay(150);
    fillAll(CRGB::Black); FastLED.show(); delay(150);
  }
}

void flashResetAck() {
  for (uint8_t i = 0; i < 3; i++) {
    fillAll(CRGB::White); FastLED.show(); delay(150);
    fillAll(CRGB::Black); FastLED.show(); delay(150);
  }
}

void checkResetGesture() {
  if (comboActive()) {
    if (comboHeldSince == 0) {
      comboHeldSince = millis();
    } else if (!resetTriggered && millis() - comboHeldSince >= RESET_HOLD_MS) {
      resetTriggered = true;
      Serial.println("Reset gesture held 3s -> clearing WiFi/timezone and restarting...");
      flashResetAck();
      prefs.remove("tz");
      prefs.remove("cfgd");      // back to "never configured": factory self-test runs again
      wm.resetSettings();        // clears WiFiManager's saved WiFi credentials
      delay(300);
      ESP.restart();
    }
  } else {
    comboHeldSince = 0;
    resetTriggered = false;
  }
}

// ----------------------------------------------------------------------------
// WiFi + timezone setup (WiFiManager: SoftAP + captive portal)
// `wm` / `prefs` / `apName` / TZ_OPTIONS / currentTzInfo / tzSelectHtml are
// declared earlier, right after `leds[]` — see the comment there.
// ----------------------------------------------------------------------------

// Build "WordClock-XXXX" from the chip's factory-burned efuse ID, once at
// boot. Reused as both the setup hotspot's SSID and the OTA hostname.
//
// Deliberately uses ESP.getEfuseMac() instead of WiFi.macAddress(): the
// latter depends on the WiFi driver already being initialized, and on
// hardware this gave a DIFFERENT "unique" suffix on every boot of the same
// chip (confirmed: WiFi.mode(WIFI_STA) alone wasn't even enough — it just
// made it consistently read back as all-zero). getEfuseMac() reads straight
// from efuse hardware, so it's available immediately and never changes.
void buildApName() {
  uint64_t chipId = ESP.getEfuseMac();
  char buf[24];
  snprintf(buf, sizeof(buf), "WordClock-%04X", (uint16_t)(chipId & 0xFFFF));
  apName = String(buf);
}

void loadTimezone() {
  String saved = prefs.getString("tz", currentTzInfo);
  saved.toCharArray(currentTzInfo, sizeof(currentTzInfo));
}

// Finds which TZ_OPTIONS entry matches a given POSIX TZ string, if any.
// Matching by content (not a stored array index) so a future firmware
// update that reorders/extends TZ_OPTIONS can't silently remap someone's
// saved timezone to the wrong entry.
int findTzOptionIndex(const char* posix) {
  for (uint8_t i = 0; i < NUM_TZ_OPTIONS; i++) {
    if (strcmp(TZ_OPTIONS[i].posix, posix) == 0) return i;
  }
  return -1;
}

String buildTzSelectHtml() {
  String html = "<br/><label for='tz'>Timezone</label><select name='tz' id='tz'>";
  for (uint8_t i = 0; i < NUM_TZ_OPTIONS; i++) {
    html += "<option value='";
    html += TZ_OPTIONS[i].posix;
    html += "'";
    if (strcmp(TZ_OPTIONS[i].posix, currentTzInfo) == 0) html += " selected";
    html += ">";
    html += TZ_OPTIONS[i].label;
    html += "</option>";
  }
  html += "</select>";
  return html;
}

// Reads the submitted "tz" field straight from the portal's web server —
// WiFiManagerParameter only renders plain text inputs natively, so the
// timezone dropdown is injected as raw HTML (buildTzSelectHtml()) and read
// back this way instead of through the normal WiFiManagerParameter::getValue().
// NOTE: if `wm.server` isn't accessible on your installed WiFiManager version,
// check Examples -> WiFiManager -> Advanced in the Arduino IDE for that
// version's equivalent of reading a custom raw-HTML field on save.
void saveParamsCallback() {
  // Fires whenever the buyer submits the portal's form, so it's also our
  // "a connection attempt is now starting" signal — see
  // handleProvisioningFailure(), which watches for that attempt timing out.
  saveAttemptAt   = millis();
  failedAttemptAt = 0;

  if (!wm.server) return;
  String tz = wm.server->arg("tz");
  if (tz.length() > 0) {
    tz.toCharArray(currentTzInfo, sizeof(currentTzInfo));
    prefs.putString("tz", tz);
    Serial.printf("Timezone saved -> %s\n", currentTzInfo);
  }
}

// Called from loop() while NOT connected. Recovers from a config-portal
// submission that failed to connect, so the buyer can pick a DIFFERENT
// network rather than being stuck editing only the password.
//
// Why this is needed: WiFiManager's NON-blocking portal deliberately leaves
// STA retrying the bad AP after a failed connect (processConfigPortal()'s
// "Portal is non blocking - remaining open" branch does nothing, whereas the
// blocking branch calls WiFi_Disconnect()). A busy STA makes
// WiFi.scanNetworks() return WIFI_SCAN_FAILED, so _numNetworks stays 0 and
// getScanItemOut() emits an EMPTY network list — which is exactly the
// "can't re-select the SSID" symptom.
//
// IMPORTANT: this only ever runs off the back of an explicit Save, so a
// normally-operating clock whose router simply went down never reaches it —
// that case just keeps retrying in the background, as it should. Credentials
// are never erased here either; only the in-progress attempt is dropped.
void handleProvisioningFailure() {
  if (saveAttemptAt != 0 && millis() - saveAttemptAt > SAVE_CONNECT_TIMEOUT_MS) {
    saveAttemptAt   = 0;
    failedAttemptAt = millis();
    Serial.println("Portal: connect attempt failed -> releasing STA so the network list scans again");
    flashConnectFailed();
    WiFi.disconnect();   // same call WiFiManager's blocking branch makes; does
                         // NOT erase saved credentials (non-persistent on ESP32)
  }

  if (failedAttemptAt != 0 && millis() - failedAttemptAt > PORTAL_IDLE_RESTART_MS) {
    Serial.println("Portal idle after a failed attempt -> restarting to resume normal retries");
    ESP.restart();
  }
}

// Has this unit ever completed setup (i.e. actually connected once)?
//
// Deliberately our OWN persisted flag rather than asking the WiFi driver
// (WiFi.SSID()) — that was the first implementation and it was WRONG: this
// early in boot the driver hasn't loaded its saved config from NVS yet, so
// it reported "no credentials" on a fully-configured unit and the factory
// self-test replayed on every single boot. (Same trap as WiFi.macAddress()
// in buildApName().) Verified on hardware: the serial log showed the test
// running immediately before "Connecting to SAVED AP".
//
// Set in onNewlyConnected(), cleared by the button reset gesture, so it
// means exactly "this unit has successfully connected at least once".
bool isConfigured() {
  return prefs.getBool("cfgd", false);
}

// Factory QC self-test: solid RED -> GREEN -> BLUE across the whole panel,
// 1 second each at half brightness, so a bad/dead/miswired WS2812 pixel is
// easy to spot by eye before a unit ships. Only runs on a unit that has
// never been configured (see isConfigured()), then falls through into the
// normal setupIndicatorFrame() confetti + captive portal flow — it's a
// prefix to that, not a replacement for it.
void runFactoryLedTest() {
  Serial.println("Factory LED test: unconfigured unit, running RGB self-test...");
  const CRGB testColors[] = { CRGB::Red, CRGB::Green, CRGB::Blue };
  const char* names[] = { "RED", "GREEN", "BLUE" };

  FastLED.setBrightness(128);   // ~50%, independent of the normal BRIGHTNESS setting
  for (uint8_t i = 0; i < 3; i++) {
    fillAll(testColors[i]);
    FastLED.show();
    Serial.printf("Factory LED test: %s\n", names[i]);
    delay(1000);
  }
  FastLED.setBrightness(BRIGHTNESS);   // restore normal operating brightness
  FastLED.clear(true);
}

void setupWiFiManager() {
  wm.setTitle("Lonely Binary Word Clock");
  // Use WiFiManager's own dark-mode theme (setDarkMode) rather than a custom
  // <style> override — a hand-rolled body{color:...} doesn't reach the
  // library's own scanned-WiFi-network list rows (they carry their own
  // styling), which made every found network invisible: black text on a
  // black background. setDarkMode's "invert" body class is handled correctly
  // by the library's bundled CSS for every element, including that list.
  wm.setDarkMode(true);
  wm.setCustomHeadElement(
    "<p style='opacity:0.7;'>Lonely Binary &mdash; Word Clock setup</p>"
  );

  tzSelectHtml = buildTzSelectHtml();
  static WiFiManagerParameter tzParam(tzSelectHtml.c_str());
  wm.addParameter(&tzParam);
  wm.setSaveParamsCallback(saveParamsCallback);

  wm.setConfigPortalBlocking(false);   // don't hang loop() while the portal is up
  wm.setConfigPortalTimeout(0);        // stay in setup mode until configured
  wm.setConnectTimeout(15);            // give a previously-saved network 15s before falling back

  // Without this, WiFiManager doesn't disconnect the STA driver before a new
  // connection attempt. On hardware this caused two bugs after a wrong
  // password: (1) the "pick a network" scan list stopped rendering (only
  // the SSID/password text fields still worked), and (2) even a correct
  // password on the next attempt would fail to connect — both because the
  // WiFi driver was left in a stuck state from the failed attempt.
  // setCleanConnect(true) makes it call WiFi.disconnect() before every
  // connect attempt, resetting that state each time.
  wm.setCleanConnect(true);

  wm.autoConnect(apName.c_str());      // open AP (no password), e.g. "WordClock-A3F2"
}

// Runs once, the moment WiFi transitions from disconnected -> connected.
void onNewlyConnected() {
  Serial.printf("WiFi connected, IP=%s\n", WiFi.localIP().toString().c_str());

  saveAttemptAt   = 0;   // whatever was pending clearly succeeded
  failedAttemptAt = 0;

  if (!isConfigured()) {
    prefs.putBool("cfgd", true);   // setup complete: no more factory self-test on boot
    Serial.println("Unit marked configured (factory self-test won't run again)");
  }

  // Use the selected timezone's own NTP servers if it has an override
  // (currently just China); otherwise fall back to the global default.
  const char* ntp1 = NTP1;
  const char* ntp2 = NTP2;
  const char* ntp3 = NTP3;
  int tzIdx = findTzOptionIndex(currentTzInfo);
  if (tzIdx >= 0 && TZ_OPTIONS[tzIdx].ntp1 != nullptr) {
    ntp1 = TZ_OPTIONS[tzIdx].ntp1;
    ntp2 = TZ_OPTIONS[tzIdx].ntp2;
    ntp3 = TZ_OPTIONS[tzIdx].ntp3;
    Serial.printf("Using region NTP servers for %s\n", TZ_OPTIONS[tzIdx].label);
  }

  configTzTime(currentTzInfo, ntp1, ntp2, ntp3);   // starts NTP sync in the background;
                                                    // loop()'s existing getLocalTime() check
                                                    // picks up the result once it lands

  // Disable WiFi modem sleep. By default the ESP32 STA dozes between DTIM
  // beacons, which adds tens-to-hundreds of ms of latency to inbound packets
  // (measured on this hardware: LAN ping averaging 45-158ms, spiking to
  // 311ms). ArduinoOTA waits only 1000ms for the FIRST chunk and — because
  // its retry path is guarded by `written &&`, which is 0 at that point —
  // gets no retries, so that latency made every OTA fail with
  // OTA_RECEIVE_ERROR. A mains-powered clock has no reason to modem-sleep.
  WiFi.setSleep(false);

  ArduinoOTA.setHostname(apName.c_str());
  ArduinoOTA.setTimeout(5000);   // more headroom than the 1000ms default

  // Report OTA progress/errors over Serial. Worth keeping permanently: the
  // ESP32 core's own internal log_e() calls are compiled out at the default
  // Arduino debug level, so without these callbacks a failed wireless update
  // is completely silent and impossible to diagnose in the field.
  ArduinoOTA.onStart([]() { Serial.println("OTA: update starting"); });
  ArduinoOTA.onEnd([]()   { Serial.println("OTA: complete, rebooting"); });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    static int lastPct = -1;
    int pct = total ? (int)((done * 100) / total) : 0;
    if (pct >= lastPct + 10) { Serial.printf("OTA: %d%%\n", pct); lastPct = pct; }
  });
  ArduinoOTA.onError([](ota_error_t e) {
    // 0=AUTH 1=BEGIN 2=CONNECT 3=RECEIVE 4=END
    Serial.printf("OTA: ERROR code %u\n", (unsigned)e);
  });

  ArduinoOTA.begin();
  Serial.printf("OTA ready: hostname=%s, no password\n", apName.c_str());

  FastLED.clear(true);   // wipe the setup/confetti visuals; loop() now shows the
                         // red NTP-sync snake until the clock is actually set
}

// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  // Why the previous boot ended. Worth keeping: a unit that reboots in the
  // field (brownout, watchdog, panic) is otherwise indistinguishable from a
  // normal power cycle. 1=POWERON 2=EXT 3=SW 4=PANIC 5=INT_WDT 6=TASK_WDT
  // 7=WDT 8=DEEPSLEEP 9=BROWNOUT 10=SDIO 11=USB 12=JTAG
  // 9 (BROWNOUT) is the one to watch for in the field: it means the supply
  // sagged — e.g. running the whole panel off the dev board's USB port.
  Serial.printf("Boot: reset reason %d\n", (int)esp_reset_reason());

  pinMode(BTN_COLOR,INPUT);   // active-high buttons
  pinMode(BTN_EFFECT,INPUT);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  random16_set_seed(esp_random());       // seed FastLED's PRNG for the effects

  prefs.begin("wordclock", false);
  buildApName();
  loadTimezone();

  if (!isConfigured()) {
    runFactoryLedTest();                 // QC check: only on a never-configured unit
  }

  // Blue snake crawls while autoConnect() blocks trying the saved network,
  // so a unit that takes a few seconds to join doesn't look dead.
  startConnectingAnimation();
  setupWiFiManager();                    // kicks off wm.autoConnect(); non-blocking
                                          // from here if setup mode is needed
  stopConnectingAnimation();             // loop() owns the LEDs from here on
}

void loop() {
  wm.process();                          // service the captive portal, if active

  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) {
      onNewlyConnected();
      wasConnected = true;
    }

    pollButtons();                       // responsive: polled every ~10 ms

    struct tm t;
    if (getLocalTime(&t, 0)) {
      if (!timeSynced) {                 // first successful read: NTP has landed
        timeSynced = true;
        Serial.println("NTP synced, showing time");
      }
      if (t.tm_min != lastMinute) {      // redraw when the minute changes
        lastMinute = t.tm_min;
        showTime(t.tm_hour, t.tm_min);
        Serial.printf("Display %02d:%02d\n", t.tm_hour, t.tm_min);
      }
    } else if (!timeSynced) {
      // Connected, but the clock isn't set yet — getLocalTime() stays false
      // until SNTP lands. Red snake so the panel isn't blank in the gap
      // between "WiFi connected" and "time on the wall".
      ntpSyncFrame();
    }

    ArduinoOTA.handle();
  } else {
    // Not connected (still in setup mode, or a saved network dropped out —
    // the ESP32 WiFi driver auto-retries saved credentials in the background,
    // so this simply reflects "not connected right now").
    wasConnected = false;
    handleProvisioningFailure();         // no-op unless a portal Save just failed
    setupIndicatorFrame();
  }

  checkResetGesture();                   // works in every state, not just at boot

  delay(10);
}
