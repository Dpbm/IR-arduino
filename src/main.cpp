#include <Arduino.h>
#include <IRremote.hpp>
#include <LiquidCrystal.h>

#define RECV_PIN 4
#define SEND_PIN 5
#define RECV_BUTTON 2
#define SEND_BUTTON 3

#define RS 7
#define E 12
#define D4 11
#define D5 10
#define D6 9
#define D7 8

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

typedef enum
{
  none,
  read,
  send
} State;

State actualState = none;

#define MIN_FREQUENCY 20
#define MAX_FREQUENCY 75
#define FREQUENCY_STEP 1

void getCode();
void goRead();
void goSend();
void sendCodes();
void showLCDMenu();
void showFunction(char *text);

void setup()
{
  Serial.begin(9600);

  lcd.begin(16, 2);
  showLCDMenu();

  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK, LED_BUILTIN);
  IrSender.begin(SEND_PIN);

  pinMode(RECV_BUTTON, INPUT);
  pinMode(SEND_BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(RECV_BUTTON), goRead, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SEND_BUTTON), goSend, CHANGE);
}

void loop()
{
  if (actualState == read)
    getCode();
  else if (actualState == send)
    sendCodes();
}

void getCode()
{

  if (IrReceiver.decode())
  {
    Serial.println();
    IrReceiver.printIRResultShort(&Serial);

    IRData decodedData = IrReceiver.decodedIRData;

    Serial.print("Protocol: ");
    Serial.println(decodedData.protocol);
    Serial.print("Address: ");
    Serial.println(decodedData.address, HEX);
    Serial.print("Command: ");
    Serial.println(decodedData.command, HEX);
    Serial.print("Extra: ");
    Serial.println(decodedData.extra, HEX);
    Serial.print("Raw Data: ");
    Serial.println(decodedData.decodedRawData, HEX);
    Serial.print("Distance Width Timing Info HeaderMarkMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.HeaderMarkMicros);
    Serial.print("Distance Width Timing Info HeaderSpaceMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.HeaderSpaceMicros);
    Serial.print("Distance Width Timing Info OneMarkMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.OneMarkMicros);
    Serial.print("Distance Width Timing Info OneSpaceMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.OneSpaceMicros);
    Serial.print("Distance Width Timing Info ZeroMarkMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.ZeroMarkMicros);
    Serial.print("Distance Width Timing Info ZeroSpaceMicros: ");
    Serial.println(decodedData.DistanceWidthTimingInfo.ZeroSpaceMicros);
    Serial.print("Raw Data Array: ");
    for (int i = 0; i < RAW_DATA_ARRAY_SIZE; i++)
    {
      Serial.print(decodedData.decodedRawDataArray[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Number of Bits: ");
    Serial.println(decodedData.numberOfBits);
    Serial.print("flags: ");
    Serial.println(decodedData.flags);
    Serial.print("Recieve Pin: ");
    Serial.println(decodedData.rawDataPtr->IRReceivePin);
    Serial.print("State for ISR: ");
    Serial.println(decodedData.rawDataPtr->StateForISR);
    Serial.print("Pin Mask: ");
    Serial.println(decodedData.rawDataPtr->IRReceivePinMask);
    Serial.print("Overflow flag: ");
    Serial.println(decodedData.rawDataPtr->OverflowFlag);
    Serial.print("Raw Length: ");
    Serial.println(decodedData.rawDataPtr->rawlen);
    Serial.print("Raw Buffer: ");
    for (int i = 0; i < decodedData.rawDataPtr->rawlen; i++)
    {
      Serial.print(decodedData.rawDataPtr->rawbuf[i]);
      Serial.print(" ");
    }
    Serial.println();

    lcd.clear();
    lcd.print("results: ");
    lcd.setCursor(0, 1);
    lcd.print(IrReceiver.decodedIRData.protocol);
    lcd.print(" 0x");
    lcd.print(IrReceiver.decodedIRData.address, HEX);
    lcd.print(" 0x");
    lcd.print(IrReceiver.decodedIRData.command, HEX);

    IrReceiver.resume();
  }
}

void goRead()
{
  if (actualState != read)
  {
    Serial.println("Start reading...");
    showFunction("Reading...");
    actualState = read;
    return;
  }

  showLCDMenu();
  Serial.println("Back to None");
  actualState = none;
}

void goSend()
{
  if (actualState != send)
  {
    Serial.println("Start sending...");
    showFunction("Sending...");
    actualState = send;
    return;
  }

  showLCDMenu();
  Serial.println("Back to None");
  actualState = none;
}

void sendCodes()
{
  IRData decodedData = IrReceiver.decodedIRData;
  int numberOfRepeats = 1;
  int numberOfRepeatsNEC = -1;
  // int deviceId = ; for apple
  // int vendorCode = ; for sendKaseikyo
  bool sharp = false;
  bool autoToggle = true;
  bool repeat = true;

  Serial.print("Adresss: ");
  Serial.println(decodedData.address, HEX);

  switch (decodedData.protocol)
  {
  case 4:
    IrSender.sendDenon(decodedData.address, decodedData.command, numberOfRepeats, sharp);
    break;

  case 5:
    IrSender.sendJVC(decodedData.decodedRawData, decodedData.numberOfBits, repeat);
    break;

  case 6:
    IrSender.sendLG(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 7:
    IrSender.sendLG2(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 8:
    IrSender.sendNEC(decodedData.address, decodedData.command, numberOfRepeatsNEC);
    break;

  case 9:
    IrSender.sendNEC2(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 10:
    IrSender.sendOnkyo(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 11:
    IrSender.sendPanasonic(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 13:
    IrSender.sendKaseikyo_Denon(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 14:
    IrSender.sendKaseikyo_Sharp(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 15:
    IrSender.sendKaseikyo_JVC(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 16:
    IrSender.sendKaseikyo_Mitsubishi(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 17:
    IrSender.sendRC5(decodedData.address, decodedData.command, numberOfRepeats, autoToggle);
    break;

  case 18:
    IrSender.sendRC6(decodedData.address, decodedData.command, numberOfRepeats, autoToggle);
    break;

  case 19:
    IrSender.sendSamsung(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 20:
    IrSender.sendSamsung48(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 21:
    IrSender.sendSamsungLG(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 22:
    IrSender.sendSharp(decodedData.address, decodedData.command, numberOfRepeats);
    break;

  case 23:
    IrSender.sendSony(decodedData.address, decodedData.command, numberOfRepeats, decodedData.numberOfBits);
    break;

  case 25:
    IrSender.sendBoseWave(decodedData.command, numberOfRepeats);
    break;

  case 29:
    IrSender.sendFAST(decodedData.command, numberOfRepeats);
    break;

  default:
    int millisecondsPeriod = decodedData.rawDataPtr->rawbuf[0] * MICROS_PER_TICK * MICROS_IN_ONE_MILLI;
    for (int frequency = MIN_FREQUENCY; frequency <= MAX_FREQUENCY; frequency += FREQUENCY_STEP)
    {
      Serial.print("Testing frequency: ");
      Serial.print(frequency);
      Serial.println("KHz");
      IrSender.sendPulseDistanceWidthFromArray(frequency, &decodedData.DistanceWidthTimingInfo, decodedData.decodedRawDataArray, decodedData.numberOfBits, decodedData.flags, millisecondsPeriod, (int_fast8_t)numberOfRepeats);
    }
    break;
  }
}

void showLCDMenu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Click on button");
}

void showFunction(char *text)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
}