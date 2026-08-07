/*
 * Lesson 3 — Colors and Grid
 * --------------------------
 * Light up the ENTIRE panel and cycle through colours, including a
 * per-pixel rainbow using FastLED's HSV colour space.
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>

#define LED_PIN     8
#define NUM_LEDS    114
#define BRIGHTNESS  80

CRGB leds[NUM_LEDS];

// Cycle through these colours, a few seconds each, then rainbow.
enum { COL_WHITE = 0, COL_RED, COL_BLUE, COL_GREEN, COL_RAINBOW, NUM_COLOR_MODES };

void fillSolid(CRGB c) {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
  FastLED.show();
}

void fillRainbow() {
  // CHSV(hue, saturation, value) — sweeping hue across the grid makes a
  // smooth rainbow. Every pixel gets a slightly different hue.
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * 256 / NUM_LEDS), 255, 255);
  }
  FastLED.show();
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
}

void loop() {
  static uint8_t mode = COL_WHITE;

  switch (mode) {
    case COL_WHITE:   fillSolid(CRGB::White); break;
    case COL_RED:     fillSolid(CRGB::Red);   break;
    case COL_BLUE:    fillSolid(CRGB::Blue);  break;
    case COL_GREEN:   fillSolid(CRGB::Green); break;
    case COL_RAINBOW: fillRainbow();          break;
  }

  mode = (mode + 1) % NUM_COLOR_MODES;
  delay(2000);   // hold each colour for 2 seconds
}
