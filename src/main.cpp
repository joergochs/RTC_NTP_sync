#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "ESP8266WiFi.h"
#include "time.h"   
#include "secrets.h" // Enthält WLAN-Zugangsdaten 

// WLAN-Anmeldedaten
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
 
// NTP-Zeitserver und Zeitzone
#define Zeitserver "de.pool.ntp.org"
#define Zeitzone "CET-1CEST,M3.5.0/02,M10.5.0/03"

// Globale Variablen für Zeitverwaltung
time_t aktuelleZeit;
tm Zeit;
int Stunden, Minuten, Sekunden; 

// RTC DS1302 Initialisierung
const int IO = D4;    // DAT
const int SCLK = D5;  // CLK
const int CE = D2;    // RST

ThreeWire myWire(D4, D5, D2);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(9600);

  // Zeitzone und NTP-Server konfigurieren
  configTzTime(Zeitzone, Zeitserver);
 
  WiFi.mode(WIFI_STA);
 
  // WiFi-Verbindung starten
  WiFi.begin(ssid, password);
 
  Serial.println("------------------------");
 
  // Warten bis WiFi verbunden ist
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
 
  Serial.println();
  Serial.print("Verbunden mit ");
  Serial.println(ssid);
  Serial.print("IP über DHCP: ");
  Serial.println(WiFi.localIP());
 
  // Aktuelle Zeit von NTP-Server holen
  time(&aktuelleZeit);
 
  // Zeit in lokale Zeitzone konvertieren
  localtime_r(&aktuelleZeit, &Zeit);
 
  // String für Jahresvergleich vorbereiten (wird zum Verifizieren der Zeitsynchronisation verwendet)
  String Jahr = String(Zeit.tm_year + 1900);
  int Zaehler = 0;
 
  // Nach "1970" suchen (Standard-Jahr bei fehlender Zeitsynchronisation)
  int Suche = Jahr.indexOf("1970");
 
  Serial.println("-------------------------");
  Serial.println("Datum und Zeit holen (maximal 90 Sekunden)...");
 
  // Schleife bis Zeitserver erfolgreich synchronisiert
  while (Suche != -1)
  {
    // Aktuelle Zeit erneut abrufen
    time(&aktuelleZeit);
 
    // In lokale Zeitzone konvertieren
    localtime_r(&aktuelleZeit, &Zeit);
    Jahr = String(Zeit.tm_year + 1900);
 
    // Prüfe ob Jahr noch auf 1970 gesetzt ist (fehlende Synchronisation)
    Suche = Jahr.indexOf("1970");
 
    // Zeit auslesen (Stunden, Minuten, Sekunden)
    Stunden = int(Zeit.tm_hour), Minuten = int(Zeit.tm_min), Sekunden = int(Zeit.tm_sec);
    delay(1000);
    Zaehler++;
 
    // Timeout nach 90 Sekunden
    if (Zaehler >= 90)
    {
      Serial.println();
      Serial.println("Datum und Zeit konnte innerhalb von " + String(Zaehler) + " Sekunden nicht geholt werden");
      Serial.println("Programm wird beendet");
 
      // Endlosschleife zur Programmunterbrechung
      while (1);
    }
 
    Serial.print(".");
  }
 
  Serial.println();
 
  // Zeige erfolgreiche Zeitsynchronisation an
  if (Suche == -1)
  {
    Serial.println("-------------------------");
    Serial.println("Datum/Zeit erfolgreich synchronisiert ...");
 
    // Ausgabe Datum: dd.mm.yyyy
    if (Zeit.tm_mday < 10) Serial.print("0");
    Serial.print(Zeit.tm_mday);
    Serial.print(".");
 
    // Monat: führende 0 ergänzen (tm_mon läuft von 0-11)
    if (Zeit.tm_mon < 9) Serial.print("0");
 
    // Monatszählung beginnt mit 0, daher +1
    Serial.print(Zeit.tm_mon + 1);
    Serial.print(".");
 
    // Jahre seit 1900 (daher +1900 zum aktuellen Jahr)
    Serial.println(Zeit.tm_year + 1900);
 
    // Ausgabe Zeit: hh:mm:ss
    if (Zeit.tm_hour < 10) Serial.print("0");
    Serial.print(Zeit.tm_hour);
    Serial.print(":");
 
    if (Zeit.tm_min < 10) Serial.print("0");
    Serial.print(Zeit.tm_min);
    Serial.print(":");
 
    if (Zeit.tm_sec < 10) Serial.print("0");
    Serial.println(Zeit.tm_sec);
    Serial.println("-------------------------");
  }

  Rtc.Begin();

  char date_buffer[20]; 
  char time_buffer[12]; 
  // sprintf konvertiert die Integer in einen formatierten String im Buffer
  sprintf(date_buffer, "Jan %02d %04d", Zeit.tm_mday, Zeit.tm_year + 1900);
  sprintf(time_buffer, "%02d:%02d:%02d", Zeit.tm_hour, Zeit.tm_min, Zeit.tm_sec);

  RtcDateTime NTPtime = RtcDateTime(date_buffer,time_buffer);

  if (!Rtc.IsDateTimeValid()) {
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC verifiziert Zeit als ungültig, setze Zeit auf NTP Zeit");
    Rtc.SetDateTime(NTPtime);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC ist schreibgeschuetzt, Aufhebung des Schutzes");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC ist gestoppt, Starte RTC");
    Rtc.SetIsRunning(true);
  }

//Rtc.SetDateTime(NTPtime);
  
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  Serial.print(now.Hour());
  Serial.print(":");
  Serial.println(now.Minute());

  delay(2000);
}
