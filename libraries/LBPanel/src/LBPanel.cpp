// LBPanel — see LBPanel.h. Most of this is lifted from WordClock.ino, which
// carries the long explanations of why each WiFi / OTA workaround exists;
// the short notes here point back to it.
#include "LBPanel.h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <PubSubClient.h>

#define LB_LED_PIN     8
#define LB_BTN_A       42
#define LB_BTN_B       41
#define LB_BRIGHTNESS  80

#define LB_RESET_HOLD_MS           3000
#define LB_SAVE_CONNECT_TIMEOUT_MS 25000
#define LB_PORTAL_IDLE_RESTART_MS  300000
#define LB_DEBOUNCE_MS             40

#define LB_SNAKE_LEN      8
#define LB_SNAKE_FRAME_MS 45
#define LB_SNAKE_BRIGHT   128

#define LB_QC_CHASE_MS 25
#define LB_QC_SOLID_MS 800

#define LB_MAX_PARAMS 8
#define LB_MAX_SUBS   6

namespace lb {

const char* MQTT_BROKER = "broker.emqx.io";
static const uint16_t MQTT_PORT = 8883;

CRGB leds[LB_NUM_LEDS];

static const LBPanelConfig* cfg = nullptr;
static WiFiManager wm;
static Preferences prefs;
static WebServer   server(80);
static String      devName, devId;

static WiFiManagerParameter* portalParams[LB_MAX_PARAMS];
static uint8_t numPortalParams = 0;

static bool     redraw        = false;
static bool     wasConnected  = false;
static bool     servicesUp    = false;
static uint32_t saveAttemptAt   = 0;
static uint32_t failedAttemptAt = 0;

static ButtonHandler handlerA = nullptr, handlerB = nullptr;

// ---------------------------------------------------------------------------
// Colour
// ---------------------------------------------------------------------------
enum { COL_AUTO = 0, COL_WHITE, COL_RED, COL_BLUE, COL_GREEN, COL_RAINBOW, NUM_COLOR_MODES };
static uint8_t colMode = COL_AUTO;
static const char* COLOR_NAMES[NUM_COLOR_MODES] = { "Auto", "White", "Red", "Blue", "Green", "Rainbow" };

CRGB color(CRGB autoColor) {
  switch (colMode) {
    case COL_AUTO:    return autoColor;
    case COL_WHITE:   return CRGB(255, 255, 255);
    case COL_RED:     return CRGB::Red;
    case COL_BLUE:    return CRGB::Blue;
    case COL_GREEN:   return CRGB::Green;
    case COL_RAINBOW: return (CRGB)CHSV(random8(), 255, 255);
  }
  return autoColor;
}
uint8_t colorMode() { return colMode; }
const char* colorModeName() { return COLOR_NAMES[colMode]; }

void nextColor() {
  colMode = (colMode + 1) % NUM_COLOR_MODES;
  prefs.putUChar("col", colMode);
  Serial.printf("Colour -> %s\n", COLOR_NAMES[colMode]);
  redraw = true;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------
static uint16_t XY(uint8_t row, uint8_t col) { return (uint16_t)row * LB_COLS + col; }

void clear() { FastLED.clear(); }
void show()  { FastLED.show(); }

void light(const LBWord& w, CRGB c) {
  for (uint8_t i = 0; i < w.len && w.col + i < LB_COLS; i++) leds[XY(w.row, w.col + i)] = c;
}

void dot(uint8_t i, CRGB c) {
  if (i < LB_NUM_DOTS) leds[LB_NUM_WORD_LEDS + i] = c;
}

static void fillAll(CRGB c) { for (int i = 0; i < LB_NUM_LEDS; i++) leds[i] = c; }

void requestRedraw() { redraw = true; }
bool takeRedraw() { bool r = redraw; redraw = false; return r; }

// ---------------------------------------------------------------------------
// Effects (same set as the Word Clock's EFFECT button)
// ---------------------------------------------------------------------------
static void fxConfetti(uint16_t ms) {
  uint32_t end = millis() + ms;
  while (millis() < end) {
    fadeToBlackBy(leds, LB_NUM_LEDS, 18);
    leds[random16(LB_NUM_LEDS)] += CHSV(random8(), 200, 255);
    FastLED.show();
    delay(18);
  }
}

static void fxRainbowComet(uint16_t ms) {
  const uint8_t cometLen = 10;
  uint32_t end = millis() + ms;
  uint8_t hueBase = 0;
  uint16_t pos = 0;
  while (millis() < end) {
    fadeToBlackBy(leds, LB_NUM_LEDS, 40);
    for (uint8_t i = 0; i < cometLen; i++) leds[(pos + i) % LB_NUM_LEDS] = CHSV(hueBase + i * 8, 255, 255);
    FastLED.show();
    delay(25);
    pos = (pos + 1) % LB_NUM_LEDS;
    hueBase += 4;
  }
}

static void fxColorWipe() {
  CRGB c = (CRGB)CHSV(random8(), 255, 255);
  FastLED.clear();
  for (int i = 0; i < LB_NUM_LEDS; i++) { leds[i] = c; FastLED.show(); delay(10); }
  delay(300);
}

static void fxTheaterChase(uint16_t ms) {
  uint32_t end = millis() + ms;
  uint8_t hue = 0, q = 0;
  while (millis() < end) {
    FastLED.clear();
    for (int i = q; i < LB_NUM_LEDS; i += 3) leds[i] = CHSV(hue, 255, 255);
    FastLED.show();
    delay(55);
    q = (q + 1) % 3;
    hue += 8;
  }
}

void sparkle(uint16_t ms) {
  uint32_t end = millis() + ms;
  while (millis() < end) {
    fadeToBlackBy(leds, LB_NUM_LEDS, 40);
    leds[random16(LB_NUM_LEDS)] = CRGB::White;
    FastLED.show();
    delay(28);
  }
  FastLED.clear(true);
  redraw = true;
}

void playRandomEffect() {
  switch (random8(5)) {
    case 0: fxConfetti(3000);     break;
    case 1: fxRainbowComet(3000); break;
    case 2: fxColorWipe();        break;
    case 3: fxTheaterChase(3000); break;
    case 4: sparkle(3000);        break;
  }
  FastLED.clear(true);
  redraw = true;
}

// Non-blocking confetti frame: "needs setup" / "WiFi lost".
static void setupIndicatorFrame() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  if (now - lastFrame < 18) return;
  lastFrame = now;
  fadeToBlackBy(leds, LB_NUM_LEDS, 18);
  leds[random16(LB_NUM_LEDS)] += CHSV(random8(), 200, 255);
  FastLED.show();
}

// ---------------------------------------------------------------------------
// Blue "joining WiFi" snake, on its own task because autoConnect() blocks.
// ---------------------------------------------------------------------------
static TaskHandle_t connectAnimHandle = NULL;

static void drawSnakeFrame(const CRGB& base) {
  static uint16_t head = 0;
  FastLED.clear();
  for (uint8_t i = 0; i < LB_SNAKE_LEN; i++) {
    uint16_t p = (head + LB_NUM_WORD_LEDS - i) % LB_NUM_WORD_LEDS;
    uint8_t row = p / LB_COLS, col = p % LB_COLS;
    if (row & 0x01) col = LB_COLS - 1 - col;
    CRGB c = base;
    c.nscale8_video(LB_SNAKE_BRIGHT - (i * (LB_SNAKE_BRIGHT / LB_SNAKE_LEN)));
    leds[XY(row, col)] = c;
  }
  FastLED.show();
  head = (head + 1) % LB_NUM_WORD_LEDS;
}

static void connectAnimTask(void*) {
  for (;;) { drawSnakeFrame(CRGB::Blue); vTaskDelay(pdMS_TO_TICKS(LB_SNAKE_FRAME_MS)); }
}

static void startConnectingAnimation() {
  if (!connectAnimHandle) xTaskCreate(connectAnimTask, "connAnim", 4096, NULL, 1, &connectAnimHandle);
}

static void stopConnectingAnimation() {
  if (!connectAnimHandle) return;
  vTaskDelete(connectAnimHandle);
  connectAnimHandle = NULL;
  delay(LB_SNAKE_FRAME_MS * 2);
  FastLED.clear(true);
}

// ---------------------------------------------------------------------------
// Factory QC self-test (identical to the Word Clock's): per colour, a single
// pixel chases the whole strip, then the whole panel goes solid.
// ---------------------------------------------------------------------------
static void runFactoryLedTest() {
  Serial.println("Factory LED test: unconfigured unit, running RGB self-test...");
  const CRGB testColors[] = { CRGB::Red, CRGB::Green, CRGB::Blue };
  FastLED.setBrightness(128);
  for (uint8_t c = 0; c < 3; c++) {
    for (int i = 0; i < LB_NUM_LEDS; i++) {
      FastLED.clear();
      leds[i] = testColors[c];
      FastLED.show();
      delay(LB_QC_CHASE_MS);
    }
    FastLED.clear(true);
    delay(LB_QC_CHASE_MS);
    fillAll(testColors[c]);
    FastLED.show();
    delay(LB_QC_SOLID_MS);
    FastLED.clear(true);
    delay(LB_QC_CHASE_MS);
  }
  FastLED.setBrightness(LB_BRIGHTNESS);
  FastLED.clear(true);
}

// ---------------------------------------------------------------------------
// Buttons + reset gesture
// ---------------------------------------------------------------------------
static bool btnAPrev = false, btnBPrev = false;
static uint32_t btnAT = 0, btnBT = 0;
static uint32_t comboHeldSince = 0;
static bool resetTriggered = false;

static bool pressedEdge(uint8_t pin, bool& prev, uint32_t& tLast) {
  bool now = (digitalRead(pin) == HIGH);
  bool fired = false;
  if (now != prev && (millis() - tLast) >= LB_DEBOUNCE_MS) {
    tLast = millis();
    if (now) fired = true;
    prev = now;
  }
  return fired;
}

static bool comboActive() { return digitalRead(LB_BTN_A) == HIGH && digitalRead(LB_BTN_B) == HIGH; }

void onButtons(ButtonHandler a, ButtonHandler b) { handlerA = a; handlerB = b; }

static void pollButtons() {
  bool combo = comboActive();
  bool aPressed = pressedEdge(LB_BTN_A, btnAPrev, btnAT);
  bool bPressed = pressedEdge(LB_BTN_B, btnBPrev, btnBT);
  if (aPressed && !combo) {
    if (!(handlerA && handlerA())) nextColor();
  }
  if (bPressed && !combo) {
    if (!(handlerB && handlerB())) playRandomEffect();
  }
}

static void flashAll(CRGB c) {
  for (uint8_t i = 0; i < 3; i++) {
    fillAll(c);           FastLED.show(); delay(150);
    fillAll(CRGB::Black); FastLED.show(); delay(150);
  }
}

static void checkResetGesture() {
  if (comboActive()) {
    if (comboHeldSince == 0) {
      comboHeldSince = millis();
    } else if (!resetTriggered && millis() - comboHeldSince >= LB_RESET_HOLD_MS) {
      resetTriggered = true;
      Serial.println("Reset gesture held 3s -> forgetting WiFi and settings, restarting...");
      flashAll(CRGB::White);
      prefs.clear();          // settings, colour, "configured" flag — all of it
      wm.resetSettings();
      delay(300);
      ESP.restart();
    }
  } else {
    comboHeldSince = 0;
    resetTriggered = false;
  }
}

// ---------------------------------------------------------------------------
// Identity + settings
// ---------------------------------------------------------------------------
const String& name() { return devName; }
const String& id()   { return devId; }
String hostname()    { String h = devName; h.toLowerCase(); return h; }

static const LBParam* findParam(const char* key) {
  if (!cfg) return nullptr;
  for (uint8_t i = 0; i < cfg->numParams; i++) if (strcmp(cfg->params[i].key, key) == 0) return &cfg->params[i];
  return nullptr;
}

String param(const char* key) {
  const LBParam* p = findParam(key);
  return prefs.getString(key, p ? p->def : "");
}

void setParam(const char* key, const String& value) {
  String v = value;
  v.trim();
  prefs.putString(key, v);
}

String randomCode(uint8_t len) {
  static const char ALPHA[] = "abcdefghjkmnpqrstuvwxyz23456789";
  String s;
  for (uint8_t i = 0; i < len; i++) s += ALPHA[esp_random() % (sizeof(ALPHA) - 1)];
  return s;
}

// ---------------------------------------------------------------------------
// HTML helpers
// ---------------------------------------------------------------------------
String esc(const String& s) {
  String o;
  o.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '<') o += "&lt;"; else if (c == '>') o += "&gt;";
    else if (c == '&') o += "&amp;"; else if (c == '\'') o += "&#39;";
    else if (c == '"') o += "&quot;"; else o += c;
  }
  return o;
}

