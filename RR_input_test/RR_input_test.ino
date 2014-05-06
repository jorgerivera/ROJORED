#include <Adafruit_NeoPixel.h>

#define BUFFER_LENGTH    3     // 3 bytes gives us 24 samples
#define NUM_INPUTS 4
//#define TARGET_LOOP_TIME 694   // (1/60 seconds) / 24 samples = 694 microseconds per sample 
//#define TARGET_LOOP_TIME 758  // (1/55 seconds) / 24 samples = 758 microseconds per sample 
#define TARGET_LOOP_TIME 744  // (1/56 seconds) / 24 samples = 744 microseconds per sample 

#define DEBUG 1
#define DEBUG3 1

#include "settings.h"

/////////////////////////
// STRUCT ///////////////
/////////////////////////
typedef struct {
  byte pinNumber;
  int keyCode;
  byte measurementBuffer[BUFFER_LENGTH]; 
  boolean oldestMeasurement;
  byte bufferSum;
  boolean pressed;
  boolean prevPressed;
  boolean isKey;
} 
RojoRedInput;

RojoRedInput inputs[NUM_INPUTS];

///////////////////////////////////
// VARIABLES //////////////////////
///////////////////////////////////
int bufferIndex = 0;
byte byteCounter = 0;
byte bitCounter = 0;
int mouseMovementCounter = 0; // for sending mouse movement events at a slower interval

int pressThreshold;
int releaseThreshold;
boolean inputChanged;

// Pin Numbers
// input pin numbers for kickstarter production board
int pinNumbers[NUM_INPUTS] = {
  12, 8, 13, 15, // 7, 6,     // top of makey makey board
//  5, 4, 3, 2, 1, 0,        // left side of female header, KEBYBOARD
//  23, 22, 21, 20, 19, 18   // right side of female header, MOUSE
//    12
};

#define PIN 16
#define INTERVAL 5000ul
#define CELLS 4
#define MAX_LEVEL 6

// timing
int loopTime = 0;
int prevTime = 0;
int loopCounter = 0;


Adafruit_NeoPixel test = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);

const uint32_t color[8] = { 0x000000, // NONE
  0x800000, // RED
  0x802000, // ORANGE
  0x606000, // YELLOW
  0x008000, // GREEN
  0x000080, // BLUE
  0x600060, // VIOLET
  0x404040  // WHITE
};
const uint8_t OFF = 0;
const uint8_t RED = 1;

uint8_t pending[NUM_INPUTS];

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
  initializeArduino();
  initializeInputs();
  for (int i=0;i<NUM_INPUTS;i++) {
    pending[i] = 0;
  }
  testLEDs();
}

void testLEDs() {
  for (int i=0;i<NUM_INPUTS; i++) {
    test.setPixelColor(i,color[RED]);
    test.show();
    delay(500);
  }
  for (int i=0;i<NUM_INPUTS; i++) {
    test.setPixelColor(i,color[0]);
    test.show();
    delay(500);
  }
  
}
uint8_t level = 0;
unsigned long start, finish;

void loop() {
  /*long now = millis();
  if (now > nextUpdate) {
    level++;
    if (level > MAX_LEVEL) {
      level = 1;
    }
    getLevelInfo(level);
    nextUpdate = now + INTERVAL;    
  }  */
  updateMeasurementBuffers();
  updateBufferSums();
  updateBufferIndex();
  updateInputStates();
  updateLEDs();
  addDelay();
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

  uint16_t selected = selectRandomly(CELLS, numRED);
  Serial.print("Selection [");
  Serial.print(selected, BIN);
  Serial.println("]");
  Serial.println();
  for (int j=0; j<CELLS; j++){
    if (selected & B1 != 0) {
      test.setPixelColor(j, color[1]);
    } else {
      test.setPixelColor(j, color[0]);
    }
    selected = selected >> 1;
  }
  test.show();
}

