/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* version 5.0.1 Experimental                                                 *
* Started:   12-July-2021                                                    *
* Finalized:                                                                 *
* Developed by: B. Michael Silva                                             *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <FastLED.h>
#include <EEPROM.h>
#include "CustomColors.h"

#define BUTTON_ONE 7
#define BUTTON_TWO 8

const uint8_t EFFECT_REG = 100, BRIGHT_REG = 101,
   ALL_OFF = 0x00, ALL_WHITE = 0x01, ALL_COLOR = 0x02, BLINK_WHITE = 0x03,
   BLINK_COLOR = 0x04, CHASE_WHITE = 0x05, CHASE_COLOR = 0x06, FILL_WHITE = 0x07,
   FILL_COLOR = 0x08, PULSE_WHITE = 0x09, PULSE_COLOR = 0x0A, ALL_FLICKER = 0x0B,
   STARS_WHITE = 0x0C, STARS_COLOR = 0x0D, LIGHT_FLASH = 0x0E;

const boolean PRESSED = 0;

#define LED_PIN     11
#define NUM_LEDS    300
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

uint8_t effect;
uint8_t maxBright;

uint8_t ledState[NUM_LEDS];
uint16_t ledTimer[NUM_LEDS];

boolean buttonAState;
boolean buttonBState;
boolean lastButtonAState = !PRESSED;
boolean lastButtonBState = !PRESSED;

uint32_t lastDebounceTimeA = 0;
uint32_t lastDebounceTimeB = 0;
uint16_t debouncDelay = 1000;

void setup() {
   // Power up safety delay
   delay(3000);

   // Setup pins for IO
   pinMode(BUTTON_ONE, INPUT_PULLUP);
   pinMode(BUTTON_TWO, INPUT_PULLUP);
   pinMode(LED_BUILTIN, OUTPUT);

   // Make sure on-board LED is off
   digitalWrite(LED_BUILTIN, LOW);

   // Read EEPROM values
   effect = EEPROM.read(EFFECT_REG);
   maxBright = EEPROM.read(BRIGHT_REG);

   // Check if buttons have changed states during start-up
   if (digitalRead(BUTTON_ONE) == PRESSED) {
      if (effect < LIGHT_FLASH) {
         effect++; // If effect NOT last effect, increment up
      }
      else {
         effect = ALL_OFF; // If effect IS last effect, reset to 0
      }

      EEPROM.write(EFFECT_REG, effect); // Write new state to EEPROM

      // FLash lED briefly
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
   }

   if (digitalRead(BUTTON_TWO) == PRESSED) {
      if (maxBright == 255) {
         maxBright = 128; // If brightness 100%, set to 50%
      }
      else if (maxBright == 128) {
         maxBright =  32; // If brightness 50%, set to 25%
      }
      else {
         maxBright = 255; // If brightness 25%, set to 100%
      }

      EEPROM.write(BRIGHT_REG, maxBright); // Write new brightness to EEPROM

      // FLash lED longer
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
   }

   // Set up seed for random number generation
   uint16_t seed = analogRead(A0); // Set seed to analog pin 1's value
   randomSeed(seed);

   // Set up WS2812 LEDs
   FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFB0F0);
   FastLED.setBrightness(maxBright);

   effectSetup();
}

