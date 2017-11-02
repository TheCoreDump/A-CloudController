#include "Common.h"

CRGBArray<NUM_LEDS> leds;
CommandBase* currentCommand;
CommandParser* mainCommandParser;

#ifndef USE_USB_SERIAL_PORT
SoftwareSerial mySerial(10, 11); // RX, TX
#endif


// Timer2 - 40Hz 

void setup() {

  // Set the timer for 30Hz
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 30.00120004800192 Hz increments
  OCR1A = 8332; // = 16000000 / (64 * 30.00120004800192) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 64 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts


  // Set up the comand processor
  mainCommandParser = new CommandParser();

  Serial.begin(9600);

  // Initialzie
  #ifndef USE_USB_SERIAL_PORT
   mySerial.begin(9600);
  #endif
  
  FastLED.addLeds<WS2812,3,RGB>(leds, NUM_LEDS); 

  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(100);
  
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(100);
  
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(100);
  
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {

  CommandBase* parsedCommand = NULL;

  #ifndef USE_USB_SERIAL_PORT

  if (mySerial.available() > 0)
  {
    int dataRead = mySerial.read();

    DEBUG_MESSAGE("Data Read");
    DEBUG_MESSAGE_DEC(dataRead);
    
    if (dataRead >= 0) {
      parsedCommand = mainCommandParser->ProcessSerialData(dataRead);
    }  
 
  #else

  if (Serial.available())
  {
    int dataRead = Serial.read();

    if (dataRead >= 0) {
      parsedCommand = mainCommandParser->ProcessSerialData(dataRead);
    }
    
  #endif

    if (parsedCommand) {
      DEBUG_MESSAGE("Command Parsed");

      noInterrupts();

      if (currentCommand) {
        DEBUG_MESSAGE("Stopping Current Command");
        currentCommand->Stop();
        delete currentCommand;
        currentCommand = NULL;
      }

      DEBUG_MESSAGE("Starting New Command");
      currentCommand = parsedCommand;
      currentCommand->Start();

      interrupts();
    }
  }
}

ISR(TIMER1_COMPA_vect){

   if (currentCommand)
   {
     if (!currentCommand->Run()) {
       currentCommand->Stop();
       delete currentCommand;
       currentCommand = NULL;
     }
   }
}