String urlEncode(const String& s) {
  String o;
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') o += (char)c;
    else { o += '%'; o += hex[c >> 4]; o += hex[c & 15]; }
  }
  return o;
}

String pageStart(const char* title) {
  String h;
  h.reserve(2048);
  h += F("<!doctype html><html><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'><title>");
  h += title;
  h += F("</title><style>"
         "body{margin:0;padding:0 1rem 3rem;background:#0e0f13;color:#e9eaee;"
         "font:16px/1.5 -apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif}"
         ".wrap{max-width:560px;margin:0 auto}"
         ".brand{padding-top:1.6rem;font-size:.75rem;letter-spacing:.16em;text-transform:uppercase;color:#9aa0ac}"
         "h1{font-size:1.5rem;margin:.3rem 0 1rem}h2{font-size:1rem;margin:1.6rem 0 .5rem;color:#9aa0ac}"
         ".grid{display:grid;grid-template-columns:1fr 1fr;gap:.5rem}"
         "button,.b{display:block;width:100%;background:#1c1f27;color:#e9eaee;border:1px solid #2a2e38;"
         "border-radius:8px;padding:.75rem .5rem;font-size:.95rem;font-weight:600;cursor:pointer;text-align:center;text-decoration:none}"
         "button.on{background:#117A80;border-color:#14898F}button.p{background:#117A80;border-color:#14898F}"
         "input{width:100%;box-sizing:border-box;background:#15171d;color:#e9eaee;border:1px solid #2a2e38;"
         "border-radius:6px;padding:.6rem;font-size:1rem;margin:.2rem 0 .8rem}"
         "label{font-size:.85rem;color:#9aa0ac}code,pre{font-family:ui-monospace,Menlo,monospace;font-size:.8rem}"
         "pre{background:#15171d;border:1px solid #23262e;border-radius:6px;padding:.6rem;overflow-x:auto;white-space:pre-wrap;word-break:break-all}"
         ".m{color:#9aa0ac;font-size:.85rem}a{color:#4da3ff}"
         "</style><script>function go(u){fetch(u,{method:'POST'}).then(()=>location.reload())}</script>"
         "</head><body><div class='wrap'><div class='brand'>Lonely Binary &middot; ");
  h += cfg ? cfg->product : "";
  h += F("</div><h1>");
  h += title;
  h += F("</h1>");
  return h;
}

