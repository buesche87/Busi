/*
   -------------------
   Config
   -------------------
*/
void load_config() {

  /* Load Config */
  devname = Load_Json("devname");
  ssid = Load_Json("ssid");
  pass = Load_Json("pass");
  dozetime = Load_Json("dozetime");
  waketime = Load_Json("waketime");
  morningtime = Load_Json("morningtime");
  noontime = Load_Json("noontime");
  afternoontime = Load_Json("afternoontime");
  sleeptime = Load_Json("sleeptime");

  if (Load_Json("dozeactive") == "true") {
    dozeactive = true;
  } else {
    dozeactive = false;
  }
  dozestartcolor = Load_Json("dozestartcolor").toInt();
  dozestopcolor = Load_Json("dozestopcolor").toInt();
  dozestartbrightness = Load_Json("dozestartbrightness").toFloat();
  dozestopbrightness = Load_Json("dozestopbrightness").toFloat();
  dozepulsespeed = Load_Json("dozepulsespeed").toFloat();

  if (Load_Json("wakeactive") == "true") {
    wakeactive = true;
  } else {
    wakeactive = false;
  }
  wakebrightness = Load_Json("wakebrightness").toInt();
  wakehue = Load_Json("wakehue").toInt();

  if (Load_Json("sleepactive") == "true") {
    sleepactive = true;
  } else {
    sleepactive = false;
  }
  sleepyellowmin = Load_Json("sleepyellowmin").toInt();
  sleepyellowrange = Load_Json("sleepyellowrange").toInt();
  sleepbrightness = Load_Json("sleepbrightness").toInt();
  sleepspeed = Load_Json("sleepspeed").toInt();
  sleepofftimer = Load_Json("sleepofftimer").toInt();

  if (Load_Json("noonactive") == "true") {
    noonactive = true;
  } else {
    noonactive = false;
  }
}

void save_config() {

  Save_Json("devname", devname);
  Save_Json("ssid", ssid);
  Save_Json("pass", pass);
  Save_Json("dozetime", dozetime);
  Save_Json("waketime", waketime);
  Save_Json("morningtime", morningtime);
  Save_Json("noontime", noontime);
  Save_Json("afternoontime", afternoontime);
  Save_Json("sleeptime", sleeptime);

  Save_Json_Bool("dozeactive", dozeactive);
  Save_Json_Bool("wakeactive", wakeactive);
  Save_Json_Bool("noonactive", noonactive);
  Save_Json_Bool("sleepactive", sleepactive);

  Save_Json_Int("sleepyellowmin", sleepyellowmin);
  Save_Json_Int("sleepyellowrange", sleepyellowrange);
  Save_Json_Int("sleepbrightness", sleepbrightness);
  Save_Json_Int("sleepspeed", sleepspeed);
  Save_Json_Int("sleepofftimer", sleepofftimer);

  Save_Json_Int("dozestartcolor", dozestartcolor);
  Save_Json_Int("dozestopcolor", dozestopcolor);
  Save_Json_Float("dozestartbrightness", dozestartbrightness);
  Save_Json_Float("dozestopbrightness", dozestopbrightness);
  Save_Json_Float("dozepulsespeed", dozepulsespeed);

  Save_Json_Int("wakehue", wakehue);
  Save_Json_Int("wakebrightness", wakebrightness);

}
