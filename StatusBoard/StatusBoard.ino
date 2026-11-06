/*
 * Status Board — ESP32-S3 Blueprints, Lonely Binary
 * -------------------------------------------------
 * Your office door sign: "I AM IN A MEETING ... ASK ... BACK AT ... OFFICE".
 * Set it from your phone at http://statusboard-xxxx.local, from a script or
 * shortcut (GET /api/set?...), or with button B on the panel. Same board,
 * base and wiring as the Word Clock. Needs the LBPanel library.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      I  A  M  V  B  A  C  K  A  T  R    I AM   BACK AT
 *   r1      B  U  S  Y  F  R  E  E  K  V  D    BUSY | FREE
 *   r2      I  N  A  M  E  E  T  I  N  G  Q    IN A MEETING
 *   r3      O  N  A  C  A  L  L  V  N  O  W    ON A CALL   NOW
 *   r4      F  O  C  U  S  K  T  I  M  E  V    FOCUS TIME
 *   r5      L  U  N  C  H  Q  B  R  E  A  K    LUNCH BREAK
 *   r6      S  H  H  K  P  L  E  A  S  E  V    SHH PLEASE
 *   r7      K  N  O  C  K  O  K  A  S  K  D    KNOCK | OK | ASK
 *   r8      S  O  O  N  L  A  T  E  R  V  Q    SOON | LATER
 *   r9      O  F  F  I  C  E  H  O  M  E  K    OFFICE | HOME
 *
 * Four independent choices, each may be off:
 *   status : busy free meeting call call_now focus lunch shh
 *   door   : knock ok ask          (may I come in?)
 *   back   : soon later back_at    (when I'll be back)
 *   where  : office home
 * "I AM" lights with BUSY / FREE / IN A MEETING / ON A CALL / OFFICE / HOME.
 *
 *   curl "http://statusboard-xxxx.local/api/set?status=meeting&door=ask"
 *   curl "http://statusboard-xxxx.local/api/set?status=&back=soon"   (blank = off)
 *
 * Button B steps through Free -> Busy -> In a meeting -> On a call ->
 * Focus time -> Lunch break -> off. Button A changes the colour.
 */
#include <LBPanel.h>

const LBWord I_AM = {0, 0, 3}, BACK_AT = {0, 4, 6};
const LBWord W_BUSY = {1, 0, 4}, FREE = {1, 4, 4};
const LBWord IN_A_MEETING = {2, 0, 10};
const LBWord ON_A_CALL = {3, 0, 7}, NOW = {3, 8, 3};
const LBWord FOCUS = {4, 0, 5}, TIME = {4, 6, 4};
const LBWord LUNCH = {5, 0, 5}, BREAK = {5, 6, 5};
const LBWord SHH = {6, 0, 3}, PLEASE = {6, 4, 6};
const LBWord KNOCK = {7, 0, 5}, OK_ = {7, 5, 2}, ASK = {7, 7, 3};
const LBWord SOON = {8, 0, 4}, LATER = {8, 4, 5};
const LBWord OFFICE = {9, 0, 6}, HOME = {9, 6, 4};

const LBPanelConfig PANEL = { "Status Board", "StatusBoard", nullptr, 0 };

const char* STATUSES[] = { "busy", "free", "meeting", "call", "call_now", "focus", "lunch", "shh" };
const char* STATUS_LABELS[] = { "Busy", "Free", "In a meeting", "On a call", "On a call now", "Focus time", "Lunch break", "Shh please" };
const char* DOORS[] = { "knock", "ok", "ask" };
const char* DOOR_LABELS[] = { "Knock", "OK", "Ask" };
const char* BACKS[] = { "soon", "later", "back_at" };
const char* BACK_LABELS[] = { "Soon", "Later", "Back at" };
const char* WHERES[] = { "office", "home" };
const char* WHERE_LABELS[] = { "Office", "Home" };

String status, door, back, where_;

bool isOneOf(const String& v, const char* const* list, size_t n) {
  for (size_t i = 0; i < n; i++) if (v == list[i]) return true;
  return false;
}
#define COUNT(a) (sizeof(a) / sizeof(a[0]))

void save() {
  lb::setParam("status", status);
  lb::setParam("door", door);
  lb::setParam("back", back);
  lb::setParam("where", where_);
  Serial.printf("Status -> '%s' door '%s' back '%s' where '%s'\n",
                status.c_str(), door.c_str(), back.c_str(), where_.c_str());
  lb::requestRedraw();
}

