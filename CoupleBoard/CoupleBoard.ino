/*
 * Couple Message Board — ESP32-S3 Blueprints, Lonely Binary
 * ---------------------------------------------------------
 * Two panels, one in each home. Tap "ON THE WAY" on your phone and it
 * lights up on your partner's panel; they answer OK / NO / MAYBE with the
 * buttons on the panel (or their phone), and the answer lights up on yours.
 * Same board, base and wiring as the Word Clock. Needs the LBPanel library.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      R  O  N  Q  T  H  E  Z  W  A  Y    ON THE WAY
 *   r1      H  O  M  E  T  L  S  A  F  E  P    HOME SAFE
 *   r2      D  L  A  T  E  Q  S  O  R  R  Y    LATE SORRY
 *   r3      C  A  L  L  R  M  E  V  N  O  W    CALL ME (NOW)
 *   r4      C  O  M  E  R  G  E  T  V  M  E    COME GET ME
 *   r5      V  D  O  W  N  S  T  A  I  R  S    DOWNSTAIRS
 *   r6      A  L  M  O  S  T  Q  H  E  R  E    ALMOST HERE
 *   r7      D  O  N  E  V  W  O  R  K  N  P    DONE WORK
 *   r8      O  K  N  O  Q  M  A  Y  Z  B  E    OK | NO | MAY BE   (replies)
 *   r9      L  U  N  C  H  D  I  N  N  E  R    LUNCH | DINNER
 *
 * Pairing: both panels need the same pair code. Each panel makes one up on
 * first boot; open your panel's page (http://coupleboard-xxxx.local), copy
 * its code into your partner's Settings, and you're paired. Messages go
 * through the public MQTT broker broker.emqx.io over TLS, on topics under
 * lonelybinary/couple/<pair code>/ — the code is the only secret, so keep it
 * long (it's 10 random characters by default).
 *
 * On the panel:
 *   - a message from your partner shows bright until you answer it:
 *       button A = OK, button B = NO (MAYBE from the phone page)
 *   - a message you sent shows dim, plus their answer once it arrives
 *   - otherwise button A changes the colour, button B clears the panel
 * A conversation clears itself after 6 hours.
 */
#include <LBPanel.h>

const LBWord ON = {0, 1, 2}, THE = {0, 4, 3}, WAY = {0, 8, 3};
const LBWord HOME = {1, 0, 4}, SAFE = {1, 6, 4};
const LBWord LATE = {2, 1, 4}, SORRY = {2, 6, 5};
const LBWord CALL = {3, 0, 4}, ME1 = {3, 5, 2}, NOW = {3, 8, 3};
const LBWord COME = {4, 0, 4}, GET = {4, 5, 3}, ME2 = {4, 9, 2};
const LBWord DOWNSTAIRS = {5, 1, 10};
const LBWord ALMOST = {6, 0, 6}, HERE = {6, 7, 4};
const LBWord DONE = {7, 0, 4}, WORK = {7, 5, 4};
const LBWord RP_OK = {8, 0, 2}, RP_NO = {8, 2, 2}, RP_MAY = {8, 5, 3}, RP_BE = {8, 9, 2};
const LBWord LUNCH = {9, 0, 5}, DINNER = {9, 5, 6};

struct Message { const char* key; const char* label; const LBWord* words[3]; };
const Message MESSAGES[] = {
  { "on_the_way",  "On the way",  { &ON, &THE, &WAY } },
  { "home_safe",   "Home safe",   { &HOME, &SAFE } },
  { "late_sorry",  "Late, sorry", { &LATE, &SORRY } },
  { "call_me",     "Call me",     { &CALL, &ME1 } },
  { "call_me_now", "Call me now", { &CALL, &ME1, &NOW } },
  { "come_get_me", "Come get me", { &COME, &GET, &ME2 } },
  { "downstairs",  "Downstairs",  { &DOWNSTAIRS } },
  { "almost_here", "Almost here", { &ALMOST, &HERE } },
  { "done_work",   "Done work",   { &DONE, &WORK } },
  { "lunch",       "Lunch?",      { &LUNCH } },
  { "dinner",      "Dinner?",     { &DINNER } },
};
const size_t NUM_MESSAGES = sizeof(MESSAGES) / sizeof(MESSAGES[0]);
const char* REPLIES[] = { "ok", "no", "maybe" };

const LBParam PARAMS[] = {
  { "pair", "Pair code (the same on both panels)", "", 32 },
};
const LBPanelConfig PANEL = { "Couple Message Board", "CoupleBoard", PARAMS, 1 };

#define CONVERSATION_MS (6UL * 60 * 60 * 1000)

String base;              // lonelybinary/couple/<pair>/
int    msgIdx = -1;       // current message, or -1
bool   fromMe = false;    // did this panel send it?
String reply;             // "", ok, no, maybe
uint32_t msgAt = 0;

int findMessage(const String& key) {
  for (size_t i = 0; i < NUM_MESSAGES; i++) if (key == MESSAGES[i].key) return i;
  return -1;
}

bool validReply(const String& r) { return r == "ok" || r == "no" || r == "maybe"; }

