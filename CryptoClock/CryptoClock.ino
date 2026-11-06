/*
 * Crypto Word Clock — ESP32-S3 Blueprints, Lonely Binary
 * ------------------------------------------------------
 * The market's mood in words (MOON / DUMP / ATH / DIP / FOMO / FUD), plus
 * meme buttons your friend group sends each other (TO THE MOON, BUY THE
 * DIP, HODL...) and LOL / RIP / SAME replies. Same board, base and wiring
 * as the Word Clock. Needs the LBPanel library. Not financial advice — it's
 * a lamp.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      Q  Z  M  O  O  N  D  U  M  P  K    MOON | DUMP        (market)
 *   r1      R  K  V  A  T  H  Q  Z  D  I  P    ATH  DIP           (market)
 *   r2      Q  F  O  M  O  Z  K  F  U  D  V    FOMO  FUD          (market)
 *   r3      Q  R  E  K  T  Z  W  H  A  L  E    REKT  WHALE        (feed)
 *   r4      Q  Z  A  P  E  K  S  E  N  D  V    APE  SEND          (friends)
 *   r5      T  O  Q  T  H  E  Z  M  O  O  N    TO THE MOON        (friends)
 *   r6      B  U  Y  Q  T  H  E  Z  D  I  P    BUY THE DIP        (friends)
 *   r7      Q  Z  R  U  G  P  U  L  L  K  V    RUGPULL            (friends)
 *   r8      Q  L  O  L  R  I  P  S  A  M  E    LOL | RIP | SAME   (replies)
 *   r9      Q  H  O  D  L  Z  P  A  N  I  C    HODL | PANIC       (friends)
 *
 * Market words — the panel fetches these itself, no keys needed:
 *   MOON / DUMP  24 h change >= +5% / <= -5%  (CoinGecko), clears inside +-3%
 *   ATH / DIP    price within 0.5% of its all-time high / more than 10% below
 *   FOMO / FUD   Fear & Greed index >= 75 / <= 25 (alternative.me)
 * Coin defaults to bitcoin; any CoinGecko id works (ethereum, solana, ...).
 *
 * REKT and WHALE come from a market feed published on MQTT at
 *   lonelybinary/crypto/global/trigger/rekt   (and .../whale)
 * — they need a backend watching liquidations / big transfers, which isn't
 * part of the panel. Anyone can publish there to test.
 *
 * Friends: everyone in a group types the same group code in Settings. A
 * button pressed on any phone lights the word on every panel in the group
 * (including your own) for 60 seconds; HODL / PANIC stay for 10 minutes.
 * Topics: lonelybinary/crypto/group/<code>/manual/<name> and .../reply/<name>
 */
#include <LBPanel.h>

const LBWord MOON = {0, 2, 4}, DUMP = {0, 6, 4};
const LBWord ATH = {1, 3, 3}, DIP = {1, 8, 3};
const LBWord FOMO = {2, 1, 4}, FUD = {2, 7, 3};
const LBWord REKT = {3, 1, 4}, WHALE = {3, 6, 5};
const LBWord APE = {4, 2, 3}, SEND = {4, 6, 4};
const LBWord TO2 = {5, 0, 2}, THE2 = {5, 3, 3}, MOON2 = {5, 7, 4};
const LBWord BUY3 = {6, 0, 3}, THE3 = {6, 4, 3}, DIP3 = {6, 8, 3};
const LBWord RUGPULL = {7, 2, 7};
const LBWord LOL = {8, 1, 3}, RIP = {8, 4, 3}, SAME = {8, 7, 4};
const LBWord HODL = {9, 1, 4}, PANIC = {9, 6, 5};

const LBParam PARAMS[] = {
  { "coin",  "Coin (CoinGecko id, e.g. bitcoin)", "bitcoin", 32 },
  { "group", "Friend group code (same for everyone)", "", 32 },
};
const LBPanelConfig PANEL = { "Crypto Word Clock", "CryptoClock", PARAMS, 2 };

#define SHOW_MS        (60UL * 1000)
#define SHOW_STANCE_MS (10UL * 60 * 1000)
#define MARKET_EVERY_MS (3UL * 60 * 1000)
#define FNG_EVERY_MS    (60UL * 60 * 1000)

// Things that light for a while after they're triggered.
struct Timed { const char* key; const char* label; uint32_t until; };
Timed TIMED[] = {
  { "rekt", "REKT", 0 }, { "whale", "Whale", 0 },
  { "ape", "Ape", 0 }, { "send", "Send", 0 }, { "tothemoon", "To the moon", 0 },
  { "buythedip", "Buy the dip", 0 }, { "rugpull", "Rugpull", 0 },
  { "hodl", "HODL", 0 }, { "panic", "Panic", 0 },
  { "lol", "LOL", 0 }, { "rip", "RIP", 0 }, { "same", "Same", 0 },
};
const size_t NUM_TIMED = sizeof(TIMED) / sizeof(TIMED[0]);
const char* MANUAL[] = { "ape", "send", "tothemoon", "buythedip", "rugpull", "hodl", "panic" };
const char* REPLY[] = { "lol", "rip", "same" };

