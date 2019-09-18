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
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * For more information, please refer to [http://unlicense.org]
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

  DBprintln("\nSimplest ESP8266 Clock");

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
  uint8_t arrayOfZeros[] = {0, 0, 0, 0}; // technically we only need one zero element
  // update display every 500 milliseconds
  if (nextTime < millis()) {
    static uint8_t colon = 0;
    int hh, mm, ss;

    time_t tt = time(nullptr); // get current time
    hh = hour(tt); // extract hours
    mm = minute(tt); // extract minutes
    ss = second(tt); // extract seconds

    if(hh < 10) {
      colon ^= 0b10000000; // blink colon
      tm1637.setSegments(arrayOfZeros, 1, 0); // erase leading digit if before 10:00
      tm1637.showNumberDecEx((hh*100)+mm, colon, true, 3, 1); // update display; h:mm
    }
    else {
      colon ^= 0b01000000; // blink colon
      tm1637.showNumberDecEx((hh*100)+mm, colon, true); // update display; hh:mm
    }

    nextTime += 500; // next update in 500 milliseconds
  }
}
