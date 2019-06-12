/*  Simplest ESP8266 Clock
 *
 *  This project uses a WiMos D1 Mini ESP8266 module and a 4-digit,
 *  TM1637-based LED display module to create an internet time server 
 *  synced clock.
 *
 *    WeMos        TM1637
 *    D1 Mini      Display
 *    ----+        +----
 *     D6 |--------| CLK
 *     D5 |--------| DIO
 *    GND |--------| GND
 *    +5V |--------| +5V
 *    ----+        +----
 *
 *  * uses WiMos Mini D1 ESP8266 WiFi module
 *  * uses 4-digit TM1637-based LED display module
 *  
 *  Pros:
 *  * minimum hardware and software
 *  * periodically synced to NTP time server
 *  
 *  Cons:
 *  * hardcoded WiFi SSID and password for simplicity
 *  * hardcoded timezone and DST offset
 *  * must be connected to internet
 *  * no battery-backed RTC
 *  * 24 hour time by default
 *  * needs a cool looking case
 */

#include <TimeLib.h> // https://github.com/PaulStoffregen/Time

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi

#include <TM1637Display.h> // https://github.com/avishorp/TM1637

#define DBprint(x) Serial.print(x)
#define DBprintln(x) Serial.println(x)

const int TM1637_CLKPIN = D6; // TM1637 CLK pin
const int TM1637_DIOPIN = D5; // TM1637 DIO pin

// LED display
TM1637Display tm1637(TM1637_CLKPIN, TM1637_DIOPIN);

// WLAN credentials
const char *ssid     = "<SSID>";
const char *password = "<PASSWORD>";

// UTC timezone
const int timezone = -5;  // eastern standard time

// DST offset
const int dstOffset = 1; // daylight savings time offset

const time_t ntpSyncInterval = 24*60*60; // resync every 24 hours

/*
 *  arduino setup routine; init serial port, connect to WiFi, init LED display
 */
void setup() {
  Serial.begin(115200);

  DBprintln("Simpest ESP8266 Clock");

  // connect to WLAN
  DBprint("SSID: "); DBprintln(ssid);
  DBprint("Password: "); DBprintln(password);
  DBprintln("Connecting to WiFi");
//  WiFi.enableInsecureWEP(); // needed for WEP networks?
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    DBprint(".");
    delay(1000);
  }
  DBprintln("\nConnected");
  DBprint("IP address: "); DBprintln(WiFi.localIP());

  // set LED brightness 0 (lowest) to 7 (highest)
  tm1637.setBrightness(7);
}

/*
 *  arduino loop routine
 */
void loop() {
  getNtpTime();
  displayTime();
}

/*
 *  periodically sync clock with NTP server (default 24 hours)
 */
void getNtpTime() {
  static unsigned long nextTime = 0;
  // periodically sync clock with NTP server
  if(nextTime < millis()) {
    DBprintln("Syncing time");
    configTime(timezone*3600, dstOffset*3600, "pool.ntp.org", "time.nist.gov");
    nextTime += ntpSyncInterval * 1000; // sync interval in miliseconds
  }
}

/*
 *  update the LED display twice a second (so we can blink the colon)
 */
void displayTime() {
  static unsigned long nextTime = 0;
  // update display every 500 milliseconds
  if (nextTime < millis()) {
    static uint8_t colon = 0;
    int hh, mm, ss;

    time_t tt = time(nullptr); // get current time
    hh = hour(tt); // extract hours
    mm = minute(tt); // extract minutes
    ss = second(tt); // extract seconds

    colon ^= 0b01000000; // blink colon
    tm1637.showNumberDecEx((hh*100)+mm, colon, true); // update display

    nextTime += 500; // next update in 500 milliseconds
  }
}
