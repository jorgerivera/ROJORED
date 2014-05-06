#include <Adafruit_NeoPixel.h>

#define PIN 16
#define INTERVAL 5000ul
#define CELLS 16
#define MAX_LEVEL 25
#define NUM_INPUTS 4

Adafruit_NeoPixel test = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

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

const uint8_t levelInfo[26] = { 
  // 0x<num_red><num_other>
  0x00, // not really a level
  0x10, //Level  1
  0x11, //Level  2
  0x12, //Level  3
  0x13, //Level  4
  0x20, //Level  5
  0x22, //Level  6
  0x24, //Level  7
  0x28, //Level  8
  0x33, //Level  9
  0x35, //Level 10
  0x37, //Level 11
  0x39, //Level 12
  0x3B, //Level 13
  0x3D, //Level 14
  0x44, //Level 15
  0x46, //Level 16
  0x48, //Level 17
  0x4A, //Level 18
  0x4C, //Level 19
  0x51, //Level 20
  0x53, //Level 21
  0x55, //Level 22
  0x57, //Level 23
  0x59, //Level 24
  0x5B, //Level 25
};

int numOfLevels;
long nextUpdate = 0;

void setup() {
  test.begin();
  test.show();
  randomSeed(analogRead(0));
  Serial.begin(115200);
  numOfLevels = sizeof(levelInfo);
  Serial.print("number of levels: ");
  Serial.println(numOfLevels, DEC);
}

uint8_t level = 0;
unsigned long start, finish;

void loop() {
  long now = millis();
  if (now > nextUpdate) {
    level++;
    if (level > MAX_LEVEL) {
      level = 1;
    }
    getLevelInfo(level);
    nextUpdate = now + INTERVAL;    
  }   
}

void getLevelInfo(uint8_t level) {
  start = micros();
  
  uint8_t levelInf = levelInfo[level];
  uint8_t numOTHER = levelInf & 0x0F;
  uint8_t numRED = levelInf >> 4;
  finish = micros();
  Serial.print("LEVEL: [");
  Serial.print(level, DEC);
  Serial.println("]");
  Serial.print("RED: [");
  Serial.print(numRED);
  Serial.print("] OTHER: [");
  Serial.print(numOTHER);
  Serial.println("]");
  Serial.print("Time [");
  Serial.print(finish-start);
  Serial.println("]");

  uint16_t selected = selectRandomlyRED(CELLS, numRED);
  uint16_t selectedOther = selectRandomlyOTHER(CELLS, numOTHER, selected);
  for (int j=0; j<CELLS; j++){
    if (selected & B1 != 0) {
      test.setPixelColor(j, color[1]);
    } else {
      if (selectedOther & B1 != 0) {
        test.setPixelColor(j, color[random(6)+2]);
      } else {
        test.setPixelColor(j, color[0]);
      }      
    }
    selected = selected >> 1;
    selectedOther = selectedOther >> 1;
  }
  test.show();
}

/*
uint32_t getTotalCombinations(int total, int select) {
  uint32_t comb = 1;
  for (select; select>0; select--) {
    comb = comb * total;
    total--;
  }
  return comb;
}*/

uint16_t selectRandomlyRED(uint8_t total, uint8_t selectRED) {
  uint8_t numSelected = 0;
  uint16_t selectedBitArray = 0;
  while (numSelected < selectRED) {
    Serial.print("selectedBitArray ["); Serial.print(selectedBitArray,BIN); Serial.println("]");
    uint8_t thisSelection = random(total);
    Serial.println(thisSelection,BIN);
    uint16_t thisSelectionBitArray = 1 << thisSelection;
    Serial.println(thisSelectionBitArray,BIN);
    Serial.println(thisSelectionBitArray);
    if ((selectedBitArray & thisSelectionBitArray) == 0) { // not yet selected
       selectedBitArray = selectedBitArray | thisSelectionBitArray; // select it
       Serial.println(selectedBitArray,BIN);
       numSelected++;
    }
  }
  return selectedBitArray;
}
    
uint16_t selectRandomlyOTHER(uint8_t total, uint8_t selectOTHER, uint16_t alreadySelected) {
  uint8_t numSelected = 0;
  uint16_t selectedBitArray = 0;
  while (numSelected < selectOTHER) {
    Serial.print("selectedBitArray ["); Serial.print(selectedBitArray,BIN); Serial.println("]");
    uint8_t thisSelection = random(total);
    uint16_t thisSelectionBitArray = 1 << thisSelection;
    if (((alreadySelected & thisSelectionBitArray)==0) && ((selectedBitArray & thisSelectionBitArray) == 0)) { // not yet selected
       selectedBitArray = selectedBitArray | thisSelectionBitArray; // select it
       numSelected++;
    }
  }
  Serial.println(selectedBitArray,BIN);
  return selectedBitArray;
}
    
 