void effectSetup() {
   if (effect == ALL_OFF) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(FULLOFF);
      }
   }
   else if (effect == ALL_WHITE) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE180);
      }
   }
   else if (effect == ALL_COLOR) {
      setBlock(RED100, ORANGE100, GREEN100, BLUE100, PINK100);
   }
   else if (effect == BLINK_WHITE) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE180);
      }
      for (uint16_t j = 0; j < 300; j += 15 ) {
         leds[j] = CRGB(BLINKER100);
      }

      for (uint16_t x = 0; x < 20; x++) {
         ledState[x] = 1;
         ledTimer[x] = random(5000, 15000);
      }
   }
   else if (effect == BLINK_COLOR) {
      setBlock(RED100, ORANGE100, GREEN100, BLUE100, PINK100);

      for (uint16_t j = 0; j < 300; j += 15) {
         leds[j] = CRGB(BLINKER100);
      }

      for (uint16_t x = 0; x < 20; x++) {
         ledState[x] = 1;
         ledTimer[x] = random(5000, 15000);
      }
   }
   else if (effect == CHASE_WHITE) {
      setBlock(WHITE180, FULLOFF, WHITE45, WHITE90, WHITE135);

      ledState[0] = 2;
      delay(175);
   }
   else if (effect == CHASE_COLOR) {
      setBlock(RED100, ORANGE100, GREEN100, BLUE100, PINK100);

      ledState[0] = 2;
      delay(175);
   }
   else if (effect == FILL_WHITE) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(FULLOFF);
      }

      ledState[0] = 0;
      ledTimer[0] = 0;
   }
   else if (effect == FILL_COLOR) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(FULLOFF);
      }

      ledState[0] = 0;
      ledTimer[0] = 0;
   }
   else if (effect == PULSE_WHITE) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE180);
      }

      FastLED.setBrightness(0);

      ledTimer[0] = 0;
      ledState[1] = 1;

      delay(500);
   }
   else if (effect == PULSE_COLOR) {
      setBlock(RED100, ORANGE100, GREEN100, BLUE100, PINK100);

      FastLED.setBrightness(0);

      ledTimer[0] = 0;
      ledState[1] = 1;

      delay(500);
   }
   else if (effect == ALL_FLICKER) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE100);

         ledState[i] = 0;
         ledTimer[i] = random(100, 500);
      }


   }
   else if (effect == STARS_WHITE) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(FULLOFF);

         ledState[i] = 0;
         ledTimer[i] = random(100, 500);
      }
   }
   else if (effect == STARS_COLOR) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(FULLOFF);

         ledState[i] = 0;
         ledTimer[i] = random(100, 500);
      }
   }
   else if (effect == LIGHT_FLASH) {
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE36);
      }

      ledState[0] = random(0, 34);
      ledTimer[0] = random(500, 8500);
   }

   FastLED.show();
}

void loop() {
   // Update animated effects
   if (effect == BLINK_WHITE) {
      blinkyWhite();
   }
   else if (effect == BLINK_COLOR) {
      blinkyColor();
   }
   else if (effect == CHASE_WHITE) {
      chasingWhite();
   }
   else if (effect == CHASE_COLOR) {
      chasingColor();
   }
   else if (effect == FILL_WHITE) {
      fillWhite();
   }
   else if (effect == FILL_COLOR) {
      fillColor();
   }
   else if (effect == PULSE_WHITE || effect == PULSE_COLOR) {
      pulsingLight();
   }
   else if (effect == ALL_FLICKER) {
      flickeringWhite();
   }
   else if (effect == STARS_WHITE) {
      whiteStars();
   }
   else if (effect == STARS_COLOR) {
      colorStars();
   }
   else if (effect == LIGHT_FLASH) {
      lightFlash();
   }

   // If not pulsing white or colors, update brightness
   if (effect != PULSE_WHITE && effect != PULSE_COLOR) {
      FastLED.setBrightness(maxBright);
      if (effect == ALL_WHITE || effect == ALL_COLOR) {
         FastLED.show();
      }
   }

   checkButtonA();
   checkButtonB();
}

void checkButtonA() {
   boolean readButtonA = digitalRead(BUTTON_ONE); // Get button state

   if (readButtonA != lastButtonAState) {
      lastDebounceTimeA = millis();
   }

   if ((millis() - lastDebounceTimeA) > debouncDelay) {
      if (readButtonA != buttonAState) {
         buttonAState = readButtonA;

         if (buttonAState == PRESSED) {
            if (effect < LIGHT_FLASH) {
               effect++;
            }
            else {
               effect = ALL_OFF;
            }

            EEPROM.write(EFFECT_REG,effect);

            effectSetup();
         }
      }
   }

   lastButtonAState = readButtonA;

   // FLash lED briefly
   digitalWrite(LED_BUILTIN, HIGH);
   delay(50);
   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
   digitalWrite(LED_BUILTIN, HIGH);
   delay(50);
   digitalWrite(LED_BUILTIN, LOW);
   delay(50);
}

