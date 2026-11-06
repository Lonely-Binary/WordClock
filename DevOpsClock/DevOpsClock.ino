/*
 * DevOps Word Clock — ESP32-S3 Blueprints, Lonely Binary
 * ------------------------------------------------------
 * CI results, deploys, uptime and resource alerts on your desk. Your own
 * tooling tells the panel what happened — from GitHub Actions, a cron job,
 * Uptime Kuma, anything that can make an HTTP request or publish MQTT.
 * Same board, base and wiring as the Word Clock. Needs the LBPanel library.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      C  I  Q  P  A  S  S  F  A  I  L    CI  PASS | FAIL
 *   r1      D  E  P  L  O  Y  Q  L  I  V  E    DEPLOY LIVE
 *   r2      Q  R  O  L  L  B  A  C  K  Z  V    ROLLBACK
 *   r3      O  N  L  I  N  E  Q  D  O  W  N    ONLINE | DOWN*
 *   r4      Q  C  P  U  Q  M  E  M  Q  Z  V    CPU  MEM
 *   r5      Q  L  O  A  D  Q  D  I  S  K  V    LOAD  DISK
 *   r6      Q  Z  H  I  G  H  Q  O  K  Z  V    HIGH*  OK*
 *   r7      B  A  C  K  U  P  Q  F  A  I  L    BACKUP FAIL
 *   r8      Q  D  B  Q  A  P  I  Q  W  E  B    DB  API  WEB
 *   r9      T  R  A  F  F  I  C  Q  E  R  R    TRAFFIC  ERR
 *   * shared words: one "CPU HIGH", "DB DOWN" ... at a time, rotating every
 *     4 seconds when several are true (spec option A).
 *
 * Send it events — over HTTP on your network:
 *   curl -X POST "http://devopsclock-xxxx.local/api/event?name=ci_pass"
 *   curl -X POST "http://devopsclock-xxxx.local/api/status?server=down"
 *   curl -X POST "http://devopsclock-xxxx.local/api/status?db=down"     (db/api/web = down|up)
 *   curl -X POST "http://devopsclock-xxxx.local/api/metric?metric=cpu&state=high"
 * or over MQTT from anywhere (broker.emqx.io, TLS 8883 or plain 1883), with
 * the channel key from Settings — the page shows ready-made commands:
 *   lonelybinary/devops/<key>/event/<ci_pass|ci_fail|deploy_live|rollback|backup_fail|clear>
 *   lonelybinary/devops/<key>/status/<online|down|db_down|db_up|api_down|...>
 *   lonelybinary/devops/<key>/metric   {"metric":"cpu","state":"high"}
 *
 * Events: CI PASS and DEPLOY LIVE show for 60 seconds. CI FAIL, ROLLBACK and
 * BACKUP FAIL stay until the next CI / deploy / backup event, or until you
 * press button B (acknowledge). Metrics: cpu mem load disk traffic err,
 * state high | ok ("ok" shows "CPU OK" for 60 s, then nothing).
 */
#include <LBPanel.h>

const LBWord CI = {0, 0, 2}, PASS = {0, 3, 4}, CI_FAIL = {0, 7, 4};
const LBWord DEPLOY = {1, 0, 6}, LIVE = {1, 7, 4};
const LBWord ROLLBACK = {2, 1, 8};
const LBWord ONLINE = {3, 0, 6}, DOWN = {3, 7, 4};
const LBWord CPU = {4, 1, 3}, MEM = {4, 5, 3};
const LBWord LOAD = {5, 1, 4}, DISK = {5, 6, 4};
const LBWord W_HIGH = {6, 2, 4}, OK_ = {6, 7, 2};
const LBWord BACKUP = {7, 0, 6}, BACKUP_FAIL = {7, 7, 4};
const LBWord DB = {8, 1, 2}, API = {8, 4, 3}, WEB = {8, 8, 3};
const LBWord TRAFFIC = {9, 0, 7}, ERR = {9, 8, 3};

