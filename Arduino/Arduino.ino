#include "FastLED.h"

#define REDPIN 10
#define GREENPIN 11
#define BLUEPIN 9

CRGB leds[17];

char c = 'm';   //Color mode: mirror
char s = 'd';   //Speed: 'd' (off)

void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  analogWrite(REDPIN, 255);
  analogWrite(GREENPIN, 255);
  analogWrite(BLUEPIN, 255);
  
  FastLED.addLeds<NEOPIXEL, 2>(leds, 17);
  Serial.begin(9600);
}
  
void loop() {
  if (Serial.available()) {
    char header = Serial.read();

    delay(5);

    switch (header) {
      case 'f':
        parseStrip();
        break;

      case 'g':
        parseCase();
        break;

      /*case 't':
        parseTemp();
        break;*/

      case 'c':
        parseColor();
        break; 
    } 
  }
}

void parseStrip() {
  int p = Serial.read();
  p--;
  
  char color[3];
  Serial.readBytes(color, 3);

  switch (c) {
    case 'm':
      stripM(p, color);
      break;

    case 't':
      stripT(p, color);
      break;

    case 'b':
      stripB(p, color);
      break;
  }

  FastLED.show();
}

void stripM(int p, char *color) {
  int i1 = 8 - p;
  int i2 = 8 + p;
  
  leds[i1].r = color[0];
  leds[i1].g = color[1];
  leds[i1].b = color[2];

  leds[i2].r = color[0];
  leds[i2].g = color[1];
  leds[i2].b = color[2];
}

void stripT(int p, char *color) {
  int i1 = p * 2;
  
  leds[i1].r = color[0];
  leds[i1].g = color[1];
  leds[i1].b = color[2];

  if (i1 > 0) {
    int i2 = i1 - 1;
    
    leds[i2].r = color[0];
    leds[i2].g = color[1];
    leds[i2].b = color[2];
  }
}

void stripB(int p, char *color) {
  int i1 = 16 - (p * 2);

  leds[i1].r = color[0];
  leds[i1].g = color[1];
  leds[i1].b = color[2];

  if (i1 > 0) {
    int i2 = i1 - 1;
    
    leds[i2].r = color[0];
    leds[i2].g = color[1];
    leds[i2].b = color[2];
  }
}

void parseCase() {
  char color[3];
  Serial.readBytes(color, 3);

  analogWrite(REDPIN, 255 - color[0]);
  analogWrite(GREENPIN, 255 - color[1]);
  analogWrite(BLUEPIN, 255 - color[2]);
}

void parseColor() {
  c = Serial.read();
}
