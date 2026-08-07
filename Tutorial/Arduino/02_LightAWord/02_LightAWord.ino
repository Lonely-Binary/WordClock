/*
 * Lesson 2 — Light a Word
 * -----------------------
 * The panel is really an 11x10 grid of letters, not just a flat strip.
 * This lesson introduces row/column mapping and lights up one whole word:
 * "IT IS" in the top-left corner.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      I  T  L  I  S  A  S  A  M  P  M    IT  IS  (AM PM)
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>

#define LED_PIN     8
#define COLS        11
#define ROWS        10
#define NUM_LEDS    114
#define BRIGHTNESS  80

CRGB leds[NUM_LEDS];

// Convert a (row, col) grid position into an index in leds[].
// The panel is wired row-major: row 0 is LEDs 0..10, row 1 is 11..21, etc.
uint16_t XY(uint8_t row, uint8_t col) {
  return (uint16_t)row * COLS + col;
}

// Light `len` letters starting at (row, col) — every word on this panel
// is horizontal and contiguous, so this one helper draws all of them.
void lightWord(uint8_t row, uint8_t col, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    leds[XY(row, col + i)] = CRGB(255, 255, 255);
  }
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();

  lightWord(0, 0, 2);   // row 0, starting at col 0, 2 letters -> "IT"
  lightWord(0, 3, 2);   // row 0, starting at col 3, 2 letters -> "IS"

  FastLED.show();
}

void loop() {
  // Nothing to do — the words just stay lit.
}
