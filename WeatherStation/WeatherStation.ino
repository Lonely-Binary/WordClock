/*
 * Weather Station — ESP32-S3 Blueprints, Lonely Binary
 * ----------------------------------------------------
 * "IT IS CLOUDY ... NOW ... FEELS LIKE COOL", from Open-Meteo (free, no API
 * key). Same board, base and wiring as the Word Clock; only the faceplate
 * and this sketch differ. Needs the LBPanel library from ../libraries.
 *
 * Setup: join the "Weather-XXXX" hotspot, pick your WiFi, type your city.
 * Change the city later at http://weather-xxxx.local/settings.
 *
 *   col ->  0  1  2  3  4  5  6  7  8  9 10
 *   r0      I  T  Q  I  S  V  Z  K  D  M  R    IT  IS
 *   r1      S  U  N  N  Y  C  L  O  U  D  Y    SUNNY | CLOUDY
 *   r2      R  A  I  N  Y  S  N  O  W  Y  B    RAINY | SNOWY
 *   r3      W  I  N  D  Y  F  O  G  G  Y  K    WINDY | FOGGY
 *   r4      S  T  O  R  M  Y  C  L  E  A  R    STORMY | CLEAR
 *   r5      H  O  T  C  O  L  D  M  I  L  D    HOT | COLD | MILD
 *   r6      W  A  R  M  C  O  O  L  D  R  Y    WARM | COOL | DRY
 *   r7      B  R  E  E  Z  Y  C  A  L  M  T    BREEZY | CALM
 *   r8      T  O  N  I  G  H  T  N  O  W  D    TONIGHT | NOW
 *   r9      F  E  E  L  S  V  L  I  K  E  R    FEELS LIKE
 *
 * What lights:
 *   IT IS <sky>        sky from the WMO weather code; CLEAR at night instead
 *                      of SUNNY; WINDY replaces a clear/cloudy sky above 40 km/h
 *   NOW / TONIGHT      day or night at your location
 *   FEELS LIKE <temp>  apparent temperature: HOT >=30, WARM >=24, MILD >=16,
 *                      COOL >=8, else COLD (degrees C)
 *   DRY                humidity under 30% (only beside HOT / COLD / MILD —
 *                      WARM / COOL share its row)
 *   BREEZY / CALM      wind 15-40 km/h / under 6 km/h
 * The first corner dot glows red while the last update failed.
 */
#include <LBPanel.h>

const LBWord IT = {0, 0, 2}, IS = {0, 3, 2};
const LBWord SUNNY = {1, 0, 5}, CLOUDY = {1, 5, 6};
const LBWord RAINY = {2, 0, 5}, SNOWY = {2, 5, 5};
const LBWord WINDY = {3, 0, 5}, FOGGY = {3, 5, 5};
const LBWord STORMY = {4, 0, 6}, CLEAR = {4, 6, 5};
const LBWord HOT = {5, 0, 3}, COLD = {5, 3, 4}, MILD = {5, 7, 4};
const LBWord WARM = {6, 0, 4}, COOL = {6, 4, 4}, DRY = {6, 8, 3};
const LBWord BREEZY = {7, 0, 6}, CALM = {7, 6, 4};
const LBWord TONIGHT = {8, 0, 7}, NOW = {8, 7, 3};
const LBWord FEELS = {9, 0, 5}, LIKE = {9, 6, 4};

const LBParam PARAMS[] = {
  { "city", "City (e.g. Sydney, or lat,lon)", "Sydney", 40 },
};
const LBPanelConfig PANEL = { "Weather Station", "Weather", PARAMS, 1 };

#define UPDATE_EVERY_MS (10UL * 60 * 1000)
#define RETRY_EVERY_MS  (60UL * 1000)

struct Weather {
  bool  valid = false;
  int   code = 0;
  bool  isDay = true;
  float feels = 0, humidity = 50, wind = 0;
} wx;

bool     lastFailed = false;
uint32_t lastFetch = 0;
bool     fetchedOnce = false;
float    lat = NAN, lon = NAN;
String   placeName;

// "Sydney" -> lat/lon via Open-Meteo geocoding; "-33.87,151.21" is used as is.
bool resolveLocation() {
  String city = lb::param("city");
  int comma = city.indexOf(',');
  if (comma > 0) {
    float a = city.substring(0, comma).toFloat(), b = city.substring(comma + 1).toFloat();
    if (a != 0 || b != 0) { lat = a; lon = b; placeName = city; return true; }
  }
  JsonDocument doc;
  if (!lb::getJson("https://geocoding-api.open-meteo.com/v1/search?count=1&format=json&name=" + lb::urlEncode(city), doc))
    return false;
  JsonVariant r = doc["results"][0];
  if (r.isNull()) { Serial.printf("Weather: no place called '%s'\n", city.c_str()); return false; }
  lat = r["latitude"];
  lon = r["longitude"];
  placeName = String((const char*)r["name"]) + ", " + String((const char*)(r["country"] | ""));
  Serial.printf("Weather: %s -> %.3f, %.3f\n", placeName.c_str(), lat, lon);
  return true;
}

