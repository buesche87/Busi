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
uint8_t modus          = CLOCK;      // Start mode (CLOCK | PROGRAM)
uint8_t prevmode       = CLOCK;      // Previous mode
uint8_t testMode       = OFF;        // Testmode
uint8_t currentState   = WAKE;       // Default LED mode
uint8_t failCount      = 0;          // WiFi FailCount
String currentColor    = "#000000";  // Color shown on Webinterface
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
uint8_t sleepofftimer;

/* Program animation */
uint8_t prghue;

/* time */
unsigned long epochTime;

/*  Webserver */
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalClient = NULL;

/*  NTP Clinet */
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, timeserver);

/*  Timezone Settings */
TimeChangeRule myDST = {"CDT", Last, Sun, Mar, 2, +120};   // Daylight time = UTC + 2 hours
TimeChangeRule mySTD = {"DST", Last, Sun, Oct, 2, +60};    // Standard time = UTC + 1 hours
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
    prevmode = CLOCK;
    timeClient.begin();
    getTime();
  }
  else {
    modus = PROGRAM;
    prevmode = PROGRAM;
    WiFi.mode(WIFI_AP);
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

  /* Test Mode */
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
    notifyClients();
  }

  /* Goto Doze-Time */
  if ( (rn >= big_time(dozetime)) && (rn < big_time(waketime)) && dozeactive == true ) {
    set_light_state(DOZE);
  }
  
  /* Goto Wake-Time */
  else if ( (rn >= big_time(waketime)) && (rn < big_time(morningtime)) && wakeactive == true ) {
    set_light_state(WAKE);
  }
  
  /* Goto Noon-Time */
  else if ( (rn >= big_time(noontime)) && (rn < big_time(afternoontime)) && noonactive == true ) {
    if (sleepofftimer > 0 && (rn > big_time(noontime) + sleepofftimer) ) {
      set_light_state(OFF);
    }
    else {
      set_light_state(SLEEP);
    }
  }

  /* Goto Sleep-Time */
  else if ( ((rn >= big_time(sleeptime)) || (rn < big_time(dozetime))) && sleepactive == true ) {

    if (sleepofftimer > 0) {
      EVERY_N_SECONDS(5) {
        
        /* Calculate offtime */
        int offtime;
        bool midnight;

        /* After midnight */
        if (big_time(sleeptime) + sleepofftimer > 1440) {
          offtime = big_time(sleeptime) + sleepofftimer - 1440;
          midnight = true;
        }
        
        /* Before midnight */
        else {
          offtime = big_time(sleeptime) + sleepofftimer;
          midnight = false;
        }

        Serial.print("offtime: "); Serial.println(offtime);

        if (rn < offtime && midnight == false) {
          set_light_state(SLEEP);
        }
        else if ( rn < big_time(sleeptime) && rn < offtime && midnight == true ) {
          set_light_state(SLEEP);
        }
        else {
          set_light_state(OFF);
        }
      }
    }
    else {
      set_light_state(SLEEP);
    }
  }

  /* Else OFF */
  else {
    EVERY_N_SECONDS(5) {
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
  
  /* Doze Test */
  if (testMode == DOZE) {
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

  /* Wake Test */
  else if (testMode == WAKE) {
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

  /* Sleep Test */
  else if (testMode == SLEEP) {
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
