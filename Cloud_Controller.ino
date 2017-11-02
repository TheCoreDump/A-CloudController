#include "Common.h"

// LED Array
CRGBArray<NUM_LEDS> leds;

// Currently Running Command
volatile CommandBase* currentCommand;

// Command Parser
CommandParser* mainCommandParser;

// Character buffer for commands
char CommandBuffer[CMD_BUFFER_SIZE];


// Initialize the software serial port
#ifndef RX_USB_SERIAL_PORT

  SoftwareSerial BlueToothSerial(10, 11); // RX, TX
  
#endif


void setup() {

  // Setup the serial communications
  SerialSetup();
  
  // Set timer period
  Timer1.initialize(TIMER_PERIOD_MICRO_SECONDS); 
  Timer1.attachInterrupt(onTimerFire);

  // Set up the command processor
  mainCommandParser = new CommandParser();


  // Clear the command buffer
  ClearCommandBuffer();
  

  // Initialize the LED library
  FastLED.addLeds<WS2812,3,RGB>(leds, NUM_LEDS).setCorrection(LEDColorCorrection::Typical8mmPixel);


  // Flash the lights so we know that the controller has been reset
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


// Main application loop.  This basically checks for commands on the serial port and 
// processes the commands as they are recieved
void loop() {

  CommandBase* parsedCommand = NULL;

  byte commandLength = ReadCommandLine();

  if (commandLength > 0)
  {
    DEBUG_MESSAGE("Data Read");
    DEBUG_MESSAGE(CommandBuffer);
    
    parsedCommand = mainCommandParser->ProcessCommand(CommandBuffer, commandLength);

    if (parsedCommand) {
      DEBUG_MESSAGE("Command Parsed");

      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

        if (currentCommand) {
          DEBUG_MESSAGE("Stopping Current Command");
          currentCommand->Stop();
          delete currentCommand;
          currentCommand = NULL;
        }

        DEBUG_MESSAGE("Starting New Command");
        currentCommand = parsedCommand;
        currentCommand->Start();
      }
    }
  }

  ClearCommandBuffer();
}


void SerialSetup() {

 #ifndef RX_USB_SERIAL_PORT

  BlueToothSerial.begin(9600);

  #ifdef DEBUGGING_MODE

   Serial.begin(9600);

  #endif

 #else

  Serial.begin(9600);
  
 #endif
  
}

byte ReadCommandLine()
{
 #ifndef RX_USB_SERIAL_PORT
   return BlueToothSerial.readBytesUntil('\n', CommandBuffer, CMD_BUFFER_SIZE);
 #else
   return Serial.readBytesUntil('\n', CommandBuffer, CMD_BUFFER_SIZE);
 #endif
}


void ClearCommandBuffer() {
  
    // Reset the command buffer
    for (byte i = 0; i < CMD_BUFFER_SIZE; i++) {
      CommandBuffer[i] = 0x00;
    }

    DEBUG_MESSAGE("Buffer Reset");
}



void onTimerFire() {
   if (currentCommand)
   {
     if (!currentCommand->Run()) {
       currentCommand->Stop();
       delete currentCommand;
       currentCommand = NULL;
     }
   }
}

