#include <Adafruit_NeoPixel.h>
#include "TimerOne.h"
#include <stdio.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Defines
#define PIN 6
#define LED_CNT_RING 12
#define LED_CNT_MID  3
#define LED_CNT (LED_CNT_RING + LED_CNT_MID)
#define BOARD_LED 13


//Global Variabels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_CNT_RING + LED_CNT_MID, PIN, NEO_GRB + NEO_KHZ800);
unsigned long millis_cnt = 0; 
uint16_t pitch = 240;

uint32_t colors[LED_CNT] ={0};

static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c) ;
    return 0 ;
}


//Init Funktions
void setup() {

 //Init Strip
 strip.begin();
 strip.show(); // Initialize all pixels to 'off'

 Serial.begin(9600);

 //pinMode(BOARD_LED, OUTPUT);
 millis_cnt = millis();

 // fill in the UART file descriptor with pointer to writer.
 fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

 // The uart is the standard output device STDOUT.
 stdout = &uartout ; 
   
}


void loop() {


  static uint8_t count =0;
  

  //pitch
  if(millis() >= millis_cnt){

    theaterChaseRing(strip.Color(80, 127, 255), 3); // Blue 
    theaterChaseMid(strip.Color(0, 255, 0), 3); // Blue    
       
    millis_cnt += (unsigned long long)pitch;
  }//end pitch

  for(int i=0; i<LED_CNT; i++)    
    strip.setPixelColor(i, colors[i]);
  strip.show();
   
}


//-------------------------| Effects |--------------------------------
//
void colorMiddel(uint32_t c){

  for(uint16_t i=LED_CNT_RING; i<LED_CNT_RING + LED_CNT_MID; i++) {    
    strip.setPixelColor(i, c);
    strip.show();
  }
}

void colorCircle(uint32_t c, uint8_t wait){

  for(uint16_t i=0; i<LED_CNT_RING; i++) {
    strip.setPixelColor(i-1, 0);    
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  //shut down last Pixel
 strip.setPixelColor(LED_CNT_RING-1, 0);
  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<LED_CNT_RING; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChaseRing(uint32_t c, uint8_t chase) {
  
  static int q = 0;
  //calc chase Step
  if(q++ > chase)
   q = 0;

  for (uint16_t i=0; i < LED_CNT_RING; ) {
    colors[i+q] = 0;    //turn every third pixel on
    i=i+3;
  }
  for (uint16_t i=0; i < LED_CNT_RING;) {
     colors[i+q] = c;       //turn every third pixel off
     i=i+3;
  }
}

//Theatre-style crawling lights.
void theaterChaseMid(uint32_t c, uint8_t chase) {
  
  static int q = 0;
  //calc chase Step
  if(q++ > chase)
   q = 0;

  for (uint16_t i=LED_CNT_RING; i < LED_CNT; ) {
    colors[i+q] = c;    //turn every third pixel on
    i=i+3;
  }
  strip.show();
  for (uint16_t i=LED_CNT_RING; i < LED_CNT;) {
     colors[i+q], 0;        //turn every third pixel off
     i=i+3;
  }
}






//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



//  colorWipe(strip.Color(255, 0, 0), 50); // Red
 // colorWipe(strip.Color(0, 255, 0), 50); // Green
 // colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
 // theaterChase(strip.Color(127, 127, 127), 50); // White
 // theaterChase(strip.Color(127, 0, 0), 50); // Red
 // theaterChase(strip.Color(0, 0, 127), 50); // Blue

 // rainbow(20);
 // rainbowCycle(20);
  //theaterChaseRainbow(50);

