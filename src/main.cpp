#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "secrets.h"
#include <RtcDS1302.h>

// WLAN-Daten
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// NTP Server Einstellungen
const char* ntpServer = "de.pool.ntp.org";
const long  gmtOffset_sec = 3600; // Für Deutschland (MEZ), bei Sommerzeit 7200
const int   daylightOffset_sec = 3600;

// DS1302 Pin Definitionen (Beispiel)
// Ersetzen Sie D2, D3, D4 durch die tatsächlichen Pins Ihres Mikrocontrollers
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> rtc(myWire);

void setup() {
  Serial.begin(115200);
  rtc.begin();

  // Verbindung mit WLAN herstellen
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WLAN verbunden.");

  // Zeit vom NTP-Server abfragen
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Warten, bis die Zeit empfangen wurde
  time_t now  = time(nullptr);
  while (now < 24 * 3600) { // Prüfen ob die Zeit gültig ist
      delay(100);
      now = time(nullptr);
  }

  // NTP-Zeit in DS1302 schreiben
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  rtc.setDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  Serial.println("DS1302 Uhrzeit wurde erfolgreich via NTP gesetzt.");
}

void loop() {
  // Optional: Die Zeit in regelmäßigen Abständen erneut synchronisieren (z.B. einmal täglich)
  // und die aktuelle Zeit aus der RTC auslesen und anzeigen.
}