String pageEnd() {
  String h = F("<p class='m' style='margin-top:2rem'>");
  h += esc(devName);
  h += F(" &middot; colour: ");
  h += COLOR_NAMES[colMode];
  h += F(" &middot; <a href='/'>Home</a> &middot; <a href='/settings'>Settings</a></p></div></body></html>");
  return h;
}

WebServer& web() { return server; }

static void handleSettings() {
  if (server.method() == HTTP_POST) {
    for (uint8_t i = 0; i < cfg->numParams; i++) {
      if (server.hasArg(cfg->params[i].key)) setParam(cfg->params[i].key, server.arg(cfg->params[i].key));
    }
    String h = pageStart("Saved");
    h += F("<p>Settings saved. The panel is restarting &mdash; this page will reload in a few seconds.</p>"
           "<script>setTimeout(()=>location.href='/',8000)</script>");
    h += pageEnd();
    server.send(200, "text/html", h);
    delay(500);
    ESP.restart();
    return;
  }
  String h = pageStart("Settings");
  h += F("<form method='post' action='/settings'>");
  for (uint8_t i = 0; i < cfg->numParams; i++) {
    const LBParam& p = cfg->params[i];
    h += F("<label>"); h += esc(p.label); h += F("</label><input name='"); h += p.key;
    h += F("' maxlength='"); h += p.maxLen; h += F("' value='"); h += esc(param(p.key)); h += F("'>");
  }
  if (cfg->numParams == 0) h += F("<p class='m'>This panel has no settings.</p>");
  else h += F("<button class='p' type='submit'>Save and restart</button>");
  h += F("</form><h2>Colour</h2><button onclick=\"go('/api/color')\">Next colour</button>"
         "<h2>WiFi</h2><p class='m'>To join a different network, hold both buttons on the panel for 3 seconds. "
         "It forgets everything and opens its setup hotspot again.</p>");
  h += pageEnd();
  server.send(200, "text/html", h);
}