void checkButtonB() {
   boolean readButtonB = digitalRead(BUTTON_TWO);

   if (readButtonB != lastButtonBState) {
      lastDebounceTimeB = millis();
   }

   if ((millis() - lastDebounceTimeB) > debouncDelay) {
      if (readButtonB != buttonBState) {
         buttonBState = readButtonB;

         if (buttonBState == PRESSED) {
            if (maxBright == 128) {
               maxBright = 32;
            }
            else if (maxBright == 32) {
               maxBright = 255;
            }
            else {
               maxBright = 128;
            }

            EEPROM.write(BRIGHT_REG, maxBright);
         }
      }
   }

   lastButtonBState = readButtonB;

   // FLash lED longer
   digitalWrite(LED_BUILTIN, HIGH);
   delay(250);
   digitalWrite(LED_BUILTIN, LOW);
   delay(250);
}

void setBlock(uint32_t color0, uint32_t color1, uint32_t color2, uint32_t color3, uint32_t color4) {
   for (uint16_t i = 0; i < 300; i += 5) {
      leds[i + 0] = CRGB(color0);
      leds[i + 1] = CRGB(color1);
      leds[i + 2] = CRGB(color2);
      leds[i + 3] = CRGB(color3);
      leds[i + 4] = CRGB(color4);
   }
}

void blinkyWhite() {
   for (uint8_t x = 0; x < 300; x += 15) {
      uint16_t endOfSegment = x + 15;

      /* On ----------------------------------------------------------------- */
      if (ledState[x] == 1) {
         if (ledTimer[x] == 0) {
            // When timer hits 0
            ledTimer[x] = random(150, 750); // Generate new timer
            ledState[x] = 0; // Set state to off
         }
         else {
            ledTimer[x]--; // Decrement timer
         }

         // Near end of time alloted, fade to off
         if (ledTimer[x] > 40) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE180);
            }

            leds[x] = CRGB(BLINKER100);
         }
         else if (ledTimer[x] < 41 && ledTimer[x] > 30) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE144);
            }

            leds[x] = CRGB(BLINKER80);
         }
         else if (ledTimer[x] < 31 && ledTimer[x] > 20) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE108);
            }

            leds[x] = CRGB(BLINKER60);
         }
         else if (ledTimer[x] < 21 && ledTimer[x] > 10) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE72);
            }

            leds[x] = CRGB(BLINKER40);
         }
         else if (ledTimer[x] < 11 && ledTimer[x] > 0) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE36);
            }

            leds[x] = CRGB(BLINKER20);
         }
      }
      /* On ----------------------------------------------------------------- */
      else {
         if (ledTimer[x] == 0) {
            // When timer hits 0
            ledTimer[x] = random(75, 500); // Generate new timer
            ledState[x] = 1; // Set state to on
         }
         else {
            ledTimer[x]--; // Decrement timer
         }

         // Near end of time allotted, fade to on
         if (ledTimer[x] > 20) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(FULLOFF);
            }
         }
         else if (ledTimer[x] < 21 && ledTimer[x] > 15) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE36);
            }

            leds[x] = CRGB(BLINKER20);
         }
         else if (ledTimer[x] < 16 && ledTimer[x] > 10) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE72);
            }

            leds[x] = CRGB(BLINKER40);
         }
         else if (ledTimer[x] < 11 && ledTimer[x] >  5) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE108);
            }

            leds[x] = CRGB(BLINKER60);
         }
         else if (ledTimer[x] <  6 && ledTimer[x] >  0) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(WHITE144);
            }

            leds[x] = CRGB(BLINKER80);
         }
      }
   }

   FastLED.show();
}

