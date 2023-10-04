#include <APA102.h>
#include "Wire.h"
#include "SPI.h"

//pov_handshake_speed
#define HS_SPD_NRML 1000
#define CHAR_LC_WIDTH 355
#define CHAR_LC_HEIGHT 16
#define POV_INV_TIME_MS 500


LIS3DH myIMU; //Default constructor is I2C, addr 0x19.

// Define which pins to use.
const uint8_t dataPin = 7;
const uint8_t clockPin = 10;
const byte interruptPin = 0;
///const byte swPin = 0;
const byte ledR = 1;
const byte ledG = 2;
const byte ledB = 3;
const byte ledW = 8;
// Create an object for writing to the LED strip.
APA102<dataPin, clockPin> ledStrip;

// Set the number of LEDs to control.
const uint16_t ledCount = 24;


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



int pov_disp_buf[CHAR_LC_HEIGHT][CHAR_LC_WIDTH]={
  {0,0,2,2,0,3,3,0,4,4,0,5,5,0,6,6,0,7,7,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,2,2,0,3,3,0,4,4,0,5,5,0,6,6,0,7,7,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,2,2,0,3,3,0,4,4,0,5,5,0,6,6,0,7,7,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,0,0,0,1,0,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,0,0,1,0,0,1,1,0,1,1,0},
  {0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0},
  {0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0},
  {0,0,1,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,1,0,0,1,0,0,1,0,1,0,1,1,0,1,1,0},
  {0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,1,1,1,0,1,0,0,1,0,0},
  {0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0},
  {0,0,1,1,1,0,1,0,1,1,1,1,0,1,0,1,0,0,1,0,0,1,1,0,1,0,1,0,1,0,0,1,1,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
uint16_t hsv_b[9][3]={
   {0,0,0},  {240,255,255},  { 60,255,255}, //none,char,yellow
   {  0,255,255},  {300,255,255},  {190,255,255}, //red,purple,skyblue
   {240,255,255},  { 70,255,255},  {120,255,255}  //blue,palegreen,green
};

bool pov_init_flag=1;
bool pov_inv_flag=0;
long handShakeSpeedMs=0;
// Create a buffer for holding the colors (3 bytes per color).
rgb_color display_colors[ledCount];
rgb_color preset__cw_colors[ledCount][CHAR_LC_WIDTH];
rgb_color preset_ccw_colors[ledCount][CHAR_LC_WIDTH];

void pov_setup(){
  for (int j=0; j < CHAR_LC_WIDTH-1; j++) {
      for (int k=0; k<CHAR_LC_HEIGHT-1;k++){
      
        preset__cw_colors[ledCount-k-1][j]=hsvToRgb( 
                                          hsv_b[pov_disp_buf[k][j]][0],
                                          hsv_b[pov_disp_buf[k][j]][1],
                                          hsv_b[pov_disp_buf[k][j]][2]);
        preset_ccw_colors[ledCount-k-1][j]=hsvToRgb( 
                                          hsv_b[pov_disp_buf[k][CHAR_LC_WIDTH-1-j]][0],
                                          hsv_b[pov_disp_buf[k][CHAR_LC_WIDTH-1-j]][1],
                                          hsv_b[pov_disp_buf[k][CHAR_LC_WIDTH-1-j]][2]);        
                                        
//    Serial.print(preset__cw_colors[k][j][1],3);
//    Serial.print(preset__cw_colors[k][j][2],3);
    }
  }
}
void pov_run(){
  pov_inv_flag=!pov_inv_flag;
  for (int j=0; j < (CHAR_LC_WIDTH-1); j++) {
      for (int k=0; k<ledCount;k++){
        if(pov_inv_flag){
          display_colors[k] = preset__cw_colors[k][j];
          //digitalWrite(ledW, LOW);   // turn the LED on
        }
        else if(!pov_inv_flag){
          display_colors[k] = preset_ccw_colors[k][j];
          //digitalWrite(ledW, HIGH);   // turn the LED on
        }
        
       //   display_colors[k] = preset__cw_colors[k][j];
      //display_colors[k] = preset_ccw_colors[k][j];

    }
  
    ledStrip.write(display_colors, ledCount, brightness);
    delay(3); 
  }
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

void setup() {
  pinMode(ledW, OUTPUT);
  digitalWrite(ledW, LOW);   // turn the LED on
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000); 
  //relax...
  //Serial.println("Processor came out of reset.\n");
  uint8_t time = millis() >> 5;
  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t p = time - i * 8;
    display_colors[i] = hsvToRgb(0,0,0);//(uint32_t)p * 359 / 256, 255, 255);  
  }

  ledStrip.write(display_colors, ledCount, brightness);
  
  //Call .begin() to configure the IMU
  Wire.setClock(400000);
  myIMU.begin();
  pov_setup();
}


void loop()
{
  pov_run();
  delay(40); 
}