// ---------------------------------------------------------------------------
// HTTPS JSON
// ---------------------------------------------------------------------------
bool getJson(const String& url, JsonDocument& doc) {
  WiFiClientSecure net;
  net.setInsecure();   // encrypted, not authenticated — no CA bundle to keep current
  HTTPClient http;
  http.setTimeout(8000);
  http.setConnectTimeout(8000);
  http.setUserAgent("LonelyBinary-Panel/1.0");
  if (!http.begin(net, url)) { Serial.printf("HTTP: bad URL %s\n", url.c_str()); return false; }
  http.addHeader("Accept", "application/json");
  int code = http.GET();
  if (code != 200) {
    Serial.printf("HTTP: %d from %s\n", code, url.c_str());
    http.end();
    return false;
  }
  String body = http.getString();
  http.end();
  DeserializationError err = deserializeJson(doc, body);
  if (err) { Serial.printf("HTTP: JSON error %s\n", err.c_str()); return false; }
  return true;
}

// ---------------------------------------------------------------------------
// MQTT
// ---------------------------------------------------------------------------
static WiFiClientSecure mqttNet;
static PubSubClient mqtt(mqttNet);
static MqttHandler mqttHandler = nullptr;
static String mqttSubs[LB_MAX_SUBS];
static uint8_t numMqttSubs = 0;
static uint32_t mqttLastTry = 0;

