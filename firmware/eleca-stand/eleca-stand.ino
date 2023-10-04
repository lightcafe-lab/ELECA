/* This example shows how to make an LED pattern with a large
 * dynamic range using the the extra 5-bit brightness register in
 * the APA102.
 *
 * It sets every LED on the strip to white, zwith the dimmest
 * possible white at the input end of the strip and the brightest
 * possible white at the other end, and a smooth logarithmic
 * gradient between them.
 *
 * The dimmest possible white is achieved by setting the red,
 * green, and blue color channels to 1, and setting the
 * brightness register to 1.  The brightest possibe white is
 * achieved by setting the color channels to 255 and setting the
 * brightness register to 31.
 */

/* By default, the APA102 library uses pinMode and digitalWrite
 * to write to the LEDs, which works on all Arduino-compatible
 * boards but might be slow.  If you have a board supported by
 * the FastGPIO library and want faster LED updates, then install
 * the FastGPIO library and uncomment the next two lines: */
// #include <FastGPIO.h>
// #define APA102_USE_FAST_GPIO

#include <APA102.h>

// Define which pins to use.
const uint8_t dataPin = 7;
const uint8_t clockPin = 10;
const byte interruptPin = 0;
const byte ledR = 1;
const byte ledG = 2;
const byte ledB = 3;
int led = D8;

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



uint8_t mode=0;
uint8_t fled_mode = 0;
bool ledFlag = 0;

void mode_sw_func() {
  if(ledFlag){
    ledFlag=0;

  mode++;
  if(mode==1){
    fled_mode=0;
    digitalWrite(led, HIGH);   // turn the LED on 
    digitalWrite(ledR, LOW);   // turn the LED on 
    digitalWrite(ledG, HIGH);   // turn the LED on 
    digitalWrite(ledB, HIGH);   // turn the LED on 
  }
  if(mode==2){
    fled_mode=0;
    digitalWrite(led, LOW);   // turn the LED on 
    digitalWrite(ledR, HIGH);   // turn the LED on 
    digitalWrite(ledG, LOW);   // turn the LED on 
    digitalWrite(ledB, HIGH);   // turn the LED on 
  }
  if(mode==3){
    fled_mode=1;    
    digitalWrite(led, HIGH);   // turn the LED on       
    digitalWrite(ledR, HIGH);   // turn the LED on 
    digitalWrite(ledG, HIGH);   // turn the LED on 
    digitalWrite(ledB, LOW);   // turn the LED on 
  }
  if(mode==4){
    fled_mode=1;    
    digitalWrite(led, LOW);   // turn the LED on   
    digitalWrite(ledR, HIGH);   // turn the LED on 
    digitalWrite(ledG, HIGH);   // turn the LED on 
    digitalWrite(ledB, HIGH);   // turn the LED on 
  }
  if(mode==5)mode=0;
    }
}

void setup()
{
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);   // turn the LED on 
//    pinMode(ledR, OUTPUT);
//  digitalWrite(ledR, HIGH);   // turn the LED on 
//    pinMode(ledG, OUTPUT);
//  digitalWrite(ledG, HIGH);   // turn the LED on 
//    pinMode(ledB, OUTPUT);
//  digitalWrite(ledB, HIGH);   // turn the LED on 
//  
//  pinMode(interruptPin, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(interruptPin), mode_sw_func, RISING);
}



// This function sends a white color with the specified power,
// which should be between 0 and 7905.
void sendWhite(uint16_t power)
{
  
  // Choose the lowest possible 5-bit brightness that will work.
  uint8_t brightness5Bit = 1;
  while(brightness5Bit * 255 < power && brightness5Bit < 31)
  {
    brightness5Bit++;
  }

  // Uncomment this line to simulate an LED strip that does not
  // have the extra 5-bit brightness register.  You will notice
  // that roughly the first third of the LED strip turns off
  // because the brightness8Bit equals zero.
  //brightness = 31;

  // Set brightness8Bit to be power divided by brightness5Bit,
  // rounded to the nearest whole number.
  uint8_t brightness8Bit = (power + (brightness5Bit / 2)) / brightness5Bit;

  // Send the white color to the LED strip.  At this point,
  // brightness8Bit multiplied by brightness5Bit should be
  // approximately equal to power.
  ledStrip.sendColor(brightness8Bit, brightness8Bit, brightness8Bit, brightness5Bit);
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
int loopcount =0;
volatile int state=LOW;
void loop()
{

  uint8_t time = millis() >> 5;

  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t p = time - i * 8;
    
  if(fled_mode==0){
    colors[i] = hsvToRgb((uint32_t)p * 359 / 256, 255, 255);  
  }else if(fled_mode==1){
    colors[i] =hsvToRgb(0,0,0);  
  }
  }

  ledStrip.write(colors, ledCount, brightness);

  delay(100);
  ledFlag=1;
}
