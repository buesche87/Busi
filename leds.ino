/*
   -------------------
   LED Functions
   -------------------
*/

/* -- LED Off -- */

void led_off() {

  fill_solid(leds, NUM_LEDS, 0x000000);
  FastLED.setBrightness(0);
  FastLED.show();

  breakcycle = false;
  currentColor = "#000000";
  delay(100);
}

/* -- LED Doze -- */

void led_doze() {

  // Color
  float delta = (dozestopbrightness - dozestartbrightness) / 2.35040238;
  float dV = ((exp(sin((dozepulsespeed) / 10 * millis() / 2000.0 * PI)) - 0.36787944) * delta);

  val = dozestartbrightness + dV;
  hue = map(val, dozestartbrightness, dozestopbrightness, dozestartcolor, dozestopcolor);
  if ( (hue == -1) || (hue == 255) ) {
    hue = dozestartcolor;
  }

  CRGB rgb;
  hsv2rgb_spectrum(CHSV(hue, 255, 255), rgb);

  // Show
  fill_solid(leds, NUM_LEDS, rgb);
  FastLED.setBrightness(val);
  FastLED.show();

  // Send currentColor to Webinterface
  EVERY_N_MILLISECONDS(100) {
    send_color(rgb);
  }
}

/* -- LED Wake -- */

void led_wake() {

  // Color
  CRGB rgb;
  hsv2rgb_spectrum(CHSV(wakehue, 255, 255), rgb);

  // Show
  fill_solid(leds, NUM_LEDS, rgb);
  FastLED.setBrightness(wakebrightness);
  FastLED.show();

  breakcycle = false;

  // Send currentColor to Webinterface
  EVERY_N_MILLISECONDS(100) {
    send_color(rgb);
  }
}

/* -- LED Sleep -- */

void led_sleep() {

  // Color
  sleepyellowmax = sleepyellowmin + sleepyellowrange;
  if (sleepyellowmax >= 255) {
    sleepyellowmax = 255;
  }
  int randmax = random(sleepyellowmin, sleepyellowmax);
  int randmin = random(sleepyellowmin, sleepyellowmax - randmax);
  if (randmin >= randmax) {
    randmin = randmax;
  }

  // Animation+
  for (int i = sleepyellowint; i <= randmax; i++) {

    if (breakcycle == true) {
      break;
    }

    int r = 255;
    int g = i;
    int b = 0;
    sleepyellowint = i;

    CRGB rgb = CRGB(r, g, b);

    for (int i=5; i>0; i--) {
      leds[random(NUM_LEDS)] = rgb;
    }
    
    // Show
    FastLED.setBrightness(sleepbrightness);
    FastLED.show();

    // Send currentColor to Webinterface
    send_color(rgb);
    delay(random(500, 1000) / sleepspeed);
  }

  // Animation-
  for (int i = sleepyellowint; i >= randmin; i--) {

    if (breakcycle == true) {
      break;
    }

    int r = 255;
    int g = i;
    int b = 0;
    sleepyellowint = i;

    CRGB rgb = CRGB(r, g, b);

    for (int i=5; i>0; i--) {
      leds[random(NUM_LEDS)] = rgb;
    }

    // Show
    FastLED.setBrightness(sleepbrightness);
    FastLED.show();

    // Send currentColor to Webinterface
    send_color(rgb);
    delay(random(500, 1000) / sleepspeed);
  }

  breakcycle = false;
}

/* -- LED Program -- */

void led_program() {

  // Animation
  prghue = beatsin8(18, 0, 36);

  // Show
  fill_solid(leds, NUM_LEDS, CHSV(prghue, 255, 255));
  FastLED.setBrightness(100);
  FastLED.show();

  // Send currentColor to Webinterface
  EVERY_N_MILLISECONDS(100) {
    CRGB rgb;
    hsv2rgb_rainbow(CHSV(prghue, 255, 255), rgb); 
    send_color(rgb);
  }
}

/* -- LED Blink -- */

void led_blink() {

  FastLED.setBrightness(50);
  fill_solid(leds, NUM_LEDS, CRGB(255, 39, 0));
  FastLED.show();
  delay(300);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(700);
}