void render() {
  lb::clear();
  CRGB busy = CRGB(255, 40, 30), free_ = CRGB(40, 220, 80), amber = CRGB(255, 150, 20), white = CRGB::White;
  CRGB sc = white;
  if (status == "busy" || status == "meeting" || status == "call" || status == "call_now" || status == "shh") sc = busy;
  else if (status == "free") sc = free_;
  else if (status == "focus" || status == "lunch") sc = amber;

  bool iam = false;
  if (status == "busy")          { lb::light(W_BUSY, lb::color(sc)); iam = true; }
  else if (status == "free")     { lb::light(FREE, lb::color(sc)); iam = true; }
  else if (status == "meeting")  { lb::light(IN_A_MEETING, lb::color(sc)); iam = true; }
  else if (status == "call")     { lb::light(ON_A_CALL, lb::color(sc)); iam = true; }
  else if (status == "call_now") { lb::light(ON_A_CALL, lb::color(sc)); lb::light(NOW, lb::color(sc)); iam = true; }
  else if (status == "focus")    { lb::light(FOCUS, lb::color(sc)); lb::light(TIME, lb::color(sc)); }
  else if (status == "lunch")    { lb::light(LUNCH, lb::color(sc)); lb::light(BREAK, lb::color(sc)); }
  else if (status == "shh")      { lb::light(SHH, lb::color(sc)); lb::light(PLEASE, lb::color(sc)); }

  if (door == "knock")    lb::light(KNOCK, lb::color(free_));
  else if (door == "ok")  lb::light(OK_, lb::color(free_));
  else if (door == "ask") lb::light(ASK, lb::color(amber));

  if (back == "soon")         lb::light(SOON, lb::color());
  else if (back == "later")   lb::light(LATER, lb::color());
  else if (back == "back_at") lb::light(BACK_AT, lb::color());

  if (where_ == "office")    { lb::light(OFFICE, lb::color()); iam = true; }
  else if (where_ == "home") { lb::light(HOME, lb::color()); iam = true; }

  if (iam) lb::light(I_AM, lb::color());
  lb::show();
}

// Button B: step through the everyday statuses.
bool onButtonB() {
  static const char* CYCLE[] = { "free", "busy", "meeting", "call", "focus", "lunch", "" };
  size_t n = COUNT(CYCLE), next = 0;
  for (size_t i = 0; i < n; i++) if (status == CYCLE[i]) { next = (i + 1) % n; break; }
  status = CYCLE[next];
  save();
  return true;
}

String group(const char* title, const char* key, const String& cur,
             const char* const* vals, const char* const* labels, size_t n) {
  String h = "<h2>" + String(title) + "</h2><div class='grid'>";
  for (size_t i = 0; i < n; i++) {
    bool on = cur == vals[i];
    h += "<button" + String(on ? " class='on'" : "") + " onclick=\"go('/api/set?" + key + "=" +
         (on ? String("") : String(vals[i])) + "')\">" + labels[i] + "</button>";
  }
  return h + "</div>";
}

void handleRoot() {
  String h = lb::pageStart("Status Board");
  h += F("<p class='m'>Tap to switch on, tap again to switch off.</p>");
  h += group("I am", "status", status, STATUSES, STATUS_LABELS, COUNT(STATUSES));
  h += group("Come in?", "door", door, DOORS, DOOR_LABELS, COUNT(DOORS));
  h += group("Back", "back", back, BACKS, BACK_LABELS, COUNT(BACKS));
  h += group("Where", "where", where_, WHERES, WHERE_LABELS, COUNT(WHERES));
  h += F("<h2>&nbsp;</h2><button onclick=\"go('/api/set?status=&door=&back=&where=')\">Clear all</button>"
         "<h2>From a script or shortcut</h2><pre>curl \"http://");
  h += lb::hostname();
  h += F(".local/api/set?status=meeting&amp;door=ask\"</pre>");
  h += lb::pageEnd();
  lb::web().send(200, "text/html", h);
}

void handleSet() {
  WebServer& s = lb::web();
  String err;
  auto take = [&](const char* key, String& field, const char* const* list, size_t n) {
    if (!s.hasArg(key)) return;
    String v = s.arg(key);
    v.trim();
    v.toLowerCase();
    if (v.length() == 0 || v == "off" || isOneOf(v, list, n)) field = (v == "off") ? "" : v;
    else err += String("unknown ") + key + " '" + v + "'. ";
  };
  take("status", status, STATUSES, COUNT(STATUSES));
  take("door", door, DOORS, COUNT(DOORS));
  take("back", back, BACKS, COUNT(BACKS));
  take("where", where_, WHERES, COUNT(WHERES));
  save();
  String body = "{\"status\":\"" + status + "\",\"door\":\"" + door + "\",\"back\":\"" + back +
                "\",\"where\":\"" + where_ + "\"}";
  if (err.length()) s.send(400, "text/plain", err);
  else s.send(200, "application/json", body);
}

void setup() {
  lb::begin(PANEL);
  status = lb::param("status");
  door   = lb::param("door");
  back   = lb::param("back");
  where_ = lb::param("where");
  lb::onButtons(nullptr, onButtonB);
  lb::web().on("/", handleRoot);
  lb::web().on("/api/set", HTTP_ANY, handleSet);
}

void loop() {
  if (!lb::loop()) return;
  if (lb::takeRedraw()) render();
}