void blinkyColor() {
   for (uint8_t x = 0; x < 20; x++) {
      uint16_t endOfSegment = x + 15;

      /* On ----------------------------------------------------------------- */
      if (ledState[x] == 1) {
         if (ledTimer[x] == 0) {
            // When timer hits 0
            ledTimer[x] = random(150, 750); // Generate new timer
            ledState[x] = 0; // Set state to off
         }
         else {
            ledTimer[x]--;  // Decrement timer
         }

         // Near end of time alloted, fade to off
         if (ledTimer[x] > 40) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED100);
               leds[i +  1] = CRGB(ORANGE100);
               leds[i +  2] = CRGB(GREEN100);
               leds[i +  3] = CRGB(BLUE100);
               leds[i +  4] = CRGB(PINK100);
            }

            leds[x] = CRGB(BLINKER100);
         }
         else if (ledTimer[x] < 41 && ledTimer[x] > 30) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED80);
               leds[i +  1] = CRGB(ORANGE80);
               leds[i +  2] = CRGB(GREEN80);
               leds[i +  3] = CRGB(BLUE80);
               leds[i +  4] = CRGB(PINK80);
            }

            leds[x] = CRGB(BLINKER80);
         }
         else if (ledTimer[x] < 31 && ledTimer[x] > 20) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED60);
               leds[i +  1] = CRGB(ORANGE60);
               leds[i +  2] = CRGB(GREEN60);
               leds[i +  3] = CRGB(BLUE60);
               leds[i +  4] = CRGB(PINK60);
            }

            leds[x] = CRGB(BLINKER60);
         }
         else if (ledTimer[x] < 21 && ledTimer[x] > 10) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED40);
               leds[i +  1] = CRGB(ORANGE40);
               leds[i +  2] = CRGB(GREEN40);
               leds[i +  3] = CRGB(BLUE40);
               leds[i +  4] = CRGB(PINK40);
            }

            leds[x] = CRGB(BLINKER40);
         }
         else if (ledTimer[x] < 11 && ledTimer[x] >  0) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED20);
               leds[i +  1] = CRGB(ORANGE20);
               leds[i +  2] = CRGB(GREEN20);
               leds[i +  3] = CRGB(BLUE20);
               leds[i +  4] = CRGB(PINK20);
            }

            leds[x] = CRGB(BLINKER20);
         }
      }
      /* Off ---------------------------------------------------------------- */
      else {
         if (ledTimer[x] == 0) {
            // When timer hits 0
            ledTimer[x] = random(50, 500); // Generate new timer
            ledState[x] = 0; // Set state to on
         }
         else {
            ledTimer[x]--; // Decrement timer
         }

         // Near end of time alloted, fade to on
         if (ledTimer[x] > 20) {
            for (uint16_t i = x; i < endOfSegment; i++) {
               leds[i] = CRGB(FULLOFF);
            }
         }
         else if (ledTimer[x] < 21 && ledTimer[x] > 15) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED20);
               leds[i +  1] = CRGB(ORANGE20);
               leds[i +  2] = CRGB(GREEN20);
               leds[i +  3] = CRGB(BLUE20);
               leds[i +  4] = CRGB(PINK20);
            }

            leds[x] = CRGB(BLINKER20);
         }
         else if (ledTimer[x] < 16 && ledTimer[x] > 10) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED40);
               leds[i +  1] = CRGB(ORANGE40);
               leds[i +  2] = CRGB(GREEN40);
               leds[i +  3] = CRGB(BLUE40);
               leds[i +  4] = CRGB(PINK40);
            }

            leds[x] = CRGB(BLINKER40);
         }
         else if (ledTimer[x] < 11 && ledTimer[x] >  5) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED60);
               leds[i +  1] = CRGB(ORANGE60);
               leds[i +  2] = CRGB(GREEN60);
               leds[i +  3] = CRGB(BLUE60);
               leds[i +  4] = CRGB(PINK60);
            }

            leds[x] = CRGB(BLINKER60);
         }
         else if (ledTimer[x] <  6 && ledTimer[x] >  0) {
            for (uint16_t i = x; i < endOfSegment; i += 5) {
               leds[i +  0] = CRGB(RED80);
               leds[i +  1] = CRGB(ORANGE80);
               leds[i +  2] = CRGB(GREEN80);
               leds[i +  3] = CRGB(BLUE80);
               leds[i +  4] = CRGB(PINK80);
            }

            leds[x] = CRGB(BLINKER80);
         }
      }
   }

   FastLED.show();
}

