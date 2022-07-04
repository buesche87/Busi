/**************************************************************************/
/*
   Busi
*/
/**************************************************************************/
#include <WiFi.h>
#include <NTPClient.h>
#include <FastLED.h>
#include <WiFiUdp.h>
#include <Timezone.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "ESPmDNS.h"
#include "time.h"

/* Run Modes */
#define PROGRAM 0
#define CLOCK   1
#define TEST    2

/* Modes */
#define OFF   0
#define DOZE  1
#define WAKE  2
#define SLEEP 3
#define AP    4

/*  WS2812 LED Settings */
#define LED_PIN      16
#define NUM_LEDS     16
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB
CRGB leds[NUM_LEDS];

/* Defaults */
const char *configfile = "/config.json";
const char* timeserver = "pool.ntp.org";
int modus              = CLOCK;      // Start mode (CLOCK | PROGRAM)
int prevmode           = CLOCK; 
uint8_t currentState   = WAKE;       // Default LED mode
int failCount          = 0;          // WiFi FailCount
String currentColor    = "#000000";  // Color shown on Webinterface
int testMode;                        // Testmode
bool breakcycle        = false;      // Break led for-loop
bool breaktest         = false;      // Break testmode while-loop
int starttime;
int endtime;


/* Wifi Setting (config.json) */
String devname;
String ssid;
String pass;
char * ipaddress = new char[40]();
uint8_t wifiretry           = 10;

/* Doze (config.json) */
bool dozeactive             = true;
String dozetime;
uint8_t dozestartcolor;
uint8_t dozestopcolor;
float dozestartbrightness;
float dozestopbrightness;
float dozepulsespeed;
uint8_t hue;
float val;

/* Wake (config.json) */
bool wakeactive             = true;
String waketime;
uint8_t wakehue;
uint8_t wakebrightness;

/* Day (config.json) */
bool noonactive             = true;
String morningtime;
String noontime;
String afternoontime;

/* Sleep (config.json) */
bool sleepactive            = true;
String sleeptime;
uint8_t sleepyellowint      = 39;
uint8_t sleepyellowmin;
uint8_t sleepyellowmax;
uint8_t sleepyellowrange;
uint8_t sleepbrightness;
uint8_t sleepspeed;

/* Program animation */
uint8_t prghue;;

/*  Webserver */
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalClient = NULL;

/*  NTP Clinet */
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, timeserver);
unsigned long epochTime;

/*  Timezone Settings */
TimeChangeRule myDST = {"CDT", Second, Sun, Mar, 2, +120};   // Daylight time = UTC + 2 hours
TimeChangeRule mySTD = {"DST", First, Sun, Nov, 2, +60};     // Standard time = UTC + 1 hours
Timezone myTZ(myDST, mySTD);
TimeChangeRule *tcr;

/*
   -------------------
   Setup
   -------------------
*/

