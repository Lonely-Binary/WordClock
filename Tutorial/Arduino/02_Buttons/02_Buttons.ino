/*
 * Lesson 4 — Buttons
 * ------------------
 * Read the two onboard buttons and use one of them to cycle the panel's
 * colour on each press.
 *
 * BUTTONS (both ACTIVE-HIGH -> reads HIGH when pressed; internal pull-down on):
 *   GPIO 42 = COLOR button (used in this lesson)
 *   GPIO 41 = EFFECT button (wired up, but unused until Lesson 7 / the full firmware)
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>

#define LED_PIN     8
#define NUM_LEDS    114
#define BRIGHTNESS  80

#define BTN_COLOR   42
#define BTN_EFFECT  41

CRGB leds[NUM_LEDS];

enum { COL_WHITE = 0, COL_RED, COL_BLUE, COL_GREEN, NUM_COLOR_MODES };
uint8_t colorMode = COL_WHITE;

void fillCurrentColor() {
  CRGB c;
  switch (colorMode) {
    case COL_WHITE: c = CRGB::White; break;
    case COL_RED:   c = CRGB::Red;   break;
    case COL_BLUE:  c = CRGB::Blue;  break;
    case COL_GREEN: c = CRGB::Green; break;
  }
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
  FastLED.show();
}

// Debounced press detection: fires once per press, ignoring contact bounce.
#define DEBOUNCE_MS 40
bool     btnColorPrev = false;
uint32_t btnColorT    = 0;

bool pressedEdge(uint8_t pin, bool &prev, uint32_t &tLast) {
  bool now   = (digitalRead(pin) == HIGH);
  bool fired = false;
  if (now != prev && (millis() - tLast) >= DEBOUNCE_MS) {
    tLast = millis();
    if (now) fired = true;   // rising edge = press
    prev = now;
  }
  return fired;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_COLOR, INPUT);
  pinMode(BTN_EFFECT, INPUT);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  fillCurrentColor();
}

void loop() {
  if (pressedEdge(BTN_COLOR, btnColorPrev, btnColorT)) {
    colorMode = (colorMode + 1) % NUM_COLOR_MODES;
    Serial.printf("Color mode -> %d\n", colorMode);
    fillCurrentColor();
  }

  delay(10);   // poll roughly every 10ms — responsive without hogging the CPU
}