void chasingWhite() {
   if (ledState[0] == 1) /* 0 degrees */ {
      setBlock(WHITE180, FULLOFF, WHITE45, WHITE90, WHITE135);
   }
   else if (ledState[0] == 2) /* 72 degrees */ {
      setBlock(WHITE135, WHITE180, FULLOFF, WHITE45, WHITE90);
   }
   else if (ledState[0] == 3) /* 144 degrees */ {
      setBlock(WHITE90, WHITE135, WHITE180, FULLOFF, WHITE45);
   }
   else if (ledState[0] == 4) /* 216 degrees */ {
      setBlock(WHITE45, WHITE90, WHITE135, WHITE180, FULLOFF);
   }
   else if (ledState[0] == 5) /* 288 degrees */ {
      setBlock(FULLOFF, WHITE45, WHITE90, WHITE135, WHITE180);
   }

   // Increment phase degree
   if (ledState[0] == 5) {
      ledState[0] = 1;
   }
   else {
      ledState[0]++;
   }

   FastLED.show();
   delay(175);
}

void chasingColor() {
   if (ledState[0] == 1) /* 0 degrees */ {
      setBlock(RED100, ORANGE100, GREEN100, BLUE100, PINK100);
   }
   else if (ledState[0] == 2) /* 72 degrees */ {
      setBlock(PINK100, RED100, ORANGE100, GREEN100, BLUE100);
   }
   else if (ledState[0] == 3) /* 144 degrees */ {
      setBlock(BLUE100, PINK100, RED100, ORANGE100, GREEN100);
   }
   else if (ledState[0] == 4) /* 216 degrees */ {
      setBlock(GREEN100, BLUE100, PINK100, RED100, ORANGE100);
   }
   else if (ledState[0] == 5) /* 288 degrees */ {
      setBlock(ORANGE100, GREEN100, BLUE100, PINK100, RED100);
   }

   // Increment phase degree
   if (ledState[0] == 5) {
      ledState[0] = 1;
   }
   else {
      ledState[0]++;
   }

   FastLED.show();
   delay(175);
}

void fillWhite() {
   if (ledState[0] == 0) {
      leds[ledTimer[0]] = CRGB(WHITE180);

      if (ledTimer[0] < 300) {
         ledTimer[0]++;
         delay(12);
      }
      else {
         ledTimer[0] = 0;
         ledState[0] = 1;
         delay(50);
      }
   }
   else {
      leds[ledTimer[0]] = CRGB(FULLOFF);

      if (ledTimer[0] < 300) {
         ledTimer[0]++;
         delay(12);
      }
      else {
         ledTimer[0] = 0;
         ledState[0] = 0;
         delay(250);
      }
   }

   FastLED.show();
}

void fillColor() {
   if (ledState[0] == 0) {
      if (ledState[1] == 0) {
         leds[ledTimer[0]] = CRGB(RED100);
      }
      else if (ledState[1] == 1) {
         leds[ledTimer[0]] = CRGB(ORANGE100);
      }
      else if (ledState[1] == 2) {
         leds[ledTimer[0]] = CRGB(GREEN100);
      }
      else if (ledState[1] == 3) {
         leds[ledTimer[0]] = CRGB(BLUE100);
      }
      else if (ledState[1] == 4) {
         leds[ledTimer[0]] = CRGB(PINK100);
      }

      if (ledState[1] == 4) {
         ledState[1] = 0;
      }
      else {
         ledState[1]++;
      }

      if (ledTimer[0] < 300) {
         ledTimer[0]++;
         delay(12);
      }
      else {
         ledTimer[0] = 0;
         ledState[0] = 1;
         delay(50);
      }
   }
   else {
      leds[ledTimer[0]] = CRGB(FULLOFF);

      if (ledTimer[0] < 300) {
         ledTimer[0]++;
         delay(25);
      }
      else {
         ledTimer[0] = 0;
         ledState[0] = 0;
         delay(250);
      }
   }

   FastLED.show();
}