void setup(void) {

  /*  Console Debug */
  Serial.begin(115200);
  Serial.println();
  Serial.print("Booting...");
  Serial.println();

  /* Initialize LED */
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(100);

  /* Initialize SPIFFS */
  initSPIFFS();

  /* Load Config */
  load_config();

  /* Connect Wifi */
  if (wifi_connect()) {
    led_off();
    modus = CLOCK;
    timeClient.begin();
    getTime();
  }
  else {
    modus = PROGRAM;
    prevmode = modus;
    WiFi.softAP(devname.c_str(), NULL);
    IPAddress IP = WiFi.softAPIP();
    sprintf(ipaddress, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]);
    Serial.print("AP IP address: "); Serial.println(IP);
  }

  /* Start ElegantOTA */
  AsyncElegantOTA.begin(&server);

  /* Start Websocket */
  initWebSocket();

  /* Start Webserver */
  start_webserver();

  /* Sart mDNS */
  if (!MDNS.begin(devname.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  MDNS.addService("http", "tcp", 80);

  Serial.println("Bootup complete...");
  Serial.println();
}

/*
   -------------------
   Loop
   -------------------
*/

void loop(void) {

  /* Program Mode */
  if (modus == PROGRAM) {
    program();
  }

  else if (modus == TEST) {
    test();
  }

  /* Run Mode */
  else {
    run();
  }
}


/*
   -------------------
   Run Mode
   -------------------
*/

void run(void) {

  /* Get Time every 30 Seconds */
  EVERY_N_SECONDS(30) {
    if (WiFi.status() == WL_CONNECTED) {
      getTime();
    }
  }

  /* Notify Websocket Clients */
  EVERY_N_SECONDS(5) {
    notifyClients();
  }

  /* Convert Local Time */
  time_t utc = now();
  time_t local = myTZ.toLocal(utc, &tcr);
  epochTime = utc;
  int rightNow[] = { hour(local), minute(local) };
  int rn = big_time_a(rightNow);

  /* Serial Output */
  EVERY_N_SECONDS(5) {
    Serial.println();
    printDateTime(utc, "UTC");
    printDateTime(local, tcr -> abbrev);
    Serial.println();
    Serial.println((String)"rightNow = " + rn);
    Serial.println((String)"DozeTime: " + big_time(dozetime) + " (" + dozetime + ")");
    Serial.println((String)"Wake-Time: " + big_time(waketime) + " (" + waketime + ")");
    Serial.println((String)"Morning-Time: " + big_time(morningtime) + " (" + morningtime + ")");
    Serial.println((String)"Noon-Time: " + big_time(noontime) + " (" + noontime + ")");
    Serial.println((String)"Afternoon-Time: " + big_time(afternoontime) + " (" + afternoontime + ")");
    Serial.println((String)"Sleep-Time: " + big_time(sleeptime) + " (" + sleeptime + ")");
    Serial.print((String)"Current State: " + currentState);
    Serial.println();
  }

  /* Goto Doze-Time */
  if ((rn >= big_time(dozetime)) && (rn < big_time(waketime))) {
    if (dozeactive == true) {
      set_light_state(DOZE);
    }
    else if (sleepactive == true) {
      set_light_state(SLEEP);
    }
    else {
      set_light_state(OFF);
    }
  }

  /* Goto Wake-Time */
  if ((rn >= big_time(waketime)) && (rn < big_time(morningtime))) {
    if (wakeactive == true) {
      set_light_state(WAKE);
    }
    else {
      set_light_state(OFF);
    }
  }

  /* Goto Morning-Time */
  if ((rn >= big_time(morningtime)) && (rn < big_time(noontime))) {
    set_light_state(OFF);
  }

  /* Goto Noon-Time */
  if ((rn >= big_time(noontime)) && (rn < big_time(afternoontime))) {
    if (noonactive == true) {
      set_light_state(SLEEP);
    }
    else {
      set_light_state(OFF);
    }
  }

  /* Goto Afternoon-Time */
  if ((rn >= big_time(afternoontime)) && (rn < big_time(sleeptime))) {
    set_light_state(OFF);
  }

  /* Goto Sleep-Time */
  if (((rn >= big_time(sleeptime)) || (rn < big_time(dozetime)))) {
    if (sleepactive == true) {
      set_light_state(SLEEP);
    }
    else {
      set_light_state(OFF);
    }
  }
}

/*
   -------------------
   Program Mode
   -------------------
*/
void program(void) {

  led_program();

}


/*
   -------------------
   Test Mode
   -------------------
*/

void test(void) {

  if (testMode == DOZE) {
    breakcycle = false;
    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <= 20000) {
      if (breaktest == true) {
        breaktest = false;
        break;
      }
      led_doze();
      endtime = millis();
    }
    modus = prevmode;
  }
  
  else if (testMode == WAKE) {
    breakcycle = false;
    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <= 10000) {
      if (breaktest == true) {
        breaktest = false;
        break;
      }
      led_wake();
      endtime = millis();
    }
    modus = prevmode;
  }
  
  else if (testMode == SLEEP) {
    breakcycle = false;
    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <= 20000) {
      if (breaktest == true) {
        breaktest = false;
        break;
      }
      led_sleep();
      endtime = millis();
    }
    modus = prevmode;
  }
}

/* -- LED Mode Select -- */

void set_light_state(uint8_t state) {

  currentState = state;

  if (state == OFF)  {
    led_off();
  }
  if (state == DOZE) {
    led_doze();
  }
  if (state == WAKE) {
    led_wake();
  }
  if (state == SLEEP) {
    led_sleep();
  }
}
