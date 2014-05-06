#include <SoftwareSerial.h>

SoftwareSerial ss(16,15);

void setup() {
  ss.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (ss.available()>0) {
    Serial.println("data avail");
    Serial.write(ss.read());
  }
}

