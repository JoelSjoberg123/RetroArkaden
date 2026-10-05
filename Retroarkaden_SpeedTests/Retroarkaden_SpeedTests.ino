#include <digitalWriteFast.h>
/*
img will be outputed to the monitor.
Will use shift register, get a pattern per row.
Will use decoder, get column to activate.

Note:
Decoder is standard high, so it will have to be on the negative side of the dot matrix.

*/
//Both are digital.
#define doShift 2
#define serialShiftOut 4


const uint8_t rowLen = 16;
const uint8_t colLen = 24;
bool img[rowLen * colLen] = {};
/*
Could make the array an array of uint8_t, where one uint8_t is one row of one dot matrix.
Probably better than one uint16_t, since the processor is an 8 bit processor.
*/
//5:MSb     8:LSb
const uint8_t amountOfDecoderPins = 5;
const uint8_t decoderPins[amountOfDecoderPins] = {5,6,7,8,9};
#define activateDecoder 10


void setup()
{
  Serial.begin(9600);
  pinModeFast(doShift, OUTPUT);
  pinModeFast(serialShiftOut, OUTPUT);
  digitalWriteFast(doShift, LOW);
  for(int i = 0; i < rowLen * colLen; i++)
  {
    img[i] = i % 2;
  }

  pinMode(activateDecoder, OUTPUT);
  for(int i = 0; i < amountOfDecoderPins; i++)
  {
    pinModeFast(decoderPins[i], OUTPUT);
  }
  
}

void loop()
{
  
  int time01 = micros();
  for(int row = 0; row < rowLen; row++)
  {
    ShiftInOneRow(row);
  }
  int time02 = micros();
  for(int row = 0; row < rowLen; row++)
  {
    for(int i = 0; i < amountOfDecoderPins; i++)
    {
      digitalWrite(decoderPins[i], bitRead(row, i));
    }
  }
  int time03 = micros();
  PrintScreen();
  int time04 = micros();
  Serial.print(time02 - time01);
  Serial.println("μs to shift in rows.");
  Serial.print(time03 - time02);
  Serial.println("μs to activate rows.");
  Serial.print(time04 - time03);
  Serial.println("μs to print screen.");
  delay(10000);
}

void PrintScreen()
{
  for(int row = 0; row < rowLen; row++)
  {
    PrintRow(row);
  }
}

void PrintRow(uint8_t row)
{
  ShiftInOneRow(row);
  for(int i = 0; i < amountOfDecoderPins; i++)
  {
    digitalWriteFast(decoderPins[i], bitRead(row, i));
  }

  digitalWriteFast(activateDecoder, HIGH);
  //Wait
  digitalWriteFast(activateDecoder, LOW);
}

void ShiftInOneRow(uint8_t row)
{
  //E.g: row = 0, start bit 0. row = 1, start bit 16(16 on first row, from 0 to 15).

  int startBit = row * rowLen;

  for(int i = 0; i < colLen; i++)
  {
    digitalWriteFast(serialShiftOut, img[startBit + i]);
    digitalWriteFast(doShift, HIGH);
    //Wait
    digitalWriteFast(doShift, LOW);
  }
}
