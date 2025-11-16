#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "SMT100Sensor.h"
#include "config.h"

SMT100Sensor smt100_1(1);
SMT100Sensor smt100_2(2);

void setup() {
  Serial.begin(115200);

  smt100_1.begin();
  smt100_1.quickWarmup();

  smt100_2.begin();
  smt100_2.quickWarmup();
}

void loop() {
  // SMT100     
  SMT100Sensor::Reading smt1 = smt100_1.read();
  Serial.println(smt1.temperature);
  Serial.println(smt1.waterContent);
  Serial.println(smt1.permittivity);
  
  SMT100Sensor::Reading smt2 = smt100_2.read();
  Serial.println(smt2.temperature);
  Serial.println(smt2.waterContent);
  Serial.println(smt2.permittivity);
}