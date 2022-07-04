/*
   -------------------
   Webserver
   -------------------
*/

void start_webserver() {
  server.serveStatic("/", SPIFFS, "/");

  /* Root */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/devicemanager.html", "text/html");
  });

  /* Config */
  server.on("/conf", HTTP_GET, [](AsyncWebServerRequest * request) {
    StaticJsonDocument<512> cfgjson;
    cfgjson["dozetime"] = dozetime;
    cfgjson["waketime"] = waketime;
    cfgjson["morningtime"] = morningtime;
    cfgjson["noontime"] = noontime;
    cfgjson["afternoontime"] = afternoontime;
    cfgjson["sleeptime"] = sleeptime;
    cfgjson["wakebrightness"] = wakebrightness;
    cfgjson["sleepbrightness"] = sleepbrightness;
    cfgjson["sleepspeed"] = sleepspeed;
    cfgjson["dozeactive"] = dozeactive;
    cfgjson["wakeactive"] = wakeactive;
    cfgjson["noonactive"] = noonactive;
    cfgjson["sleepactive"] = sleepactive;
    cfgjson["wakehue"] = wakehue;
    cfgjson["sleepyellowmin"] = sleepyellowmin;
    cfgjson["sleepyellowrange"] = sleepyellowrange;
    cfgjson["dozestartcolor"] = dozestartcolor;
    cfgjson["dozestopcolor"] = dozestopcolor;
    cfgjson["dozestartbrightness"] = dozestartbrightness;
    cfgjson["dozestopbrightness"] = dozestopbrightness;
    cfgjson["dozepulsespeed"] = dozepulsespeed;
    cfgjson["ipaddress"] = ipaddress;

    String response;
    serializeJson(cfgjson, response);
    Serial.print("HTTP sending config: "); Serial.println(response);
    request->send(200, "application/json", response);
  });

  /* OK */
  server.on("/ok", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(devname).c_str());
  });

  server.begin();
}
