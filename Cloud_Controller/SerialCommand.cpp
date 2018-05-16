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
  CreateStrike();
}

void CmdThunderstorm::CreateStrike() {
  unsigned long currentMillis = millis();
  segmentCutoff = currentMillis + random(200, 500);
  boltCutoff = currentMillis + random(1000, 3000); 
  isPausing = false;
  NewSegment();
}

void CmdThunderstorm::CreatePause() {
  unsigned long currentMillis = millis();
  boltCutoff = currentMillis + random(1500, 5000); 
  isPausing = true;
}


bool CmdThunderstorm::Run() {
  unsigned long currentMillis = millis();
  bool beforeBoltCutoff = currentMillis < boltCutoff;
  bool beforeSegmentCutoff = currentMillis < segmentCutoff;

  if (isPausing) {
    if (beforeBoltCutoff) {
      //Do nothing - waiting for the next strike
      return true;
    } else {
      CreateStrike();
      return true; 
    }    
  } else {
    int intensityMultiplier = 255 - (random(0,4) * 32);

    // Check to see if the bolt should quit
    if (beforeBoltCutoff) {

      // Check to see if the segment should quit
      if (!beforeSegmentCutoff) {
        NewSegment();
        segmentCutoff = currentMillis + random(250, 750);
      } else {    
        for (int i = 0; i < (Width * 2) + 1; i++)
        {
          leds[(i + StartLED) % NUM_LEDS] = CRGB(intensityMultiplier, intensityMultiplier, intensityMultiplier);
        }
  
        FastLED.show();
      }
    
      return true;
    } else {
      // Current Strike is done, pause until the next strike
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();

      CreatePause();
      
      return true;
    } 
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
}


CommandBase* CommandParser::ProcessCommand(char* command, byte commandLength) {
  DEBUG_MESSAGE("Command Length:");
  DEBUG_MESSAGE_DEC(commandLength);
  
  if (commandLength > 0) {
    byte commandType = CommandParser::GetNybbleVal(command[0]);

    DEBUG_MESSAGE("Command Type");
    DEBUG_MESSAGE_HEX(commandType);

    switch (commandType)
    {
      case CMD_OFF:
      {
          CmdOff* OffResult = new CmdOff();
          OffResult->Initialize(commandLength, command);
          return OffResult;
      }
      case CMD_STORM:
      {
          CmdThunderstorm* StormResult = new CmdThunderstorm();
          StormResult->Initialize(commandLength, command);
          return StormResult;
      }
      case CMD_TWINKLE:
      {
          CmdTwinkle* TwinkleResult = new CmdTwinkle();
          TwinkleResult->Initialize(commandLength, command);
          return TwinkleResult;
      }
      case CMD_SOLIDCOLOR:
      {
          CmdSolidColor* SolidColorResult = new CmdSolidColor();
          SolidColorResult->Initialize(commandLength, command);
          return SolidColorResult;
      }      
      case CMD_RAINBOW:
      {
          CmdRainbow* RainbowResult = new CmdRainbow();
          RainbowResult->Initialize(commandLength, command);
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

