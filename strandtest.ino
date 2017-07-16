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
uint16_t pitch_ring = 250;
uint16_t pitch_mid  = 100;



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
  RING_STATE_RAINBOW,
  RING_STATE_WIPE,
  RING_STATE_NUM
};

enum RING_STATE ring_state = RING_STATE_GREEN;

enum MID_STATE {

  MID_STATE_RED = 0,
  MID_STATE_GREEN,
  MID_STATE_BLUE,
  MID_STATE_CHASE_1,
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

     case RING_STATE_RAINBOW:
       pitch_ring = 250; //we must to be a litle bit faster
       effekt_RainbowRing();
       break;

     case RING_STATE_WIPE:
       pitch_ring = 90;
       effekt_colorWipeRing();
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


     case MID_STATE_CHASE_1:
       theaterChaseMid( strip.Color(255, 255, 255));  
       break;  
  
  }
}//end ring StateMachinge



/*********************************************************************************************
                       --------------
                      |    Effekts   |
                       --------------
**********************************************************************************************/ 

//Effekt Ring
void effekt_set_color_ring( uint32_t c)
{
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);    
  }    
  strip.show(); 
}


void effekt_RainbowRing(void) {

  static int8_t led_cnt = 0;
  static int8_t q = 0;

  strip.setPixelColor(led_cnt,  Wheel( (q) % 255) );
  strip.show();

  //we full, clear the strip
  if(++q > 255) q = 0;  
  if(++led_cnt > strip.numPixels()) led_cnt = 0;      
}


void effekt_colorWipeRing(void) {

  static int8_t led_cnt = 0;
  static int8_t q = 0;
  static uint32_t c = Wheel( (q) % 255);

  strip.setPixelColor(led_cnt, c );
  strip.show();

  //we full, clear the strip
  if(++led_cnt > strip.numPixels()){
    //calc new Color
    if(q == 0) c = strip.Color(255, 0, 0);
    else if(q == 1) c = strip.Color(0, 255, 0);
    else if(q == 2) c = strip.Color(0, 0, 255);
    else c = strip.Color(255, 255, 255);
    if(++q > 4) q = 0;      
    led_cnt = 0;      
  }
}


//Mid Effetks
void effekt_set_color_mid( uint32_t c)
{
  for (uint16_t i=0; i < mids.numPixels(); i++) {
    mids.setPixelColor(i, c);    
  }    
  mids.show(); 
}


//Theatre-style crawling lights.
void theaterChaseMid(uint32_t c) {

  static int8_t q = 0;
  static uint8_t j = 0;

  if(++q >= 3) q = -1;

  if(++j > 255) j = 0;
  

   for (uint16_t i=0; i < mids.numPixels(); i=i+1) {
    mids.setPixelColor(i, 0);        //turn every third pixel off
   }

  for (uint16_t i=0; i < mids.numPixels(); i=i+2) {
    mids.setPixelColor(i+q, Wheel( (i+j) % 255) );    //turn every third pixel on
  }
  mids.show();
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



/*********************************************************************************************
                       ----------------------
                      | Button State Machine |
                       ----------------------
**********************************************************************************************/ 
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
}//end check Button