void render() {
  lb::clear();
  if (msgIdx >= 0) {
    CRGB c = lb::color(CRGB(255, 90, 140));
    if (fromMe) c.nscale8_video(90);      // what I sent: dim
    for (const LBWord* w : MESSAGES[msgIdx].words) if (w) lb::light(*w, c);
    CRGB rc = lb::color(reply == "no" ? CRGB(255, 40, 30) : reply == "ok" ? CRGB(40, 220, 80) : CRGB(255, 170, 20));
    if (!fromMe) rc.nscale8_video(90);    // my own answer to them: dim
    if (reply == "ok") lb::light(RP_OK, rc);
    else if (reply == "no") lb::light(RP_NO, rc);
    else if (reply == "maybe") { lb::light(RP_MAY, rc); lb::light(RP_BE, rc); }
  }
  if (!lb::mqttConnected()) lb::dot(0, CRGB(80, 0, 0));
  lb::show();
}

bool sendMessage(const String& key) {
  int i = findMessage(key);
  if (i < 0) return false;
  if (!lb::mqttPublish(base + "msg", "{\"from\":\"" + lb::id() + "\",\"m\":\"" + key + "\"}")) return false;
  msgIdx = i; fromMe = true; reply = ""; msgAt = millis();
  lb::requestRedraw();
  return true;
}

bool sendReply(const String& r) {
  if (msgIdx < 0 || fromMe || !validReply(r)) return false;
  if (!lb::mqttPublish(base + "reply", "{\"from\":\"" + lb::id() + "\",\"r\":\"" + r + "\"}")) return false;
  reply = r;
  lb::requestRedraw();
  return true;
}

void onMqtt(const String& topic, const String& payload) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) return;
  String from = doc["from"] | "";
  if (from == lb::id()) return;                  // our own message, echoed back
  if (topic == base + "msg") {
    int i = findMessage(doc["m"] | "");
    if (i < 0) return;
    msgIdx = i; fromMe = false; reply = ""; msgAt = millis();
    lb::sparkle(1500);                           // something new: catch the eye
  } else if (topic == base + "reply") {
    String r = doc["r"] | "";
    if (msgIdx >= 0 && fromMe && validReply(r)) reply = r;
  }
  lb::requestRedraw();
}

bool awaitingMyReply() { return msgIdx >= 0 && !fromMe && reply == ""; }

bool onButtonA() { return awaitingMyReply() ? sendReply("ok") : false; }    // else: next colour
bool onButtonB() {
  if (awaitingMyReply()) { sendReply("no"); return true; }
  msgIdx = -1; reply = "";
  lb::requestRedraw();
  return true;
}

void handleRoot() {
  String h = lb::pageStart("Couple Message Board");
  if (msgIdx >= 0) {
    h += "<p>" + String(fromMe ? "You sent: <b>" : "Your partner says: <b>") + MESSAGES[msgIdx].label + "</b>";
    if (reply.length()) h += String(fromMe ? " &mdash; they said <b>" : " &mdash; you said <b>") + reply + "</b>";
    h += "</p>";
  }
  if (awaitingMyReply()) {
    h += F("<h2>Answer</h2><div class='grid'>");
    for (const char* r : REPLIES)
      h += "<button class='p' onclick=\"go('/api/reply?r=" + String(r) + "')\">" + (String(r) == "maybe" ? "Maybe" : String(r) == "ok" ? "OK" : "No") + "</button>";
    h += F("</div>");
  }
  h += F("<h2>Send</h2><div class='grid'>");
  for (size_t i = 0; i < NUM_MESSAGES; i++)
    h += "<button onclick=\"go('/api/send?m=" + String(MESSAGES[i].key) + "')\">" + MESSAGES[i].label + "</button>";
  h += F("<button onclick=\"go('/api/clear')\">Clear panel</button></div>");
  if (!lb::mqttConnected()) h += F("<p class='m'>Not connected to the message server yet &mdash; messages can't be sent.</p>");
  h += F("<h2>Pairing</h2><p class='m'>Pair code:</p><pre>");
  h += lb::esc(lb::param("pair"));
  h += F("</pre><p class='m'>Type exactly this code into your partner's panel under <a href='/settings'>Settings</a> "
         "(or paste theirs into yours). Both panels must show the same code.</p>");
  h += lb::pageEnd();
  lb::web().send(200, "text/html", h);
}

void setup() {
  lb::begin(PANEL);
  String pair = lb::param("pair");
  pair.trim();
  if (pair.length() == 0) { pair = lb::randomCode(10); lb::setParam("pair", pair); }
  base = "lonelybinary/couple/" + pair + "/";

  lb::onButtons(onButtonA, onButtonB);
  lb::mqttBegin(onMqtt);
  lb::mqttSubscribe(base + "#");

  lb::web().on("/", handleRoot);
  lb::web().on("/api/send", HTTP_ANY, []() {
    bool ok = sendMessage(lb::web().arg("m"));
    lb::web().send(ok ? 200 : 503, "text/plain", ok ? "sent" : "not sent (unknown message, or not connected)");
  });
  lb::web().on("/api/reply", HTTP_ANY, []() {
    bool ok = sendReply(lb::web().arg("r"));
    lb::web().send(ok ? 200 : 409, "text/plain", ok ? "sent" : "nothing to answer");
  });
  lb::web().on("/api/clear", HTTP_ANY, []() {
    msgIdx = -1; reply = ""; lb::requestRedraw();
    lb::web().send(200, "text/plain", "cleared");
  });
}

void loop() {
  static bool wasMqtt = false;
  if (!lb::loop()) return;
  if (lb::mqttConnected() != wasMqtt) { wasMqtt = lb::mqttConnected(); lb::requestRedraw(); }
  if (msgIdx >= 0 && millis() - msgAt > CONVERSATION_MS) { msgIdx = -1; reply = ""; lb::requestRedraw(); }
  if (lb::takeRedraw()) render();
}