static void mqttCallback(char* topic, byte* payload, unsigned int len) {
  String t(topic), p;
  p.reserve(len);
  for (unsigned int i = 0; i < len; i++) p += (char)payload[i];
  Serial.printf("MQTT <- %s %s\n", t.c_str(), p.c_str());
  if (mqttHandler) mqttHandler(t, p);
}

void mqttBegin(MqttHandler handler) {
  mqttHandler = handler;
  mqttNet.setInsecure();
  mqttNet.setHandshakeTimeout(8);
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(8);
  mqtt.setCallback(mqttCallback);
}

void mqttSubscribe(const String& filter) {
  if (numMqttSubs < LB_MAX_SUBS) mqttSubs[numMqttSubs++] = filter;
  if (mqtt.connected()) mqtt.subscribe(filter.c_str());
}

bool mqttPublish(const String& topic, const String& payload) {
  if (!mqtt.connected()) { Serial.println("MQTT: not connected, dropped publish"); return false; }
  Serial.printf("MQTT -> %s %s\n", topic.c_str(), payload.c_str());
  return mqtt.publish(topic.c_str(), payload.c_str());
}

bool mqttConnected() { return mqtt.connected(); }

static void mqttService() {
  if (!mqttHandler) return;
  if (mqtt.connected()) { mqtt.loop(); return; }
  // Each attempt can block for a few seconds (TLS handshake), during which
  // buttons and the control page don't respond — so back off 10 s, 20 s,
  // 40 s ... up to 5 minutes while the broker stays unreachable.
  static uint32_t backoff = 5000;   // doubled before first use -> 10 s
  if (mqttLastTry != 0 && millis() - mqttLastTry < backoff) return;
  mqttLastTry = millis();
  String clientId = devName + "-" + randomCode(4);
  Serial.printf("MQTT: connecting to %s as %s...\n", MQTT_BROKER, clientId.c_str());
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("MQTT: connected");
    for (uint8_t i = 0; i < numMqttSubs; i++) mqtt.subscribe(mqttSubs[i].c_str());
    backoff = 5000;
    redraw = true;
  } else {
    backoff = min<uint32_t>(backoff * 2, 300000);
    Serial.printf("MQTT: failed, state %d (retry in %lus)\n", mqtt.state(), (unsigned long)(backoff / 1000));
  }
}

// ---------------------------------------------------------------------------
// WiFi setup (WiFiManager), same flow as the Word Clock
// ---------------------------------------------------------------------------
static bool isConfigured() { return prefs.getBool("cfgd", false); }

static void saveParamsCallback() {
  saveAttemptAt = millis();
  failedAttemptAt = 0;
  for (uint8_t i = 0; i < numPortalParams; i++) {
    setParam(portalParams[i]->getID(), String(portalParams[i]->getValue()));
  }
}

// See WordClock.ino handleProvisioningFailure(): releases the STA after a
// failed portal Save so the network list scans again.
static void handleProvisioningFailure() {
  if (saveAttemptAt != 0 && millis() - saveAttemptAt > LB_SAVE_CONNECT_TIMEOUT_MS) {
    saveAttemptAt = 0;
    failedAttemptAt = millis();
    Serial.println("Portal: connect attempt failed -> releasing STA");
    flashAll(CRGB::Red);
    WiFi.disconnect();
  }
  if (failedAttemptAt != 0 && millis() - failedAttemptAt > LB_PORTAL_IDLE_RESTART_MS) {
    ESP.restart();
  }
}