void pulsingLight() {
   if (ledState[1] == 0) {
      ledState[0]--;
   }
   else {
      ledState[0]++;
   }

   FastLED.setBrightness(ledState[0]);
   FastLED.show();

   if (maxBright == 255) {
      delay(5);

      if (ledState[1] == 0 && ledState[0] == 0) {
         ledState[1] = 1;
         delay(1250);
      }

      else if (ledState[1] == 1 && ledState[0] == maxBright) {
         ledState[1] = 0;
         delay(500);
      }
   }
   if (maxBright == 128) {
      delay(10);

      if (ledState[1] == 0 && ledState[0] == 0) {
         ledState[1] = 1;
         delay(1250);
      }

      else if (ledState[1] == 1 && ledState[0] == maxBright) {
         ledState[1] = 0;
         delay(500);
      }
   }
   else if (maxBright == 32) {
      delay(40);

      if (ledState[1] == 0 && ledState[0] == 0) {
         ledState[1] = 1;
         delay(1250);
      }

      else if (ledState[1] == 1 && ledState[0] == maxBright) {
         ledState[1] = 0;
         delay(500);
      }
   }
}

void flickeringWhite() {
   for (uint16_t i = 0; i < 300; i++) {
      /* Base color --------------------------------------------------------- */
      if (ledState[i] == 0) {
         // When timer hits 0
         if (ledTimer[i] == 0) {
            ledTimer[i] = 26; // Set timer
            ledState[i] = random(1, 12); // Pick new state
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         leds[i] = CRGB(WHITE100);
      }
      /* Fade bright -------------------------------------------------------- */
      else if (ledState[i] >= 1 && ledState[i] <=  4) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 500);  // Generate new timer
            ledState[i] = 0; // Revert to base color
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(BRIGHT20);
         }
         else if (ledTimer[i] == 21) {
            leds[i] = CRGB(BRIGHT40);
         }
         else if (ledTimer[i] == 19) {
            leds[i] = CRGB(BRIGHT60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(BRIGHT80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(BRIGHT100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(BRIGHT80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(BRIGHT60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(BRIGHT40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(BRIGHT20);
         }
      }
      /* Flicker dim -------------------------------------------------------- */
      else if (ledState[i] == 5) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 500); // Generate new timer
            ledState[i] = 0; // Revert to base color
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(FLAME20);
         }
         else if (ledTimer[i] == 21) {
            leds[i] = CRGB(FLAME40);
         }
         else if (ledTimer[i] == 19) {
            leds[i] = CRGB(FLAME60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(FLAME80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(FLAME100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(FLAME80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(FLAME60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(FLAME40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(FLAME20);
         }
      }
      /* Flicker fast ------------------------------------------------------- */
      else if (ledState[i] == 6) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 500); // Generate new timer
            ledState[i] = 0; // Revert to base color
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 21){
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 15){
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 11){
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  4){
            leds[i] = CRGB(WHITE80);
         }
      }
      /* FLicker ------------------------------------------------------------ */
      else if (ledState[i] == 7) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 500); // Generate new timer
            ledState[i] = 0; // Revert to base color
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 23) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 18) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] ==  9) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] ==  7) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(WHITE80);
         }
      }
      /* Fade off ----------------------------------------------------------- */
      else if (ledState[i] >= 8 && ledState[i] <= 12) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 500); // Generate new timer
            ledState[i] = 0; // Revert to base color
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 21) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 19) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(FULLOFF);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(WHITE80);
         }
      }
   }

   FastLED.show();
}

