#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "SR04.h"

#define trig_pin_1 5
#define echo_pin_1 18
#define trig_pin_2 17
#define echo_pin_2 16

// UUIDs
#define SERVICE_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DATE_CHAR_UUID          "e3b8e7e2-8c2a-4b9b-9e7e-2e8c2a4b9b9e"

SR04 sensor1 = SR04(echo_pin_1, trig_pin_1);
SR04 sensor2 = SR04(echo_pin_2, trig_pin_2);

long d1;            
long d2;
long t1 = 0;
long t2 = 0;
float max_distance = 80;

uint16_t people_counter = 0;
uint16_t entrance_counter = 0;
uint16_t exit_counter = 0;

uint32_t date = 14011111;

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pDateCharacteristic = NULL;
BLEAdvertising *advertising;

byte reset;

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
  advertising = BLEDevice::getAdvertising();
  advertising->setAdvertisementData(advdata);
  advertising->start();
}

class DateCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == sizeof(float)) {
      
      memcpy(&reset, value.data(), sizeof(byte));
      Serial.print("reset : ");
      Serial.println(reset);
    } else {
      Serial.println("envoi erroné");
    }
  }

  void onRead(BLECharacteristic *pCharacteristic) {
    pCharacteristic->setValue((uint8_t*)&date, sizeof(date));
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Client Connecté !");
  }
  void onDisconnect(BLEServer *pServer) {
    Serial.println("Client déconnecté !");
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("peter");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);

  // Date
  pDateCharacteristic = pService->createCharacteristic(
    DATE_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pDateCharacteristic->setCallbacks(new DateCallbacks());

  pService->start();

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
  if (reset != 0) {
    people_counter = 0;
    exit_counter = 0;
    entrance_counter = 0;
    reset = 0;
    advertiseCounters();
    Serial.println("Compteurs réinitialisés");
  }
}
