/*
 * Lesson 1 — Single Pixel
 * -----------------------
 * The "hello world" of the word clock panel: light up exactly one LED.
 * If this works, your wiring, power, and FastLED install are all good.
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 * Power: flash via the dev board's Type-C port, then power the panel from
 *        the BASE's Type-C port to actually see the LED — the dev board's
 *        own USB port can't supply enough current for the panel.
 */

#include <FastLED.h>

#define LED_PIN     8      // WS2812 data line
#define NUM_LEDS    114    // 110 letters + 4 corner dots
#define BRIGHTNESS  80      // 0-255, keep it low on a full grid

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.clear();                // turn every LED off first
  leds[0] = CRGB(255, 255, 255);  // LED index 0 = top-left letter, "I"
  FastLED.show();                 // nothing changes on the panel until you call show()
}

void loop() {
  // Nothing to do — the panel just stays lit.
}