Timed* findTimed(const String& k) {
  for (size_t i = 0; i < NUM_TIMED; i++) if (k == TIMED[i].key) return &TIMED[i];
  return nullptr;
}
bool active(const char* k) {
  Timed* t = findTimed(k);
  return t && t->until && (int32_t)(t->until - millis()) > 0;
}
void trigger(const String& k) {
  Timed* t = findTimed(k);
  if (!t) return;
  bool stance = (k == "hodl" || k == "panic");
  if (k == "hodl") findTimed("panic")->until = 0;          // one or the other
  if (k == "panic") findTimed("hodl")->until = 0;
  if (k == "lol" || k == "rip" || k == "same")               // one reply at a time
    for (const char* r : REPLY) if (k != r) findTimed(r)->until = 0;
  t->until = millis() + (stance ? SHOW_STANCE_MS : SHOW_MS);
  if (t->until == 0) t->until = 1;
  lb::requestRedraw();
}

// Market state, each -1 / 0 / +1 with hysteresis.
int moveState = 0;     // +1 MOON, -1 DUMP
int athState = 0;      // +1 ATH, -1 DIP
int moodState = 0;     // +1 FOMO, -1 FUD
float price = 0, change24 = 0, ath = 0;
int fng = -1;
bool marketFailed = false;
String coin, groupBase;

bool fetchMarket() {
  JsonDocument doc;
  if (!lb::getJson("https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&ids=" + lb::urlEncode(coin), doc))
    return false;
  JsonVariant m = doc[0];
  if (m.isNull()) { Serial.printf("Crypto: CoinGecko doesn't know '%s'\n", coin.c_str()); return false; }
  price = m["current_price"] | 0.0f;
  change24 = m["price_change_percentage_24h"] | 0.0f;
  ath = m["ath"] | 0.0f;

  if (change24 >= 5) moveState = 1;
  else if (change24 <= -5) moveState = -1;
  else if (fabs(change24) < 3) moveState = 0;

  if (ath > 0) {
    float below = (ath - price) / ath * 100;   // % under the all-time high
    if (below <= 0.5) athState = 1;
    else if (below > 10) athState = -1;
    else if (athState == 1 && below > 2) athState = 0;
    else if (athState == -1 && below < 8) athState = 0;
  }
  Serial.printf("Crypto: %s $%.2f, 24h %+.2f%%, ATH $%.2f\n", coin.c_str(), price, change24, ath);
  return true;
}

bool fetchFearGreed() {
  JsonDocument doc;
  if (!lb::getJson("https://api.alternative.me/fng/?limit=1", doc)) return false;
  const char* v = doc["data"][0]["value"] | "";
  if (!*v) return false;
  fng = atoi(v);
  if (fng >= 75) moodState = 1;
  else if (fng <= 25) moodState = -1;
  else if (fng > 30 && fng < 70) moodState = 0;
  Serial.printf("Crypto: Fear & Greed %d\n", fng);
  return true;
}

void onMqtt(const String& topic, const String& payload) {
  (void)payload;
  int slash = topic.lastIndexOf('/');
  String name = topic.substring(slash + 1);
  String parent = topic.substring(0, slash);
  if (parent == "lonelybinary/crypto/global/trigger" && (name == "rekt" || name == "whale")) trigger(name);
  else if (groupBase.length() && (parent == groupBase + "manual" || parent == groupBase + "reply")) trigger(name);
}

bool send(const String& kind, const String& name) {
  if (!groupBase.length()) return false;
  return lb::mqttPublish(groupBase + kind + "/" + name, "{\"from\":\"" + lb::id() + "\"}");
}

void render() {
  lb::clear();
  CRGB market = CRGB(255, 140, 20), friends = CRGB(60, 120, 255);
  if (moveState > 0) lb::light(MOON, lb::color(CRGB(40, 220, 80)));
  if (moveState < 0) lb::light(DUMP, lb::color(CRGB(255, 40, 30)));
  if (athState > 0)  lb::light(ATH, lb::color(CRGB(40, 220, 80)));
  if (athState < 0)  lb::light(DIP, lb::color(market));
  if (moodState > 0) lb::light(FOMO, lb::color(market));
  if (moodState < 0) lb::light(FUD, lb::color(market));
  if (active("rekt"))  lb::light(REKT, lb::color(CRGB(255, 40, 30)));
  if (active("whale")) lb::light(WHALE, lb::color(market));
  if (active("ape"))   lb::light(APE, lb::color(friends));
  if (active("send"))  lb::light(SEND, lb::color(friends));
  if (active("tothemoon")) { lb::light(TO2, lb::color(friends)); lb::light(THE2, lb::color(friends)); lb::light(MOON2, lb::color(friends)); }
  if (active("buythedip")) { lb::light(BUY3, lb::color(friends)); lb::light(THE3, lb::color(friends)); lb::light(DIP3, lb::color(friends)); }
  if (active("rugpull")) lb::light(RUGPULL, lb::color(CRGB(255, 40, 30)));
  if (active("lol"))   lb::light(LOL, lb::color(CRGB(255, 200, 0)));
  if (active("rip"))   lb::light(RIP, lb::color(CRGB(255, 40, 30)));
  if (active("same"))  lb::light(SAME, lb::color(CRGB(40, 220, 80)));
  if (active("hodl"))  lb::light(HODL, lb::color(friends));
  if (active("panic")) lb::light(PANIC, lb::color(friends));
  if (marketFailed) lb::dot(0, CRGB(80, 0, 0));
  if (groupBase.length() && !lb::mqttConnected()) lb::dot(1, CRGB(80, 0, 0));
  lb::show();
}