const LBParam PARAMS[] = {
  { "key", "Channel key (keep it secret)", "", 32 },
};
const LBPanelConfig PANEL = { "DevOps Word Clock", "DevOpsClock", PARAMS, 1 };

#define BRIEF_MS  (60UL * 1000)
#define ROTATE_MS 4000

const CRGB RED = CRGB(255, 40, 30), GREEN = CRGB(40, 220, 80), AMBER = CRGB(255, 150, 20);

// CI / deploy / backup
enum { CI_NONE, CI_PASSED, CI_FAILED } ci = CI_NONE;
enum { DEP_NONE, DEP_LIVE, DEP_ROLLBACK } deploy = DEP_NONE;
bool backupFailed = false;
uint32_t ciUntil = 0, deployUntil = 0;   // 0 = until replaced

// Server + services: -1 unknown, 0 up, 1 down
int serverDown = -1;
bool svcDown[3] = { false, false, false };
const char* SVC[] = { "db", "api", "web" };
const LBWord* SVC_WORD[] = { &DB, &API, &WEB };

// Metrics: 0 normal, 1 high, 2 recovered (showing OK until okUntil)
const char* METRIC[] = { "cpu", "mem", "load", "disk", "traffic", "err" };
const LBWord* METRIC_WORD[] = { &CPU, &MEM, &LOAD, &DISK, &TRAFFIC, &ERR };
uint8_t metric[6] = { 0 };
uint32_t okUntil[6] = { 0 };

String channel;   // lonelybinary/devops/<key>/

// One "label + shared word" line, e.g. CPU + HIGH. Declared up here so
// Arduino's auto-generated prototypes can see it.
struct Item { const LBWord* label; const LBWord* state; CRGB col; };
uint8_t rotateIdx = 0;

bool expired(uint32_t until) { return until && (int32_t)(until - millis()) <= 0; }

bool applyEvent(const String& e) {
  if (e == "ci_pass")          { ci = CI_PASSED; ciUntil = millis() + BRIEF_MS; }
  else if (e == "ci_fail")     { ci = CI_FAILED; ciUntil = 0; }
  else if (e == "deploy_live") { deploy = DEP_LIVE; deployUntil = millis() + BRIEF_MS; }
  else if (e == "rollback")    { deploy = DEP_ROLLBACK; deployUntil = 0; }
  else if (e == "backup_fail") { backupFailed = true; }
  else if (e == "backup_ok")   { backupFailed = false; }
  else if (e == "clear")       { ci = CI_NONE; deploy = DEP_NONE; backupFailed = false; }
  else return false;
  if (ciUntil == 0 && ci == CI_PASSED) ciUntil = 1;
  if (deployUntil == 0 && deploy == DEP_LIVE) deployUntil = 1;
  lb::requestRedraw();
  return true;
}

// "online" "down" "db_down" "api_up" ...
bool applyStatus(const String& s) {
  if (s == "online" || s == "up") serverDown = 0;
  else if (s == "down") serverDown = 1;
  else {
    int u = s.indexOf('_');
    if (u < 0) return false;
    String svc = s.substring(0, u), st = s.substring(u + 1);
    int i = -1;
    for (int k = 0; k < 3; k++) if (svc == SVC[k]) i = k;
    if (i < 0 || (st != "down" && st != "up")) return false;
    svcDown[i] = (st == "down");
  }
  lb::requestRedraw();
  return true;
}

bool applyMetric(const String& m, const String& state) {
  for (int i = 0; i < 6; i++) {
    if (m != METRIC[i]) continue;
    if (state == "high") { metric[i] = 1; }
    else if (state == "ok") {
      if (metric[i] == 1) { metric[i] = 2; okUntil[i] = millis() + BRIEF_MS; }
    } else return false;
    lb::requestRedraw();
    return true;
  }
  return false;
}

