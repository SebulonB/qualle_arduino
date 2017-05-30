#include <Adafruit_NeoPixel.h>
#include "TimerOne.h"
#include <EEPROM.h>
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
#define BUTTON 2


//Global Variabels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_CNT_RING, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel mids = Adafruit_NeoPixel(LED_CNT_MID, 5, NEO_GRB + NEO_KHZ800);
unsigned long millis_cnt_ring = 0; 
unsigned long millis_cnt_mid = 0; 
unsigned long millis_cnt_100ms = 0;
uint16_t pitch_ring = 1000;
uint16_t pitch_mid  = 1000;



static FILE uartout = {0};



enum BUTTON_STATE
{
 BUTTON_STATE_RUN = 0,
 BUTTON_STATE_CONFIG_RING,
 BUTTON_STATE_GO_TO_CONFIG_RING,
 BUTTON_STATE_CONFIG_MID,
 BUTTON_STATE_NUM 
};

enum BUTTON_STATE button_state = BUTTON_STATE_RUN;
bool button_read = 0;
bool button_latch = false;

enum RING_STATE {

  RING_STATE_RED = 0,
  RING_STATE_GREEN,
  RING_STATE_BLUE,
  RING_STATE_NUM
};

enum RING_STATE ring_state = RING_STATE_GREEN;

enum MID_STATE {

  MID_STATE_RED = 0,
  MID_STATE_GREEN,
  MID_STATE_BLUE,
  MID_STATE_NUM
};

enum MID_STATE mid_state = MID_STATE_RED;


enum EEPROM_ADR
{
  EEPROM_ADR_STATE_RING = 0,
  EEPROM_ADR_STATE_MID  = 4,
};


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

 pinMode(BUTTON, INPUT); 

 //Init Strip
 strip.begin();
 strip.show(); // Initialize all pixels to 'off'

 mids.begin();
 mids.show(); // Initialize all pixels to 'off'

 Serial.begin(9600);

 //pinMode(BOARD_LED, OUTPUT);
 millis_cnt_ring  = millis();
 millis_cnt_mid   = millis();
 millis_cnt_100ms = millis();

 ring_state = EEPROM.read((int)EEPROM_ADR_STATE_RING);
 mid_state = EEPROM.read((int)EEPROM_ADR_STATE_MID);

 // fill in the UART file descriptor with pointer to writer.
 fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

 // The uart is the standard output device STDOUT.
 stdout = &uartout ; 
   
}



void loop() {


  static uint8_t count =0;
  uint32_t color;

  //do something all 100ms
  if(millis() >= millis_cnt_100ms){
    check_button_state();
    millis_cnt_100ms += (unsigned long long)100;
  }
  
  //pitch ring
  if(millis() >= millis_cnt_ring){
    if(button_state == BUTTON_STATE_RUN){
      ring_state_machine();      
    }//end if button State
    millis_cnt_ring += (unsigned long long)pitch_ring;
  }//end pitch

  //pitch mid
  if(millis() >= millis_cnt_mid){
    if(button_state == BUTTON_STATE_RUN){
      mid_state_machine();      
    }//end if button State
    millis_cnt_mid += (unsigned long long)pitch_mid;
  }//end pitch
}


void ring_state_machine(void)
{

  printf("Ring State: %d \n", ring_state);
  switch(ring_state)
  {
     case RING_STATE_RED:
       effekt_set_color_ring( strip.Color(255, 0, 0));  
       break;  

     case RING_STATE_GREEN:
       effekt_set_color_ring( strip.Color(0, 255, 0));  
       break;   

     case RING_STATE_BLUE:
       effekt_set_color_ring( strip.Color(0, 0, 255));  
       break;  
    
  }
}//end ring StateMachinge


void mid_state_machine(void)
{
  printf("Mid State: %d\n", mid_state);  
  switch(mid_state)
  {
     case MID_STATE_RED:
       effekt_set_color_mid( strip.Color(255, 0, 0));  
       break;  

     case MID_STATE_GREEN:
       effekt_set_color_mid( strip.Color(0, 255, 0));  
       break;   

     case MID_STATE_BLUE:
       effekt_set_color_mid( strip.Color(0, 0, 255));  
       break;  
    
  }
}//end ring StateMachinge




void effekt_set_color_ring( uint32_t c)
{
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);    
  }    
  strip.show(); 
}

void effekt_set_color_mid( uint32_t c)
{
  for (uint16_t i=0; i < mids.numPixels(); i++) {
    mids.setPixelColor(i, c);    
  }    
  mids.show(); 
}




