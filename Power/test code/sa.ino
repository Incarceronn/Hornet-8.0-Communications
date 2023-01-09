//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in loopback mode, for the Raspberry Pi Pico
//  Demo sketch that uses SPI1
//——————————————————————————————————————————————————————————————————————————————

  

//——————————————————————————————————————————————————————————————————————————————

#include <ACAN2515.h>

//——————————————————————————————————————————————————————————————————————————————
// The Pico has two SPI peripherals, SPI and SPI1. Either (or both) can be used.
// The are no default pin assignments so they must be set explicitly.
// Testing was done with Earle Philhower's arduino-pico core:
// https://github.com/earlephilhower/arduino-pico
//——————————————————————————————————————————————————————————————————————————————

static const byte MCP2515_SCK  = 13 ; // SCK input of MCP2515 (adapt to your design)
static const byte MCP2515_MOSI = 11 ; // SDI input of MCP2515 (adapt to your design)
static const byte MCP2515_MISO = 12 ; // SDO output of MCP2515 (adapt to your design)
static const byte MCP2515_CS   = 10 ;  // CS input of MCP2515 (adapt to your design)
//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Driver object
//——————————————————————————————————————————————————————————————————————————————

ACAN2515 can (MCP2515_CS, SPI, 255) ;

//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Quartz: adapt to your design
//——————————————————————————————————————————————————————————————————————————————

static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL ; // 20 MHz

int current_reading1 = A2;
int batt1_v_reading = A0;
int batt2_v_reading = A1;
//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————

void setup () {
  pinMode(current_reading1,INPUT);
  pinMode(batt1_v_reading,INPUT);
  pinMode(batt2_v_reading,INPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  Serial.begin(9600);
  //--- Switch on builtin led
  pinMode (LED_BUILTIN, OUTPUT) ;
  digitalWrite (LED_BUILTIN, HIGH) ;
  //--- Start serial
  Serial.begin (115200) ;
  //--- Wait for serial (blink led at 10 Hz during waiting)
//  while (!Serial) {
//    delay (50) ;
//    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
//  }
  //--- There are no default SPI1 pins so they must be explicitly assigned
  //--- Begin SPI1
  SPI.begin () ;
  //--- Configure ACAN2515
  Serial.println ("Configure ACAN2515") ;
  Serial.println ("Configure help") ;
  ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
  // settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
  const uint16_t errorCode = can.begin (settings, NULL) ;
  //const uint16_t errorCode = can.begin (settings, [] {}) ;
  if (errorCode == 0) {
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Propagation Segment: ") ;
    Serial.println (settings.mPropagationSegment) ;
    Serial.print ("Phase segment 1: ") ;
    Serial.println (settings.mPhaseSegment1) ;
    Serial.print ("Phase segment 2: ") ;
    Serial.println (settings.mPhaseSegment2) ;
    Serial.print ("SJW: ") ;
    Serial.println (settings.mSJW) ;
    Serial.print ("Triple Sampling: ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate: ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ? ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Sample point: ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
  }else{
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

//----------------------------------------------------------------------------------------------------------------------

static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

//——————————————————————————————————————————————————————————————————————————————

void loop () {
  can.poll();
  byte nano_v_reading1=int(batt1_v_reading, HEX);
  byte nano_v_reading2=int(batt2_v_reading, HEX);
  byte nano_c_reading=int(current_reading1, HEX);
  CANMessage frame ;
   if (gBlinkLedDate < millis ()) {
     gBlinkLedDate += 2000 ;
     digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
     frame.ext = false ;
     frame.id = 0xFF ;
     frame.len = 8 ;
     frame.data [0] = nano_v_reading1 ;
     frame.data [1] = nano_v_reading2 ;
     frame.data [2] = nano_c_reading;
     frame.data [3] = 0x00 ;
     frame.data [4] = 0x00 ;
     frame.data [5] = 0x00 ;
     frame.data [6] = 0x00 ;
     frame.data [7] = 0x00 ;
     //for (int i = 0; i < 3; ++i) Serial.println(frame.data[i]);
     const bool ok = can.tryToSend (frame) ;
     if (ok) {
       gSentFrameCount += 1 ;
       Serial.print ("Sent: ") ;
       Serial.println (gSentFrameCount) ;
     }else{
       Serial.println ("Send failure") ;
     }
   }
//  if (can.receive (frame)) {
//    gReceivedFrameCount ++ ;
//    Serial.print ("  id: ");Serial.println (frame.id,HEX);
//    Serial.print ("  ext: ");Serial.println (frame.ext);
//    Serial.print ("  rtr: ");Serial.println (frame.rtr);
//    Serial.print ("  len: ");Serial.println (frame.len);
//    Serial.print ("  data: ");
//    for(int x=0;x<frame.len;x++) {
//      Serial.print (frame.data[x],HEX); Serial.print(":");
//    }
//    Serial.println ("");
//    Serial.print ("Received: ") ;
//    Serial.println (gReceivedFrameCount) ;
//  }
}

//——————————————————————————————————————————————————————————————
