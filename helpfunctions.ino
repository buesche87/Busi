/*
   -------------------
   Help Functions
   -------------------
*/

/* -- Wifi Connect -- */

bool wifi_connect() {

  if (ssid == "" || pass == "") {
    Serial.println(); Serial.println("Undefined SSID or Passphrase");
    return false;
  }

  Serial.print("Connecting to: "); Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    led_blink();
    Serial.print(".");
    failCount++;
    if (failCount == wifiretry)
    {
      Serial.print("Failed connecting to Wifi after: "); Serial.print(failCount);
      Serial.print(" tries"); Serial.println();
      return false;
    }
  }

  Serial.println();
  Serial.print("WiFi connected: "); Serial.println(WiFi.localIP());
  IPAddress IP = WiFi.localIP();
  sprintf(ipaddress, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]);
  failCount = 0;
  return true;

}

/* -- Initialize SPIFFS -- */

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

/* -- Load Json -- */

String Load_Json(String VarName) {

  File configFile = SPIFFS.open(configfile, "r");
  if (!configFile) {
    Serial.println("- failed to open config file for writing");
    return "Null";
  }
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, configFile);
  configFile.close();
  return doc[VarName];
}

/* -- Save Json -- */

void Save_Json(String VarName, String VarValue) {

  File configFile = SPIFFS.open(configfile, "r");
  if (!configFile) {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, configFile);
  configFile.close();
  doc[VarName] = VarValue;
  configFile = SPIFFS.open(configfile, "w");
  serializeJson(doc, configFile);
  configFile.close();
}

void Save_Json_Bool(String VarName, bool VarValue) {

  File configFile = SPIFFS.open(configfile, "r");
  if (!configFile) {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, configFile);
  configFile.close();
  doc[VarName] = (bool)VarValue;
  configFile = SPIFFS.open(configfile, "w");
  serializeJson(doc, configFile);
  configFile.close();
}

void Save_Json_Int(String VarName, int VarValue) {

  File configFile = SPIFFS.open(configfile, "r");
  if (!configFile) {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, configFile);
  configFile.close();
  doc[VarName] = (int)VarValue;
  configFile = SPIFFS.open(configfile, "w");
  serializeJson(doc, configFile);
  configFile.close();
}

void Save_Json_Float(String VarName, float VarValue) {

  File configFile = SPIFFS.open(configfile, "r");
  if (!configFile) {
    Serial.println("- failed to open config file for writing");
    return;
  }
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, configFile);
  configFile.close();
  doc[VarName] = (float)VarValue;
  configFile = SPIFFS.open(configfile, "w");
  serializeJson(doc, configFile);
  configFile.close();
}

/* -- Get Time -- */

void getTime() {
  while (1) {
    timeClient.update();
    epochTime = timeClient.getEpochTime();
    if (epochTime == 0) {
      delay(2000);
      Serial.println("Retrying...");
    }
    else break;
  }
  Serial.println();
  Serial.print("Current Epoch-Time: "); Serial.println(epochTime);

  setTime(epochTime);
}

/* -- Print Time -- */

void printDateTime(time_t t, const char *tz)
{
  char buf[32];
  char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
  strcpy(m, monthShortStr(month(t)));
  sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
          hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
  Serial.println(buf);
}

/* -- Convert Time Array -- */

int big_time_a(int hoursmins[2]) {
  return (hoursmins[0] * 60) + hoursmins[1];
}

int big_time(String hoursmins) {

  String xval = getValue(hoursmins, ':', 0);
  String yval = getValue(hoursmins, ':', 1);

  int hours = xval.toInt();
  int mins = yval.toInt();

  return (hours * 60) + mins;
}

/* -- Convert Time String -- */

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void changemode(int newmode, int testmode) {

  Serial.println();
  Serial.print("Cur Mode: "); Serial.println(modus);
  Serial.print("Prv Mode: "); Serial.println(prevmode);

  if (modus == CLOCK) {
    breakcycle = true;
  }

  if (modus == TEST) {
    breakcycle = true;
    breaktest = true;
  }

  modus = newmode;
  testMode = testmode;
  Serial.print("New Mode: "); Serial.println(modus);
  notifyClients();
  delay(10);
}
