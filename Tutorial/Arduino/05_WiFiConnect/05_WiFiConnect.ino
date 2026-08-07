/*
 * Lesson 5 — WiFi Connect
 * -----------------------
 * Connect the ESP32-S3 to your WiFi network, with a small LED sweep
 * animation on the panel while it connects.
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>
#include <WiFi.h>

// ----------------------------------------------------------------------------
// USER CONFIG — edit these
// ----------------------------------------------------------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

#define LED_PIN     8
#define COLS        11
#define NUM_LEDS    114
#define BRIGHTNESS  80

CRGB leds[NUM_LEDS];

uint16_t XY(uint8_t row, uint8_t col) {
  return (uint16_t)row * COLS + col;
}

// A small progress sweep across row 0 while we wait for something.
void statusSweep(CRGB c) {
  static uint8_t p = 0;
  FastLED.clear();
  leds[XY(0, p % COLS)] = c;
  FastLED.show();
  p++;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    statusSweep(CRGB(0, 40, 80));   // blue-ish sweep = joining WiFi
    Serial.print('.');
    delay(250);
  }
  Serial.printf("\nWiFi OK, IP=%s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  connectWiFi();

  // Show solid green once connected.
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Green;
  FastLED.show();
}

void loop() {
  // Nothing to do — stays green once connected.
}
