/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <APA102.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


const uint8_t dataPin = 7;
const uint8_t clockPin = 10;
const byte ledW = 8;
uint8_t fled_mode = 1;

// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Set the number of LEDs to control.
const uint16_t ledCount = 48;//24;
// Create a buffer for holding the colors (3 bytes per color).
rgb_color colors[ledCount];
// Set the brightness to use (the maximum is 31).
const uint8_t brightness = 1;

// We define "power" in this sketch to be the product of the
// 8-bit color channel value and the 5-bit brightness register.
// The maximum possible power is 255 * 31 (7905).
const uint16_t maxPower = 255 * 31;

// The power we want to use on the first LED is 1, which
// corresponds to the dimmest possible white.
const uint16_t minPower = 1;

// Calculate what the ratio between the powers of consecutive
// LEDs needs to be in order to reach the max power on the last
// LED of the strip.
const float multiplier = pow(maxPower / minPower, 1.0 / (ledCount - 1));

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
//        Serial.println("*********");
//        Serial.print("New value: ");
//        for (int i = 0; i < value.length(); i++)
//          Serial.print(value[i]);
//
//        Serial.println();
//        Serial.println("*********");
        if(value[0]==0&&value.length()==1){
          Serial.print("ON");
          Serial.println();
          //digitalWrite(ledW, LOW);   // turn the LED on
          fled_mode=0;
        }else if(value[0]==1&&value.length()==1){
          Serial.print("OFF");
          Serial.println();
          //digitalWrite(ledW, HIGH);   // turn the LED on
          fled_mode=1;
        }else {
          Serial.print("other value: ");
          for (int i = 0; i < value.length(); i++)
            Serial.print(value[i]);
          Serial.println();
        }        
      }
    }
};

void setup() {
  pinMode(ledW, OUTPUT);
  digitalWrite(ledW, HIGH);   // turn the LED on
  
  Serial.begin(115200);

  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to MyESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =)");

  BLEDevice::init("ELECA");//("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  //pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  pinMode(ledW, OUTPUT);
  digitalWrite(ledW, HIGH);   // turn the LED on 
}

void loop() {
  led_lighting();
  // put your main code here, to run repeatedly:
  //delay(2000);
  delay(100);
}

/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}
void led_lighting(){
    uint8_t time = millis() >> 5;

  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t p = time - i * 8;
    
  if(fled_mode){
    colors[i] = hsvToRgb(100, 255, 255);  
  }else if(!fled_mode){
    colors[i] = hsvToRgb(
      0, 255, 255);  
    //colors[i] =hsvToRgb(0,0,0);  
  }
  }

  ledStrip.write(colors, ledCount, brightness);
}
