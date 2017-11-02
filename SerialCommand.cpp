#include "Common.h"

extern CRGBArray<NUM_LEDS> leds;
extern SoftwareSerial mySerial;

void CommandBase::Stop(){
  // Turn off the lights.
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show(); 
}



/* Off command - just a way to turn off the lights */
bool CmdOff::Initialize(int dataLength, char *commandData)
{
  return true;
}

void CmdOff::Start()
{
}

bool CmdOff::Run()
{
  return false;
}


// Format of the thunderstorm command is:
// byte 1: command ID 
// byte 2: Intensity
// byte 3,4: startColorRed
// byte 5,6: startColorGreen
// byte 7,8: startColorBlue
// 
bool CmdThunderstorm::Initialize(int dataLength, char *commandData)
{
  if ((commandData) && (dataLength >= 7)) {
    if (commandData[0] == CMD_STORM) {
       return true;
    }
  }
  
  return false;
}


void CmdThunderstorm::Start() {
  // Pick a starting point
  unsigned long currentMillis = millis();
  segmentCutoff = currentMillis + random(250, 500);
  boltCutoff = currentMillis + random(1500, 3000); 

  NewSegment();
}

bool CmdThunderstorm::Run() {
  unsigned long currentMillis = millis();
  int intensityMultiplier = 255 - (random(0,4) * 32);

  // Check to see if the bolt should quit
  if (currentMillis < boltCutoff) {

    // Check to see if the segment should quit
    if (segmentCutoff < currentMillis) {
      NewSegment();
      segmentCutoff = currentMillis + random(250, 750);
    }
    else {    
      for (int i = 0; i < (Width * 2) + 1; i++)
      {
        leds[(i + StartLED) % NUM_LEDS] = CRGB(intensityMultiplier, intensityMultiplier, intensityMultiplier);
      }
  
      FastLED.show();
    }
    
    return true;
  }
  else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    return false;
  } 
}

void CmdThunderstorm::NewSegment() {
  CenterLED = random(0, NUM_LEDS - 1);
  Width = random(0, 2);
  StartLED = ((CenterLED - Width) < 0) ? (CenterLED - Width) + NUM_LEDS : CenterLED - Width;
  EndLED = (CenterLED + Width) % NUM_LEDS;

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  for (int i = 0; i < (Width * 2) + 1; i++)
  {
    leds[(i + StartLED) % NUM_LEDS] = CRGB::White;
  }
  
  FastLED.show();
}

bool CmdTwinkle::Initialize(int dataLength, char *commandData)
{
  
}

void CmdTwinkle::Start()
{
  // Clear out all of the lights.
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

bool CmdTwinkle::Run()
{
  // Clear out all of the lights.
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  //Randomly pick some LEDs
  int numLEDsToPick = random(0, 4);

  for (int i = 0; i < numLEDsToPick; i++)
  {
    int ledIndex = random(0, NUM_LEDS - 1);
    leds[ledIndex] = CRGB::White;
  }

  FastLED.show();

  return true;
}


bool CmdRainbow::Initialize(int dataLength, char *commandData)
{
  
}

void CmdRainbow::Start()
{
  InitialHue = 0x00;
  
  fill_rainbow(leds, NUM_LEDS, InitialHue, 10);
  FastLED.show();
}

bool CmdRainbow::Run()
{
  InitialHue = InitialHue + 5;
  
  fill_rainbow(leds, NUM_LEDS, InitialHue++, 10);
  FastLED.show();

  return true;
}


bool CmdSolidColor::Initialize(int dataLength, char *commandData)
{
  if ((dataLength >= 7) && (CommandParser::GetNybbleVal(commandData[0]) == CMD_SOLIDCOLOR))
  {
    redValue = CommandParser::GetByteVal(commandData[1], commandData[2]);
    greenValue = CommandParser::GetByteVal(commandData[3], commandData[4]);
    blueValue = CommandParser::GetByteVal(commandData[5], commandData[6]);
    
  }
}

void CmdSolidColor::Start()
{
  // Clear out all of the lights.
  fill_solid(leds, NUM_LEDS, CRGB(redValue, greenValue, blueValue));
  FastLED.show();
}

bool CmdSolidColor::Run()
{
  // We don't have to do anything for a solid color
  return true;
}


CommandParser::CommandParser() {
  CmdBuffer = new char[CMD_BUFFER_SIZE];
  ResetBuffer();
}

/*
 *  char* CmdBuffer;
 *  byte BufferSize;
 *  byte CommandLength;
 *  
 */

CommandBase* CommandParser::ProcessSerialData(int data) {
  
  DEBUG_MESSAGE("Processing data (Hex)");
  DEBUG_MESSAGE_DEC(data)
  DEBUG_MESSAGE_HEX(data)

  if (data < 0)
    return NULL;


  char c = char(data);
  
  // Check to see if we encounter a terminator character.  If so, then 
  // process any data in the buffer, and reset.
  if (c == CMD_TERMINATOR_CHAR) {
    DEBUG_MESSAGE("Processing Command - Length:");
    DEBUG_MESSAGE_DEC(CommandLength);
    
    CommandBase* Result = NULL;

    // If we have a command returned, 
    if (CommandLength > 0) {
      Result = ProcessCommand();
    }

    ResetBuffer();

    // Return the command object if we have one.
    return Result;
  } else if (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))) {
    // Check for the rare condition that the command buffer is overrun
    if (CommandLength >= CMD_BUFFER_SIZE) {

      DEBUG_MESSAGE("Buffer Overrun")
      ResetBuffer();
      
    } else {
      CmdBuffer[CommandLength++] = c;
    }
  } else {
    // Not a hex character, so eat it.
    DEBUG_MESSAGE("Eating Character (Hex):")
    DEBUG_MESSAGE_HEX(c)
  }

  return NULL;
}


