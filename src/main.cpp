#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include "SR04.h"

#define trig_pin_1 5
#define echo_pin_1 18
#define trig_pin_2 17
#define echo_pin_2 16

SR04 sensor1 = SR04(echo_pin_1, trig_pin_1);
SR04 sensor2 = SR04(echo_pin_2, trig_pin_2);

long d1;            
long d2;
long t1 = 0;
long t2 = 0;
float max_distance = 30;

uint16_t people_counter = 0;
uint16_t entrance_counter = 0;
uint16_t exit_counter = 0;

BLEAdvertising *advertising;

void advertiseCounters() {
  BLEAdvertisementData advdata;
  std::string counterData = "";
  counterData += char(0x09);
  counterData += char(0xFF);
  counterData += char(0xE5);
  counterData += char(0x02);
  counterData += char(people_counter & 0xFF);
  counterData += char((people_counter >> 8) & 0xFF);
  counterData += char(entrance_counter & 0xFF);
  counterData += char((entrance_counter >> 8) & 0xFF);
  counterData += char(exit_counter & 0xFF);
  counterData += char((exit_counter >> 8) & 0xFF);
  advdata.addData(counterData);
  advertising->setAdvertisementData(advdata);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("peter");
  advertising = BLEDevice::getAdvertising();
  advertising->start();
  advertiseCounters();
}

void loop() {
  d1 = sensor1.Distance();
  d2 = sensor2.Distance();

  if (d1 < max_distance)
    t1 = millis();
  else if (d2 < max_distance)
    t2 = millis();

  if (t1 > 0 && t2 > 0) {
    if (t1 < t2) {
      Serial.println("Left to right");
      people_counter++;
      entrance_counter++;
      advertiseCounters();
    } else if (t2 < t1) {
      Serial.println("Right to left");
      if (people_counter > 0) people_counter--;
      exit_counter++;
      advertiseCounters();
    }
    t1 = 0;
    t2 = 0;
    delay(1000);
  }
}