void check_button_state()
{
  static uint32_t time_old = 0;
  static uint8_t button_cnt = 0;

  switch(button_state)
  {

    case BUTTON_STATE_RUN:
    
      if( !digitalRead(BUTTON) && (button_latch == false) ){
        button_state =  BUTTON_STATE_GO_TO_CONFIG_RING;
        time_old = millis();
        //printf("Change Button state to: GO RING\n");
        for (uint16_t i=0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, 0);   
          strip.show();
        }    
        for (uint16_t i=0; i < mids.numPixels(); i++) {
          mids.setPixelColor(i, 0);   
          mids.show();
        }                  
      }
      //if we come from an other state where the button is pressed
      if( digitalRead(BUTTON) ) button_latch = false;
      break;

    case BUTTON_STATE_GO_TO_CONFIG_RING:
      if( !digitalRead(BUTTON) ){
        if((millis() - time_old) >= 2000){
          button_state =  BUTTON_STATE_CONFIG_RING;  
          button_latch = true;
          button_cnt = (uint8_t)ring_state;
          printf("Change Button State to: CONFIG RING\n");  
          for (uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(255, 0, 0));    //turn every third pixel on
            strip.show();
            delay(100);
          }            
          for (uint16_t i=0; i < strip.numPixels(); i++) {
            if(i <= ring_state)
              strip.setPixelColor(i, strip.Color(255, 0, 0));    //turn every third pixel on
            else
              strip.setPixelColor(i, strip.Color(0, 0, 0));    //turn every third pixel on            
            strip.show();
          }     
        }
      }
      else
      {
        button_state = BUTTON_STATE_RUN;
        //printf("GOTO RUN\n");
      }
      break;

    case BUTTON_STATE_CONFIG_RING:
        //select ring config
        if(button_read == 1 && (digitalRead(BUTTON) == 0)){
          if(++button_cnt >= RING_STATE_NUM){
            button_cnt = 0;
          }  
          time_old = millis();
          button_latch = false;
          //printf("Button CNT: %d\n", button_cnt);
          for (uint16_t i=0; i < strip.numPixels(); i++) {
            if(i <= button_cnt)
              strip.setPixelColor(i, strip.Color(255, 0, 0));    //turn every third pixel on
            else
              strip.setPixelColor(i, strip.Color(0, 0, 0));    //turn every third pixel on            
            strip.show();
          }  
          
        }

        //go to config mid mode
        if( !button_latch && !digitalRead(BUTTON) && ((millis() - time_old) >= 2000) ){
          //get orginal BUTTON CNT
          if(button_cnt == 0) button_cnt = (RING_STATE_NUM - 1);
          else if(--button_cnt == 0xFF) button_cnt = 0; //!! works only with uint8_t
          ring_state = button_cnt;
          EEPROM.write((int)EEPROM_ADR_STATE_RING, (uint8_t)ring_state);

          button_cnt = (uint8_t)mid_state;
          button_state =  BUTTON_STATE_CONFIG_MID;  
          button_latch = true;
          
          //printf("Goto CONFIG MID, with RING state: %d\n", ring_state);      

          for (uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));    //turn every third pixel on
            strip.show();
            delay(100);
          }            
          for (uint16_t i=0; i < strip.numPixels(); i++) {
            if(i <= mid_state)
              strip.setPixelColor(i, strip.Color(0, 255, 0));    //turn every third pixel on
            else
              strip.setPixelColor(i, strip.Color(0, 0, 0));    //turn every third pixel on            
            strip.show();
          }  
                
        }

        //read old button State for (Flankenerkennung
        button_read = digitalRead(BUTTON);
      break;

    case BUTTON_STATE_CONFIG_MID:
        //select ring config
        if(button_read == 1 && (digitalRead(BUTTON) == 0)){
          if(++button_cnt >= MID_STATE_NUM){
            button_cnt = 0;
          }  
          time_old = millis();
          button_latch = false;
          printf("Button CNT: %d\n", button_cnt);
          for (uint16_t i=0; i < strip.numPixels(); i++) {
            if(i <= button_cnt)
              strip.setPixelColor(i, strip.Color(0, 255, 0));    //turn every third pixel on
            else
              strip.setPixelColor(i, strip.Color(0, 0, 0));    //turn every third pixel on            
            strip.show();
          }  

          
        }

        //go to run mode
        if( !button_latch && !digitalRead(BUTTON) && ((millis() - time_old) >= 2000) ){
          //get orginal BUTTON CNT
          if(button_cnt == 0) button_cnt = (MID_STATE_NUM - 1);
          else if(--button_cnt == 0xFF) button_cnt = 0; //!! works only with uint8_t
          mid_state = button_cnt;
          EEPROM.write((int)EEPROM_ADR_STATE_MID, (uint8_t)mid_state);
          button_state =  BUTTON_STATE_RUN;  
          button_latch = true;
          //printf("Goto RUN Mode, witht MID state: %d\n", mid_state);            
        }

        //read old button State for (Flankenerkennung
        button_read = digitalRead(BUTTON);
      break;      

    default:
        button_state = BUTTON_STATE_RUN;
        //printf("GO FROM default TO RUN");  
      break;  
       
  }





  
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
  uint16_t i;
  static uint16_t j =0;

  if(j++ >255)
    j=0;


    for(i=0; i<LED_CNT_RING; i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }

}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i;
  static uint16_t j = 0;

  if(j++ > (255 * 5));
    j=0;
  


    for(i=0; i< 13; i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }

}

//Theatre-style crawling lights.
void theaterChaseRing(uint32_t c, uint8_t chase) {
  
  static int q = 0;
  //calc chase Step
  if(q++ > chase)
   q = 0;

  for (uint16_t i=0; i < LED_CNT_RING; ) {
    strip.setPixelColor((i+q), 0);    //turn every third pixel on
    i=i+3;
  }

  for (uint16_t i=0; i < LED_CNT_RING;) {
     strip.setPixelColor((i+q), c);       //turn every third pixel off
     i=i+2;
  }

  
}

//Theatre-style crawling lights.
void theaterChaseMid(uint32_t c, uint8_t chase) {
  
  static int q , j= 128;
  //calc chase Step
  if(q++ > LED_CNT)
   q = LED_CNT_RING;

  if(j++ > 255)
    j =0;

  for (uint16_t i=LED_CNT_RING; i < LED_CNT; ) {
    strip.setPixelColor((i),  Wheel((i+j) & 255));    //turn every third pixel on
   i=i+1;
  }

  //strip.setPixelColor((q), 0);        //turn every third pixel off
 
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