void whiteStars() {
   for (uint16_t i = 0; i < 300; i++) {
      /* Off ---------------------------------------------------------------- */
      if (ledState[i] == 0 || ledState[i] == 1) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = 87; // Set timer
            ledState[i] = random(1, 22); // Pick new state
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         leds[i] = CRGB(  0,   0,   0); // off
      }
      /* Flash bright ------------------------------------------------------- */
      else if (ledState[i] >=  2 && ledState[i] <=  7) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 25) {
            leds[i] = CRGB(WHITE36);
         }
         else if (ledTimer[i] == 23) {
            leds[i] = CRGB(WHITE72);
         }
         else if (ledTimer[i] == 21) {
            leds[i] = CRGB(WHITE108);
         }
         else if (ledTimer[i] == 19) {
            leds[i] = CRGB(WHITE144);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(WHITE180);
         }
         else if (ledTimer[i] == 10) {
            leds[i] = CRGB(WHITE144);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE108);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE72);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(WHITE36);
         }
      }
      /* Flicker ------------------------------------------------------------ */
      else if (ledState[i] ==  8 || ledState[i] ==  9) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* Flash -------------------------------------------------------------- */
      else if (ledState[i] >= 10 && ledState[i] <= 19) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 56) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 52) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 48) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 44) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* Hold --------------------------------------------------------------- */
      else if (ledState[i] >= 20 && ledState[i] <= 22) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(750, 3000); // Generate new timer
            ledState[i] = random(23, 24); // Pick out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 20) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE100);
         }
      }
      /* Fade out ----------------------------------------------------------- */
      else if (ledState[i] == 23) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* Flicker out -------------------------------------------------------- */
      else if (ledState[i] == 24) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 59) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 57) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 52) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 50) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 43) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 41) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 39) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
   }

   FastLED.show();
}