void onMqtt(const String& topic, const String& payload) {
  if (!topic.startsWith(channel)) return;
  String rest = topic.substring(channel.length());
  if (rest.startsWith("event/")) applyEvent(rest.substring(6));
  else if (rest.startsWith("status/")) applyStatus(rest.substring(7));
  else if (rest == "metric") {
    JsonDocument doc;
    if (deserializeJson(doc, payload)) return;
    applyMetric(String((const char*)(doc["metric"] | "")), String((const char*)(doc["state"] | "")));
  }
}

// The shared-word items currently true, in a fixed order.
uint8_t collect(Item* out) {
  uint8_t n = 0;
  for (int i = 0; i < 3; i++) if (svcDown[i]) out[n++] = { SVC_WORD[i], &DOWN, RED };
  for (int i = 0; i < 6; i++) {
    if (metric[i] == 1) out[n++] = { METRIC_WORD[i], &W_HIGH, AMBER };
    else if (metric[i] == 2 && i < 4) out[n++] = { METRIC_WORD[i], &OK_, GREEN };  // TRAFFIC/ERR have no "OK" pairing in the spec
  }
  return n;
}

void render() {
  lb::clear();
  if (ci != CI_NONE) {
    lb::light(CI, lb::color());
    lb::light(ci == CI_PASSED ? PASS : CI_FAIL, lb::color(ci == CI_PASSED ? GREEN : RED));
  }
  if (deploy == DEP_LIVE) { lb::light(DEPLOY, lb::color(GREEN)); lb::light(LIVE, lb::color(GREEN)); }
  if (deploy == DEP_ROLLBACK) lb::light(ROLLBACK, lb::color(AMBER));
  if (backupFailed) { lb::light(BACKUP, lb::color(RED)); lb::light(BACKUP_FAIL, lb::color(RED)); }

  Item items[9];
  uint8_t n = collect(items);
  if (serverDown == 1) {
    lb::light(DOWN, lb::color(RED));                 // the whole box is down: nothing else matters
  } else {
    if (serverDown == 0) lb::light(ONLINE, lb::color(GREEN));
    if (n) {
      Item& it = items[rotateIdx % n];
      lb::light(*it.label, lb::color(it.col));
      lb::light(*it.state, lb::color(it.col));
    }
  }
  if (channel.length() && !lb::mqttConnected()) lb::dot(0, CRGB(80, 0, 0));
  lb::show();
}

bool onButtonB() {   // acknowledge: clear the sticky failures
  ci = CI_NONE; deploy = DEP_NONE; backupFailed = false;
  for (int i = 0; i < 6; i++) if (metric[i] == 2) metric[i] = 0;
  lb::requestRedraw();
  return true;
}

