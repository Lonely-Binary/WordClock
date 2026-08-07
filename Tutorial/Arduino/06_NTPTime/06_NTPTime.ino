/*
 * Lesson 6 — NTP Time
 * -------------------
 * Once online, fetch the real current time over the internet (NTP) and
 * print it to Serial every second. No panel rendering yet — that's Lesson 7.
 *
 * Board: ESP32-S3 (esp32:esp32:esp32s3)
 */

#include <WiFi.h>
#include <time.h>

// ----------------------------------------------------------------------------
// USER CONFIG — edit these
// ----------------------------------------------------------------------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// US Eastern Time, with automatic daylight saving. POSIX TZ rule:
// "EST5EDT,M3.2.0,M11.1.0" = EST is UTC-5, EDT (DST) is UTC-4, DST runs from
// the 2nd Sunday in March to the 1st Sunday in November.
// Change this to your own timezone if you're elsewhere.
const char* TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";
const char* NTP1 = "us.pool.ntp.org";
const char* NTP2 = "time.google.com";
const char* NTP3 = "time.nist.gov";

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

void syncTime() {
  configTzTime(TZ_INFO, NTP1, NTP2, NTP3);
  Serial.print("NTP syncing");
  struct tm t;
  while (!getLocalTime(&t, 500)) {
    Serial.print('.');
  }
  Serial.printf("\nTime: %04d-%02d-%02d %02d:%02d:%02d\n",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                t.tm_hour, t.tm_min, t.tm_sec);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  connectWiFi();
  syncTime();
}

void loop() {
  struct tm t;
  if (getLocalTime(&t, 0)) {
    Serial.printf("Now: %02d:%02d:%02d\n", t.tm_hour, t.tm_min, t.tm_sec);
  }
  delay(1000);
}