bool fetchWeather() {
  if (isnan(lat) && !resolveLocation()) return false;
  String url = "https://api.open-meteo.com/v1/forecast?timezone=auto&wind_speed_unit=kmh"
               "&current=apparent_temperature,relative_humidity_2m,is_day,weather_code,wind_speed_10m"
               "&latitude=" + String(lat, 4) + "&longitude=" + String(lon, 4);
  JsonDocument doc;
  if (!lb::getJson(url, doc)) return false;
  JsonVariant c = doc["current"];
  if (c.isNull()) return false;
  wx.code     = c["weather_code"] | 0;
  wx.isDay    = (c["is_day"] | 1) == 1;
  wx.feels    = c["apparent_temperature"] | 0.0f;
  wx.humidity = c["relative_humidity_2m"] | 50.0f;
  wx.wind     = c["wind_speed_10m"] | 0.0f;
  wx.valid    = true;
  Serial.printf("Weather: code %d, feels %.1fC, RH %.0f%%, wind %.0f km/h, %s\n",
                wx.code, wx.feels, wx.humidity, wx.wind, wx.isDay ? "day" : "night");
  return true;
}

// WMO weather interpretation codes -> one sky word.
const LBWord* skyWord() {
  int c = wx.code;
  const LBWord* w;
  if (c >= 95) w = &STORMY;
  else if ((c >= 71 && c <= 77) || c == 85 || c == 86) w = &SNOWY;
  else if ((c >= 51 && c <= 67) || (c >= 80 && c <= 82)) w = &RAINY;
  else if (c == 45 || c == 48) w = &FOGGY;
  else if (c >= 2) w = &CLOUDY;
  else w = wx.isDay ? &SUNNY : &CLEAR;
  if ((w == &CLOUDY || w == &SUNNY || w == &CLEAR) && wx.wind >= 40) w = &WINDY;
  return w;
}

void render() {
  lb::clear();
  lb::light(IT, lb::color());
  lb::light(IS, lb::color());
  if (wx.valid) {
    const LBWord* sky = skyWord();
    CRGB skyCol = (sky == &SUNNY) ? CRGB(255, 180, 40)
                : (sky == &RAINY || sky == &STORMY) ? CRGB(60, 120, 255)
                : CRGB::White;
    lb::light(*sky, lb::color(skyCol));
    lb::light(wx.isDay ? NOW : TONIGHT, lb::color());

    const LBWord* t;
    CRGB tCol;
    if      (wx.feels >= 30) { t = &HOT;  tCol = CRGB(255, 60, 20); }
    else if (wx.feels >= 24) { t = &WARM; tCol = CRGB(255, 140, 20); }
    else if (wx.feels >= 16) { t = &MILD; tCol = CRGB::White; }
    else if (wx.feels >= 8)  { t = &COOL; tCol = CRGB(80, 170, 255); }
    else                     { t = &COLD; tCol = CRGB(40, 90, 255); }
    lb::light(FEELS, lb::color());
    lb::light(LIKE, lb::color());
    lb::light(*t, lb::color(tCol));
    if (wx.humidity < 30 && t->row == 5) lb::light(DRY, lb::color());

    if (sky != &WINDY && wx.wind >= 15 && wx.wind < 40) lb::light(BREEZY, lb::color());
    else if (wx.wind < 6) lb::light(CALM, lb::color());
  }
  if (lastFailed) lb::dot(0, CRGB(80, 0, 0));
  lb::show();
}

void handleRoot() {
  String h = lb::pageStart("Weather Station");
  if (wx.valid) {
    h += "<p>" + lb::esc(placeName) + "<br>Feels like " + String(wx.feels, 1) + " &deg;C &middot; humidity " +
         String(wx.humidity, 0) + "% &middot; wind " + String(wx.wind, 0) + " km/h</p>";
  } else {
    h += F("<p>No weather yet.</p>");
  }
  if (lastFailed) h += F("<p class='m'>The last update failed &mdash; check the city name in Settings.</p>");
  h += F("<div class='grid'><button onclick=\"go('/api/refresh')\">Update now</button>"
         "<a class='b' href='/settings'>Change city</a></div>"
         "<p class='m'>Weather data by <a href='https://open-meteo.com/'>Open-Meteo.com</a>, updated every 10 minutes.</p>");
  h += lb::pageEnd();
  lb::web().send(200, "text/html", h);
}

void setup() {
  lb::begin(PANEL);
  lb::web().on("/", handleRoot);
  lb::web().on("/api/refresh", HTTP_ANY, []() {
    lastFetch = 0; fetchedOnce = false;
    lb::web().send(200, "text/plain", "ok");
  });
}

void loop() {
  if (!lb::loop()) return;

  uint32_t wait = lastFailed ? RETRY_EVERY_MS : UPDATE_EVERY_MS;
  if (!fetchedOnce || millis() - lastFetch >= wait) {
    lastFetch = millis();
    fetchedOnce = true;
    lastFailed = !fetchWeather();
    lb::requestRedraw();
  }
  if (lb::takeRedraw()) render();
}