void handleRoot() {
  String host = lb::hostname() + ".local";
  String h = lb::pageStart("DevOps Word Clock");
  h += F("<h2>Try it</h2><div class='grid'>"
         "<button onclick=\"go('/api/event?name=ci_pass')\">CI pass</button>"
         "<button onclick=\"go('/api/event?name=ci_fail')\">CI fail</button>"
         "<button onclick=\"go('/api/event?name=deploy_live')\">Deploy live</button>"
         "<button onclick=\"go('/api/event?name=rollback')\">Rollback</button>"
         "<button onclick=\"go('/api/event?name=backup_fail')\">Backup fail</button>"
         "<button onclick=\"go('/api/status?server=online')\">Server online</button>"
         "<button onclick=\"go('/api/status?server=down')\">Server down</button>"
         "<button onclick=\"go('/api/status?db=down')\">DB down</button>"
         "<button onclick=\"go('/api/metric?metric=cpu&state=high')\">CPU high</button>"
         "<button onclick=\"go('/api/metric?metric=cpu&state=ok')\">CPU ok</button>"
         "<button onclick=\"go('/api/reset')\">Clear everything</button></div>");
  h += F("<h2>From your network</h2><pre>");
  h += "curl -X POST \"http://" + host + "/api/event?name=ci_fail\"\n"
       "curl -X POST \"http://" + host + "/api/status?api=down\"\n"
       "curl -X POST \"http://" + host + "/api/metric?metric=disk&amp;state=high\"";
  h += F("</pre><h2>From anywhere (MQTT)</h2>");
  if (channel.length()) {
    String k = lb::esc(channel);
    h += F("<pre>");
    h += "mosquitto_pub -h broker.emqx.io -p 8883 --capath /etc/ssl/certs \\\n  -t '" + k + "event/ci_pass' -m 1\n\n"
         "mosquitto_pub -h broker.emqx.io -p 8883 --capath /etc/ssl/certs \\\n  -t '" + k + "metric' -m '{\"metric\":\"cpu\",\"state\":\"high\"}'";
    h += F("</pre><p class='m'>GitHub Actions: add a last step with <code>if: always()</code> that runs the first command "
           "with <code>ci_${{ job.status == 'success' &amp;&amp; 'pass' || 'fail' }}</code>. "
           "Anyone who knows the key can light your panel &mdash; change it in <a href='/settings'>Settings</a>.</p>");
  }
  h += lb::pageEnd();
  lb::web().send(200, "text/html", h);
}

void reply(bool ok) { lb::web().send(ok ? 200 : 400, "text/plain", ok ? "ok" : "unknown value"); }

void setup() {
  lb::begin(PANEL);
  String key = lb::param("key");
  key.trim();
  if (!key.length()) { key = lb::randomCode(12); lb::setParam("key", key); }
  channel = "lonelybinary/devops/" + key + "/";

  lb::onButtons(nullptr, onButtonB);
  lb::mqttBegin(onMqtt);
  lb::mqttSubscribe(channel + "#");

  WebServer& w = lb::web();
  w.on("/", handleRoot);
  w.on("/api/event", HTTP_ANY, []() { reply(applyEvent(lb::web().arg("name"))); });
  w.on("/api/status", HTTP_ANY, []() {
    WebServer& s = lb::web();
    bool ok = s.args() > 0;
    if (s.hasArg("server")) ok &= applyStatus(s.arg("server"));
    for (int i = 0; i < 3; i++) if (s.hasArg(SVC[i])) ok &= applyStatus(String(SVC[i]) + "_" + s.arg(SVC[i]));
    reply(ok);
  });
  w.on("/api/metric", HTTP_ANY, []() { reply(applyMetric(lb::web().arg("metric"), lb::web().arg("state"))); });
  w.on("/api/reset", HTTP_ANY, []() {
    onButtonB();
    serverDown = -1;
    for (int i = 0; i < 3; i++) svcDown[i] = false;
    for (int i = 0; i < 6; i++) metric[i] = 0;
    reply(true);
  });
}

void loop() {
  static uint32_t lastRotate = 0, lastTick = 0;
  static bool wasMqtt = false;
  if (!lb::loop()) return;

  if (millis() - lastTick >= 500) {
    lastTick = millis();
    if (ci == CI_PASSED && expired(ciUntil)) { ci = CI_NONE; lb::requestRedraw(); }
    if (deploy == DEP_LIVE && expired(deployUntil)) { deploy = DEP_NONE; lb::requestRedraw(); }
    for (int i = 0; i < 6; i++) if (metric[i] == 2 && expired(okUntil[i])) { metric[i] = 0; lb::requestRedraw(); }
  }
  if (millis() - lastRotate >= ROTATE_MS) {
    lastRotate = millis();
    Item tmp[9];
    if (collect(tmp) > 1) { rotateIdx++; lb::requestRedraw(); }
  }
  if (lb::mqttConnected() != wasMqtt) { wasMqtt = lb::mqttConnected(); lb::requestRedraw(); }
  if (lb::takeRedraw()) render();
}