void colorStars() {
   for (uint16_t i = 0; i < 300; i++) {
      /* Off ---------------------------------------------------------------- */
      if (ledState[i] >= 0 && ledState[i] <= 2) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = 87; // Set timer
            ledState[i] = random(1, 34); // Pick new state
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         leds[i] = CRGB(  0,   0,   0); // off
      }
      /* Flash red ---------------------------------------------------------- */
      else if (ledState[i] >=  3 && ledState[i] <=  6) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 41) {
            leds[i] = CRGB(RED20);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(RED40);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] == 29) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 25) {
            leds[i] = CRGB(RED100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(RED40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(RED20);
         }
      }
      /* Flash orange ------------------------------------------------------- */
      else if (ledState[i] >=  7 && ledState[i] <=  9) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 41) {
            leds[i] = CRGB(ORANGE20);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(ORANGE40);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] == 29) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 25) {
            leds[i] = CRGB(ORANGE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(ORANGE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(ORANGE20);
         }
      }
      /* Flash green -------------------------------------------------------- */
      else if (ledState[i] >= 10 && ledState[i] <= 12) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 41) {
            leds[i] = CRGB(GREEN20);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(GREEN40);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] == 29) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 25) {
            leds[i] = CRGB(GREEN100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(GREEN40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(GREEN20);
         }
      }
      /* Flash blue --------------------------------------------------------- */
      else if (ledState[i] >= 13 && ledState[i] <= 15) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 41) {
            leds[i] = CRGB(BLUE20);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(BLUE40);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] == 29) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(BLUE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(BLUE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(BLUE20);
         }
      }
      /* Flash pink --------------------------------------------------------- */
      else if (ledState[i] >= 16 && ledState[i] <= 19) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 41) {
            leds[i] = CRGB(PINK20);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(PINK40);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] == 29) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 25) {
            leds[i] = CRGB(PINK100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(PINK40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(PINK20);
         }
      }
      /* Flicker white ------------------------------------------------------ */
      else if (ledState[i] == 20) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* Flicker red -------------------------------------------------------- */
      else if (ledState[i] == 21) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(RED20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(RED40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(RED100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(RED100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(RED100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(RED80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(RED60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(RED40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(RED20);
         }
      }
      /* Flicker orange ----------------------------------------------------- */
      else if (ledState[i] == 22) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(ORANGE20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(ORANGE40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(ORANGE100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(ORANGE100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(ORANGE100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(ORANGE80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(ORANGE60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(ORANGE40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(ORANGE20);
         }
      }
      /* Flicker green ------------------------------------------------------ */
      else if (ledState[i] == 23) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(GREEN20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(GREEN40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(GREEN100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(GREEN100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(GREEN100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(GREEN80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(GREEN60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(GREEN40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(GREEN20);
         }
      }
      /* Flicker blue ------------------------------------------------------- */
      else if (ledState[i] == 24) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(BLUE20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(BLUE40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(BLUE100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(BLUE100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(BLUE100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(BLUE80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(BLUE60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(BLUE40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(BLUE20);
         }
      }
      /* Flicker pink ------------------------------------------------------- */
      else if (ledState[i] == 25) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 53) {
            leds[i] = CRGB(PINK20);
         }
         else if (ledTimer[i] == 51) {
            leds[i] = CRGB(PINK40);
         }
         else if (ledTimer[i] == 49) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] == 47) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 45) {
            leds[i] = CRGB(PINK100);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 38) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] == 33) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 31) {
            leds[i] = CRGB(PINK100);
         }
         else if (ledTimer[i] == 24) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 22) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] == 17) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] == 15) {
            leds[i] = CRGB(PINK100);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(PINK80);
         }
         else if (ledTimer[i] ==  6) {
            leds[i] = CRGB(PINK60);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(PINK40);
         }
         else if (ledTimer[i] ==  2) {
            leds[i] = CRGB(PINK20);
         }
      }
      /* Flash white -------------------------------------------------------- */
      else if (ledState[i] >= 26 && ledState[i] <= 31) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 56) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 52) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 48) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 44) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 40) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* Hold white --------------------------------------------------------- */
      else if (ledState[i] >= 32 && ledState[i] <= 34) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(750, 3000); // Generate new timer
            ledState[i] = random(35, 36); // Pick out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 20) {
            leds[i] = CRGB(WHITE20);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE100);
         }
      }
      /* White fade out ----------------------------------------------------- */
      else if (ledState[i] == 35) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
      /* White flicker out -------------------------------------------------- */
      else if (ledState[i] == 36) {
         if (ledTimer[i] == 0) {
            // When timer hits 0
            ledTimer[i] = random(100, 1000); // Generate new timer
            ledState[i] = 0; // Go out
         }
         else {
            ledTimer[i]--; // Decrement timer
         }

         if (ledTimer[i] == 59) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 57) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 52) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 50) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 43) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 41) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] == 39) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 37) {
            leds[i] = CRGB(WHITE100);
         }
         else if (ledTimer[i] == 16) {
            leds[i] = CRGB(WHITE80);
         }
         else if (ledTimer[i] == 12) {
            leds[i] = CRGB(WHITE60);
         }
         else if (ledTimer[i] ==  8) {
            leds[i] = CRGB(WHITE40);
         }
         else if (ledTimer[i] ==  4) {
            leds[i] = CRGB(WHITE20);
         }
      }
   }

   FastLED.show();
}

void lightFlash() {
   /* Candlelight ----------------------------------------------------------- */
   if (ledState[0] == 0) {
      if (ledTimer[0] == 0) {
         // When timer hits 0
         ledTimer[0] = 20; // Set timer
         ledState[0] = 1; // Flash
      }
      else {
         ledTimer[0]--; // Decrement timer
      }
      for (uint16_t i = 0; i < 300; i++) {
         leds[i] = CRGB(WHITE36);
      }
   }
   /* Lightning flash ------------------------------------------------------- */
   else {
      if (ledTimer[0] == 0) {
         // When timer hits 0
         ledTimer[0] = random(500, 8500); // Generate new timer
         ledState[0] = 0; // Revert to base color
      }
      else {
         ledTimer[0]--; // Decrement timer
      }

      if (ledTimer[0] == 19) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH60);
         }
      }
      else if (ledTimer[0] == 17) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH100);
         }
      }
      else if (ledTimer[0] ==  8) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH80);
         }
      }
      else if (ledTimer[0] == 6) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH60);
         }
      }
      else if (ledTimer[0] ==  4) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH40);
         }
      }
      else if (ledTimer[0] ==  2) {
         for (uint16_t i = 0; i < 300; i++) {
            leds[i] = CRGB(FLASH20);
         }
      }
   }

   FastLED.show();
}