void updateLEDs() {

  for (int l=0; l<NUM_INPUTS; l++) {
    if (pending[l]==1) {
      pending[l] = 0;
      Serial.print("updateLEDs [");
      Serial.print(l);
      Serial.println("] changed");
      if (test.getPixelColor(l)==color[RED]) {
        test.setPixelColor(l, color[OFF]);
      } else {
        test.setPixelColor(l, color[RED]);
      }
      test.show();
    }  
  }
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

uint16_t selectRandomly(uint8_t total, uint8_t select) {
  uint8_t numSelected = 0;
  uint16_t selectedBitArray = 0;
  while (numSelected < select) {
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
    
//// START METHODS TAKEN FROM makey_makey.ino /////  

//////////////////////////
// INITIALIZE ARDUINO
//////////////////////////
void initializeArduino() {

  /* Set up input pins 
   DEactivate the internal pull-ups, since we're using external resistors */
  for (int i=0; i<NUM_INPUTS; i++)
  {
    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);
  }
 

#ifdef DEBUG
 // delay(4000); // allow us time to reprogram in case things are freaking out
#endif

}

///////////////////////////
// INITIALIZE INPUTS
///////////////////////////
void initializeInputs() {

  float thresholdPerc = SWITCH_THRESHOLD_OFFSET_PERC;
  float thresholdCenterBias = SWITCH_THRESHOLD_CENTER_BIAS/50.0;
  float pressThresholdAmount = (BUFFER_LENGTH * 8) * (thresholdPerc / 100.0);
  float thresholdCenter = ( (BUFFER_LENGTH * 8) / 2.0 ) * (thresholdCenterBias);
  pressThreshold = int(thresholdCenter + pressThresholdAmount);
  releaseThreshold = int(thresholdCenter - pressThresholdAmount);

#ifdef DEBUG
  Serial.println(pressThreshold);
  Serial.println(releaseThreshold);
#endif

  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].pinNumber = pinNumbers[i];
    inputs[i].keyCode = keyCodes[i];

    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].measurementBuffer[j] = 0;
    }
    inputs[i].oldestMeasurement = 0;
    inputs[i].bufferSum = 0;

    inputs[i].pressed = false;
    inputs[i].prevPressed = false;

    inputs[i].isKey = false;


  }
}


//////////////////////////////
// UPDATE MEASUREMENT BUFFERS
//////////////////////////////
void updateMeasurementBuffers() {

  for (int i=0; i<NUM_INPUTS; i++) {

    // store the oldest measurement, which is the one at the current index,
    // before we update it to the new one 
    // we use oldest measurement in updateBufferSums
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    inputs[i].oldestMeasurement = (currentByte >> bitCounter) & 0x01; 

    // make the new measurement
    boolean newMeasurement = digitalRead(inputs[i].pinNumber);

    // invert so that true means the switch is closed
    newMeasurement = !newMeasurement; 

    // store it    
    if (newMeasurement) {
      currentByte |= (1<<bitCounter);
    } 
    else {
      currentByte &= ~(1<<bitCounter);
    }
    inputs[i].measurementBuffer[byteCounter] = currentByte;
  }
}

///////////////////////////
// UPDATE BUFFER SUMS
///////////////////////////
void updateBufferSums() {

  // the bufferSum is a running tally of the entire measurementBuffer
  // add the new measurement and subtract the old one

  for (int i=0; i<NUM_INPUTS; i++) {
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    boolean currentMeasurement = (currentByte >> bitCounter) & 0x01; 
    if (currentMeasurement) {
      inputs[i].bufferSum++;
    }
    if (inputs[i].oldestMeasurement) {
      inputs[i].bufferSum--;
    }
  }  
}

///////////////////////////
// UPDATE BUFFER INDEX
///////////////////////////
void updateBufferIndex() {
  bitCounter++;
  if (bitCounter == 8) {
    bitCounter = 0;
    byteCounter++;
    if (byteCounter == BUFFER_LENGTH) {
      byteCounter = 0;
    }
  }
}

///////////////////////////
// UPDATE INPUT STATES
///////////////////////////
void updateInputStates() {
  inputChanged = false;
  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].prevPressed = inputs[i].pressed; // store previous pressed state (only used for mouse buttons)
    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        inputChanged = true;
        inputs[i].pressed = false;
        // do RELEASE stuff here
        Serial.print("[");
        Serial.print(i);
        Serial.println("] released");
      }

    } 
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputChanged = true;
        inputs[i].pressed = true; 
        // do PRESS stuff here
        pending[i] = 1;
        Serial.print("[");
        Serial.print(i);
        Serial.println("] pressed");
      }
    }
  }
#ifdef DEBUG3
  if (inputChanged) {
    Serial.println("change");
  }
#endif
}  


///////////////////////////
// ADD DELAY
///////////////////////////
void addDelay() {

  loopTime = micros() - prevTime;
  if (loopTime < TARGET_LOOP_TIME) {
    int wait = TARGET_LOOP_TIME - loopTime;
    delayMicroseconds(wait);
  }

  prevTime = micros();

#ifdef DEBUG_TIMING
  if (loopCounter == 0) {
    int t = micros()-prevTime;
    Serial.println(t);
  }
  loopCounter++;
  loopCounter %= 999;
#endif

}

//// END METHODS TAKEN FROM makey_makey.ino /////
