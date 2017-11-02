#ifndef CloudCommon_h
#define CloudCommon_h

#define DEBUGGING_MODE
//#define USE_USB_SERIAL_PORT

#ifdef DEBUGGING_MODE

#define DEBUG_MESSAGE(MSG) { Serial.println(MSG); }
#define DEBUG_MESSAGE_DEC(MSG) { Serial.println(MSG, DEC); }
#define DEBUG_MESSAGE_HEX(MSG) { Serial.println(MSG, HEX); }

#else

#define DEBUG_MESSAGE(MSG) 
#define DEBUG_MESSAGE_DEC(MSG) 
#define DEBUG_MESSAGE_HEX(MSG) 

#endif


// Library Includes
#include <FastLED.h>
#include <SoftwareSerial.h>
#include "SerialCommand.h"

// Command Constants
#define CMD_BUFFER_SIZE 128
#define CMD_TERMINATOR_CHAR '@'

#define CMD_OFF 0
#define CMD_STORM 1
#define CMD_TWINKLE 2
#define CMD_SOLIDCOLOR 3
#define CMD_RAINBOW 4


// LED constants
#define NUM_LEDS 25

#endif