static void setupWiFiManager() {
  static String title = String("Lonely Binary ") + cfg->product;
  static String head  = String("<p style='opacity:0.7;'>Lonely Binary &mdash; ") + cfg->product + " setup</p>";
  wm.setTitle(title);
  wm.setDarkMode(true);
  wm.setCustomHeadElement(head.c_str());

  numPortalParams = 0;
  for (uint8_t i = 0; i < cfg->numParams && i < LB_MAX_PARAMS; i++) {
    const LBParam& p = cfg->params[i];
    static String values[LB_MAX_PARAMS];
    values[i] = param(p.key);
    portalParams[i] = new WiFiManagerParameter(p.key, p.label, values[i].c_str(), p.maxLen);
    wm.addParameter(portalParams[i]);
    numPortalParams++;
  }
  wm.setSaveParamsCallback(saveParamsCallback);

  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(0);
  wm.setConnectTimeout(15);
  wm.setCleanConnect(true);
  wm.autoConnect(devName.c_str());
}

static void startServices() {
  // A portal-driven connect can leave WiFiManager's own server on port 80.
  if (wm.getConfigPortalActive()) wm.stopConfigPortal();

  WiFi.setSleep(false);   // see WordClock.ino: modem sleep breaks OTA

  ArduinoOTA.setHostname(devName.c_str());
  ArduinoOTA.setTimeout(5000);
  ArduinoOTA.onStart([]() { Serial.println("OTA: update starting"); });
  ArduinoOTA.onEnd([]()   { Serial.println("OTA: complete, rebooting"); });
  ArduinoOTA.onError([](ota_error_t e) { Serial.printf("OTA: ERROR code %u\n", (unsigned)e); });
  ArduinoOTA.begin();     // also starts mDNS under the same hostname
  MDNS.addService("http", "tcp", 80);

  server.on("/settings", HTTP_ANY, handleSettings);
  server.on("/api/color", HTTP_ANY, []() { nextColor(); server.send(200, "text/plain", COLOR_NAMES[colMode]); });
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();

  Serial.printf("Control page: http://%s.local  (or http://%s)\n",
                hostname().c_str(), WiFi.localIP().toString().c_str());
}

static void onNewlyConnected() {
  Serial.printf("WiFi connected, IP=%s\n", WiFi.localIP().toString().c_str());
  bool viaPortal = saveAttemptAt != 0;
  saveAttemptAt = 0;
  failedAttemptAt = 0;
  if (!isConfigured()) prefs.putBool("cfgd", true);
  // Settings typed into the setup portal were saved after the sketch's
  // setup() had already read them. One clean restart picks them up.
  if (viaPortal && cfg->numParams > 0) {
    Serial.println("Setup complete -> restarting once to apply the portal settings");
    delay(500);
    ESP.restart();
  }
  if (!servicesUp) { startServices(); servicesUp = true; }
  FastLED.clear(true);
  redraw = true;
}

// ---------------------------------------------------------------------------
void begin(const LBPanelConfig& config) {
  cfg = &config;
  Serial.begin(115200);
  delay(200);
  Serial.printf("Boot: %s, reset reason %d\n", cfg->product, (int)esp_reset_reason());

  pinMode(LB_BTN_A, INPUT);
  pinMode(LB_BTN_B, INPUT);

  FastLED.addLeds<WS2812B, LB_LED_PIN, GRB>(leds, LB_NUM_LEDS);
  FastLED.setBrightness(LB_BRIGHTNESS);
  FastLED.clear(true);
  random16_set_seed(esp_random());

  prefs.begin("lbpanel", false);
  colMode = prefs.getUChar("col", COL_AUTO) % NUM_COLOR_MODES;

  char buf[32];
  uint16_t suffix = (uint16_t)(ESP.getEfuseMac() & 0xFFFF);   // see WordClock.ino buildApName()
  snprintf(buf, sizeof(buf), "%04X", suffix);
  devId = buf;
  devName = String(cfg->hostPrefix) + "-" + devId;

  if (!isConfigured()) runFactoryLedTest();

  startConnectingAnimation();
  setupWiFiManager();
  stopConnectingAnimation();
}

bool loop() {
  wm.process();
  checkResetGesture();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) { onNewlyConnected(); wasConnected = true; }
    server.handleClient();
    ArduinoOTA.handle();
    mqttService();
    pollButtons();
    delay(5);
    return true;
  }

  if (wasConnected) { wasConnected = false; Serial.println("WiFi lost, retrying in the background"); }
  handleProvisioningFailure();
  setupIndicatorFrame();
  delay(5);
  return false;
}

}  // namespace lb