void CommandParser::ResetBuffer() {
  
    // Reset the command buffer
    for (byte i = 0; i < CMD_BUFFER_SIZE; i++) {
      CmdBuffer[i] = 0;
    }

    // Reset the command length to 0
    CommandLength = 0;\

    DEBUG_MESSAGE("Buffer Reset");
}

CommandBase* CommandParser::ProcessCommand() {
  DEBUG_MESSAGE("Command Length:");
  DEBUG_MESSAGE_DEC(CommandLength);
  
  if (CommandLength > 0) {
    byte commandType = CommandParser::GetNybbleVal(CmdBuffer[0]);

    DEBUG_MESSAGE("Command Type");
    DEBUG_MESSAGE_HEX(commandType);

    switch (commandType)
    {
      case CMD_OFF:
      {
          CmdOff* OffResult = new CmdOff();
          OffResult->Initialize(CommandLength, CmdBuffer);
          return OffResult;
      }
      case CMD_STORM:
      {
          CmdThunderstorm* StormResult = new CmdThunderstorm();
          StormResult->Initialize(CommandLength, CmdBuffer);
          return StormResult;
      }
      case CMD_TWINKLE:
      {
          CmdTwinkle* TwinkleResult = new CmdTwinkle();
          TwinkleResult->Initialize(CommandLength, CmdBuffer);
          return TwinkleResult;
      }
      case CMD_SOLIDCOLOR:
      {
          CmdSolidColor* SolidColorResult = new CmdSolidColor();
          SolidColorResult->Initialize(CommandLength, CmdBuffer);
          return SolidColorResult;
      }      
      case CMD_RAINBOW:
      {
          CmdRainbow* RainbowResult = new CmdRainbow();
          RainbowResult->Initialize(CommandLength, CmdBuffer);
          return RainbowResult;
      }
    }
  }
}


byte CommandParser::GetByteVal(char highByte, char lowByte)
{
  byte Result = (byte) GetNybbleVal(highByte);
  Result = Result << 4;
  Result = Result | (byte) GetNybbleVal(lowByte);

  return Result;
}


byte CommandParser::GetNybbleVal(char charValue)
{
  switch (charValue)
  {
   case '0':
    return 0;
   case '1':
    return 0x01;
   case '2':
    return 0x02;
   case '3':
    return 0x03;
   case '4':
    return 0x04;
   case '5':
    return 0x05;
   case '6':
    return 0x06;
   case '7':
    return 0x07;
   case '8':
    return 0x08;
   case '9':
    return 0x09;
   case 'A':
   case 'a':
    return 0x0a;
   case 'B':
   case 'b':
    return 0x0b;
   case 'C':
   case 'c':
    return 0x0c;
   case 'D':
   case 'd':
    return 0x0d;
   case 'E':
   case 'e':
    return 0x0e;
   case 'F':
   case 'f':
    return 0x0f;
  }
}

