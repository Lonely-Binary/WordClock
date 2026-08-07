/*
 * Lesson 8 — OTA Updates (Arduino/C++ only, bonus lesson)
 * ---------------------------------------------------------
 * Update the ESP32-S3 wirelessly: once it's on WiFi, upload new firmware
 * from the Arduino IDE without touching a USB cable.
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// ----------------------------------------------------------------------------
// USER CONFIG — edit these
// ----------------------------------------------------------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

#define LED_PIN     8
#define NUM_LEDS    114
#define BRIGHTNESS  80

// Try changing this colour, then re-upload over WiFi (not USB) — that's the
// "did OTA actually work" check. See the README for how.
CRGB DEMO_COLOR = CRGB::Blue;

CRGB leds[NUM_LEDS];

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

void setupOTA() {
  ArduinoOTA.setHostname("wordclock-tutorial");   // shows up under this name in Tools > Port
  // No password set here — see the README for why, and how to add one.

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update starting...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update complete, rebooting.");
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    Serial.printf("OTA progress: %u%%\n", (done * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error [%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready.");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  connectWiFi();
  setupOTA();

  for (int i = 0; i < NUM_LEDS; i++) leds[i] = DEMO_COLOR;
  FastLED.show();
}

void loop() {
  ArduinoOTA.handle();   // must run often, or an in-progress upload will stall and fail
  delay(10);
}