void handleRoot() {
  String h = lb::pageStart("Crypto Word Clock");
  if (price > 0)
    h += "<p>" + lb::esc(coin) + " $" + String(price, price < 10 ? 4 : 2) + " &middot; 24 h " + String(change24, 2) +
         "% &middot; " + String(ath > 0 ? (ath - price) / ath * 100 : 0, 1) + "% under ATH";
  else
    h += F("<p>No market data yet.");
  if (fng >= 0) h += " &middot; Fear &amp; Greed " + String(fng);
  h += F("</p>");
  if (groupBase.length()) {
    h += F("<h2>Send to your group</h2><div class='grid'>");
    for (const char* k : MANUAL) h += "<button onclick=\"go('/api/send?m=" + String(k) + "')\">" + findTimed(k)->label + "</button>";
    h += F("</div><h2>Reply</h2><div class='grid'>");
    for (const char* k : REPLY) h += "<button onclick=\"go('/api/reply?r=" + String(k) + "')\">" + findTimed(k)->label + "</button>";
    h += F("</div>");
    if (!lb::mqttConnected()) h += F("<p class='m'>Not connected to the message server yet.</p>");
  } else {
    h += F("<p class='m'>Add a friend group code in <a href='/settings'>Settings</a> to send TO THE MOON, "
           "BUY THE DIP and friends to each other. Everyone in the group uses the same code.</p>");
  }
  h += F("<p class='m'>Prices by CoinGecko, Fear &amp; Greed by alternative.me. Not financial advice.</p>");
  h += lb::pageEnd();
  lb::web().send(200, "text/html", h);
}

void setup() {
  lb::begin(PANEL);
  coin = lb::param("coin");
  coin.trim();
  coin.toLowerCase();
  if (!coin.length()) coin = "bitcoin";
  String group = lb::param("group");
  group.trim();
  if (group.length()) groupBase = "lonelybinary/crypto/group/" + group + "/";

  lb::mqttBegin(onMqtt);
  lb::mqttSubscribe("lonelybinary/crypto/global/trigger/#");
  if (groupBase.length()) lb::mqttSubscribe(groupBase + "#");

  lb::web().on("/", handleRoot);
  lb::web().on("/api/send", HTTP_ANY, []() {
    String m = lb::web().arg("m");
    bool known = false;
    for (const char* k : MANUAL) if (m == k) known = true;
    bool ok = known && send("manual", m);
    lb::web().send(ok ? 200 : 400, "text/plain", ok ? "sent" : "not sent");
  });
  lb::web().on("/api/reply", HTTP_ANY, []() {
    String r = lb::web().arg("r");
    bool ok = (r == "lol" || r == "rip" || r == "same") && send("reply", r);
    lb::web().send(ok ? 200 : 400, "text/plain", ok ? "sent" : "not sent");
  });
}

void loop() {
  static uint32_t lastMarket = 0, lastFng = 0;
  static bool first = true, wasMqtt = false;
  if (!lb::loop()) return;

  if (first || millis() - lastMarket >= MARKET_EVERY_MS) {
    lastMarket = millis();
    marketFailed = !fetchMarket();
    lb::requestRedraw();
  }
  if (first || millis() - lastFng >= FNG_EVERY_MS || (fng < 0 && millis() - lastFng >= MARKET_EVERY_MS)) {
    lastFng = millis();
    fetchFearGreed();
    lb::requestRedraw();
  }
  first = false;

  // Timed words going out.
  static uint32_t lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    for (size_t i = 0; i < NUM_TIMED; i++)
      if (TIMED[i].until && (int32_t)(TIMED[i].until - millis()) <= 0) { TIMED[i].until = 0; lb::requestRedraw(); }
  }
  if (lb::mqttConnected() != wasMqtt) { wasMqtt = lb::mqttConnected(); lb::requestRedraw(); }
  if (lb::takeRedraw()) render();
}
