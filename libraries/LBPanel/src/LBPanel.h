/*
 * LBPanel — shared base for the Lonely Binary 11x10 word panels
 * -------------------------------------------------------------
 * Same hardware as the Word Clock (ESP32-S3, 110 WS2812B letter LEDs on
 * GPIO 8 plus 4 corner dots, buttons on GPIO 42 / 41). Each panel sketch
 * only supplies its letter grid and where its words come from; everything
 * else lives here and behaves exactly like the Word Clock:
 *
 *   - never-configured unit: factory LED self-test, then confetti and an
 *     open "<Prefix>-XXXX" setup hotspot with a captive portal
 *   - BLUE snake while joining WiFi
 *   - both buttons held 3 s: forget everything, back to setup
 *   - OTA from the Arduino IDE (hostname = the hotspot name, no password)
 *
 * and, new for these panels:
 *
 *   - a control page on the local network: http://<prefix>-xxxx.local
 *     (the panel adds its own routes; /settings is built in)
 *   - MQTT over TLS to a public broker, for panels that talk to each other
 *   - HTTPS JSON fetches, for panels that read public APIs
 *
 * Buttons: GPIO 42 = "A", GPIO 41 = "B". By default A cycles the colour
 * (Auto -> White -> Red -> Blue -> Green -> Rainbow, remembered across
 * reboots) and B plays a random effect. A panel can take either button over
 * with lb::onButtons().
 */
#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define LB_COLS          11
#define LB_ROWS          10
#define LB_NUM_WORD_LEDS (LB_COLS * LB_ROWS)            // 110
#define LB_NUM_DOTS      4
#define LB_NUM_LEDS      (LB_NUM_WORD_LEDS + LB_NUM_DOTS)  // 114

// One word on the grid: row 0..9, first column 0..10, length in letters.
struct LBWord { uint8_t row, col, len; };

// A text setting shown in the setup portal and on /settings, saved to flash.
// key: max 15 characters (Preferences limit).
struct LBParam {
  const char* key;
  const char* label;
  const char* def;
  uint8_t     maxLen;
};

struct LBPanelConfig {
  const char*    product;     // "Weather Station" — portal title, page headings
  const char*    hostPrefix;  // "Weather" -> hotspot / hostname "Weather-A3F2"
  const LBParam* params;      // may be nullptr
  uint8_t        numParams;
};

namespace lb {

extern CRGB leds[LB_NUM_LEDS];

// Call once from setup(). Blocks while it tries saved WiFi (blue snake).
void begin(const LBPanelConfig& cfg);

// Call every loop(). Returns true while WiFi is connected — the panel owns
// the LEDs then. While it returns false the library is drawing its own
// "needs setup / reconnecting" animation, so the panel should draw nothing.
bool loop();

// Set when the panel should redraw: first connect, colour change, after an
// effect. takeRedraw() returns it once and clears it.
void requestRedraw();
bool takeRedraw();

const String& name();   // "Weather-A3F2"
const String& id();     // "A3F2"
String hostname();      // "weather-a3f2" (for http://weather-a3f2.local)

String param(const char* key);
void   setParam(const char* key, const String& value);

// Colour for a word under the current colour mode. In Auto mode the panel's
// own meaning colour is used (e.g. red for FAIL); in Rainbow mode every call
// returns a new random colour.
CRGB color(CRGB autoColor = CRGB::White);
uint8_t colorMode();
const char* colorModeName();

void clear();                         // buffer only
void show();
void light(const LBWord& w, CRGB c);
void dot(uint8_t i, CRGB c);          // corner dot 0..3

// Effects (blocking, a few seconds) — they request a redraw when done.
void playRandomEffect();
void sparkle(uint16_t ms);            // short attention flash for new messages

// Return true from a handler to say "handled"; false falls back to the
// default action (A = next colour, B = random effect). Either may be nullptr.
typedef bool (*ButtonHandler)();
void onButtons(ButtonHandler a, ButtonHandler b);
void nextColor();

// Local control page. Register routes in setup(), after begin(); the server
// starts by itself once WiFi is up. /settings is provided by the library.
WebServer& web();
String pageStart(const char* title);
String pageEnd();
String esc(const String& s);          // HTML-escape
String urlEncode(const String& s);

// HTTPS GET + JSON parse. Blocks for up to ~8 s.
bool getJson(const String& url, JsonDocument& doc);

// MQTT over TLS (broker.emqx.io:8883). Subscriptions are re-applied on
// every reconnect. Messages for subscribed topics arrive at the handler.
typedef void (*MqttHandler)(const String& topic, const String& payload);
void mqttBegin(MqttHandler handler);
void mqttSubscribe(const String& filter);
bool mqttPublish(const String& topic, const String& payload);
bool mqttConnected();
extern const char* MQTT_BROKER;

// Random lowercase code (no look-alike characters) for pair / group keys.
String randomCode(uint8_t len);

}  // namespace lb
