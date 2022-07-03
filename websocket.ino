/*
   -------------------
   Websocket Functions
   -------------------
*/

/* -- Send JSON -- */

void notifyClients() {

  if (globalClient != NULL && globalClient->status() == WS_CONNECTED) {
    const uint8_t size = JSON_OBJECT_SIZE(10);
    StaticJsonDocument<size> json;
    json["devname"] = devname;
    json["epochTime"] = epochTime;
    json["currentColor"] = currentColor;

    char data[1000];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
  }
}

/* -- Receive JSON -- */

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

    StaticJsonDocument<2048> json;
    DeserializationError err = deserializeJson(json, data);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }

    /* Manual Mode */
    if (!json["manual"].isNull()) {
      uint64_t manualepoch = uint64_t(json["manual"].as<uint64_t>());

      time_t utc = (manualepoch / 1000);
      setTime(utc);
      time_t local = myTZ.toLocal(utc, &tcr);
      Serial.print("manualepoch: "); Serial.println(utc);
      modus = CLOCK;
    }

    /* Testmode */
    if (!json["mode"].isNull()) {
      const char *action = json["mode"];
      if (strcmp(action, "dozetest") == 0) {
        Serial.println("TEST-Mode: DOZE");
        if (modus == CLOCK) {
          breakcycle = true;
          prevmode = CLOCK;
        }
        modus = TEST;
        testMode = DOZE;
        delay(10);
      }
      if (strcmp(action, "waketest") == 0) {
        Serial.println("TEST-Mode: WAKE");
        if (modus == CLOCK) {
          breakcycle = true;
          prevmode = CLOCK;
        }
        modus = TEST;
        testMode = WAKE;
        delay(10);
      }
      if (strcmp(action, "sleeptest") == 0) {
        Serial.println("TEST-Mode: SLEEP");
        if (modus == CLOCK) {
          breakcycle = true;
          prevmode = CLOCK;
        }
        modus = TEST;
        testMode = SLEEP;
        delay(10);
      }
    }

    /* Configuration */
    if (!json["config"].isNull()) {
      const char *action = json["config"];
      if (strcmp(action, "save") == 0) {
        Serial.println(); Serial.print("Saving config"); Serial.println();
        save_config();
      }
      if (strcmp(action, "load") == 0) {
        Serial.println(); Serial.print("Loading config"); Serial.println();
        load_config();
      }
      if (strcmp(action, "reboot") == 0) {
        Serial.println(); Serial.print("Rebooting"); Serial.println();
        ESP.restart();
      }
    }

    /* Network Settings */
    if (!json["devname"].isNull()) {
      if (!strcmp("", json["devname"]) == 0) {
        devname = json["devname"].as<String>();
        Serial.print("devname: "); Serial.println(devname);
      }
    }
    if (!json["ssid"].isNull()) {
      if (!strcmp("", json["ssid"]) == 0) {
        ssid = json["ssid"].as<String>();
        Serial.print("ssid: "); Serial.println(ssid);
      }
    }
    if (!json["pass"].isNull()) {
      if (!strcmp("", json["pass"]) == 0) {
        pass = json["pass"].as<String>();
        Serial.print("pass: "); Serial.println(pass);
      }
    }

    /* Timer Settings */
    if (!json["dozetime"].isNull()) {
      dozetime = json["dozetime"].as<String>();
      Serial.print("dozetime: "); Serial.println(dozetime);
    }
    if (!json["waketime"].isNull()) {
      waketime = json["waketime"].as<String>();
      Serial.print("waketime: "); Serial.println(waketime);
    }
    if (!json["morningtime"].isNull()) {
      morningtime = json["morningtime"].as<String>();
      Serial.print("morningtime: "); Serial.println(morningtime);
    }
    if (!json["noontime"].isNull()) {
      noontime = json["noontime"].as<String>();
      Serial.print("noontime: "); Serial.println(noontime);
    }
    if (!json["afternoontime"].isNull()) {
      afternoontime = json["afternoontime"].as<String>();
      Serial.print("afternoontime: "); Serial.println(afternoontime);
    }
    if (!json["sleeptime"].isNull()) {
      sleeptime = json["sleeptime"].as<String>();
      Serial.print("sleeptime: "); Serial.println(sleeptime);
    }

    /* Active Settings */
    if (!json["dozeactive"].isNull()) {
      dozeactive = json["dozeactive"].as<bool>();
      Serial.print("dozeactive: "); Serial.println(dozeactive);
    }
    if (!json["wakeactive"].isNull()) {
      wakeactive = json["wakeactive"].as<bool>();
      Serial.print("wakeactive: "); Serial.println(wakeactive);
    }
    if (!json["noonactive"].isNull()) {
      noonactive = json["noonactive"].as<bool>();
      Serial.print("noonactive: "); Serial.println(noonactive);
    }
    if (!json["sleepactive"].isNull()) {
      sleepactive = json["sleepactive"].as<bool>();
      Serial.print("sleepactive: "); Serial.println(sleepactive);
    }

    /* Sleep Settings */
    if (!json["sleepyellowmin"].isNull()) {
      sleepyellowmin = int(json["sleepyellowmin"]);
      Serial.print("sleepyellowmin: "); Serial.println(sleepyellowmin);
    }
    if (!json["sleepyellowrange"].isNull()) {
      sleepyellowrange = int(json["sleepyellowrange"]);
      Serial.print("sleepyellowrange: "); Serial.println(sleepyellowrange);
    }
    if (!json["sleepbrightness"].isNull()) {
      sleepbrightness = int(json["sleepbrightness"]);
      Serial.print("sleepbrightness: "); Serial.println(sleepbrightness);
      FastLED.setBrightness(sleepbrightness);
    }
    if (!json["sleepspeed"].isNull()) {
      sleepspeed = int(json["sleepspeed"]);
      Serial.print("sleepspeed: "); Serial.println(sleepspeed);
    }

    /* Doze Settings */
    if (!json["dozestartcolor"].isNull()) {
      dozestartcolor = int(json["dozestartcolor"]);
      Serial.print("dozestartcolor: "); Serial.println(dozestartcolor);
    }
    if (!json["dozestopcolor"].isNull()) {
      dozestopcolor = int(json["dozestopcolor"]);
      Serial.print("dozestopcolor: "); Serial.println(dozestopcolor);
    }
    if (!json["dozestartbrightness"].isNull()) {
      dozestartbrightness = float(json["dozestartbrightness"]);
      Serial.print("dozestartbrightness: "); Serial.println(dozestartbrightness);
    }
    if (!json["dozestopbrightness"].isNull()) {
      dozestopbrightness = float(json["dozestopbrightness"]);
      Serial.print("dozestopbrightness: "); Serial.println(dozestopbrightness);
    }
    if (!json["dozepulsespeed"].isNull()) {
      dozepulsespeed = float(json["dozepulsespeed"]);
      Serial.print("dozepulsespeed: "); Serial.println(dozepulsespeed);
    }

    /* Wake Settings */
    if (!json["wakehue"].isNull()) {
      wakehue = int(json["wakehue"]);
      Serial.print("wakehue: "); Serial.println(wakehue);
    }
    if (!json["wakebrightness"].isNull()) {
      wakebrightness = int(json["wakebrightness"]);
      FastLED.setBrightness(wakebrightness);
      Serial.print("wakebrightness: "); Serial.println(wakebrightness);
    }    
  }
}

/* -- On Event -- */

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {

  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      globalClient = client;
      notifyClients();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      globalClient = NULL;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

/* -- Initialize Websocket -- */

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
