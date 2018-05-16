#ifndef SerialCommand_h
#define SerialCommand_h


class CommandBase
{
public:
  virtual bool Initialize(int dataLength, char *commandData)=0;
  virtual void Start()=0;
  virtual bool Run()=0;
  virtual void Stop();
};

class CmdOff : public CommandBase
{
public:
  bool Initialize(int dataLength, char *commandData);
  void Start();
  bool Run();
};


class CmdThunderstorm : public CommandBase
{
public:
  bool Initialize(int dataLength, char *commandData);
  void Start();
  bool Run();

  bool isPausing;
  unsigned long segmentCutoff;
  unsigned long boltCutoff;

private:
  void CreateStrike();
  void CreatePause();
  void NewSegment();

  byte CenterLED;
  byte Width;
  byte StartLED;
  byte EndLED;

};


class CmdTwinkle : public CommandBase
{
public:
  bool Initialize(int dataLength, char *commandData);
  void Start();
  bool Run();

};


class CmdSolidColor : public CommandBase
{
public:
  bool Initialize(int dataLength, char *commandData);
  void Start();
  bool Run();

private:
  byte redValue;
  byte greenValue;
  byte blueValue;
};


class CmdRainbow : public CommandBase
{
public:
  bool Initialize(int dataLength, char *commandData);
  void Start();
  bool Run();
  
protected:
  byte InitialHue;
};


class CommandParser
{
public:
  CommandParser();
  CommandBase* ProcessCommand(char* command, byte commandLength);
  static byte GetByteVal(char highNybble, char lowNybble);
  static byte GetNybbleVal(char charValue);
};

#endif

