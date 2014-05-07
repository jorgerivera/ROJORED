///////////////////////////////////////////////////
///// 
///// ROJORED LED dance
/////     by: Jorge Rivera
/////     date: 2014 05 06
/////
/////     This is a snippet that creates a random
/////     LED dance in the 4x4 NeoPixel panel to
/////     be used in the ROJORED game.
/////     
/////     This will be used for random displays
/////     during the idle and the transitional
/////     states.
/////
///////////////////////////////////////////////////

#include <Adafruit_NeoPixel.h>

#define PIN 16
#define CELLS 16
#define MAX_LEVEL 25
#define NUM_INPUTS 4

Adafruit_NeoPixel panel = Adafruit_NeoPixel(CELLS, PIN, NEO_GRB + NEO_KHZ800);

const uint32_t color[8] = { 0x000000, // NONE
  0x800000, // RED
  0x802000, // ORANGE
  0x606000, // YELLOW
  0x008000, // GREEN
  0x000080, // BLUE
  0x600060, // VIOLET
  0x404040  // WHITE
};
const uint8_t RED = 1;


void setup() {
  panel.begin();
  panel.show();
  randomSeed(analogRead(A8));
}

unsigned long start, finish, nextUpdate, interval, now;

void loop() {
  danceLEDs(5, 5000);
  delay(8000);
  danceLEDs(100, 200);
  delay(8000);
  
}

void danceLEDs(uint8_t cycles, uint32_t maxInterval) {
  uint32_t nextUpdate = millis() + maxInterval;
  while (cycles > 0) {
    now = millis();
    if (now>nextUpdate) {
      // use a random-ish interval to give it a little variation
      nextUpdate = now + random(maxInterval>>2, maxInterval);
      cycles--;
      randomCombination();
      Serial.print("interval [");
      Serial.print(nextUpdate-now);
      Serial.print("ms] randomComb duration [");
      Serial.print(finish-start);
      Serial.println("us]");
    }
  }
  while (millis()<nextUpdate);
  resetPanel();
  Serial.print("resetPanel randomComb duration [");
  Serial.print(finish-start);
  Serial.println("us]");
}

void randomCombination() {// calling this takes 2160~2180 us
  start = micros();
  for (int i = 0; i < CELLS; i++) {
    panel.setPixelColor(i, color[random(8)]);
  }
  panel.show();
  finish = micros();
}

void resetPanel() { // this takes about 608~616 us
  start = micros();
  for (int i = 0; i < CELLS; i++) {
    panel.setPixelColor(i, 0x000000);
  }
  panel.show();
  finish = micros();
}
 
