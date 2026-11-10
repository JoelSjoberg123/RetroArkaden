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

#define upKey A0
#define leftKey A1
#define rightKey A2
#define downKey A3
//Timing of the game
uint32_t timeLastUpdateInMiliseconds = 0;
uint16_t timeUpdate = 1000;
//The tetriminos

/*
Only needs two layers, upper and lower are always 0000. Example:
uint16_t iTerimino  = 0b0000-0000-1111-0000;
The second last bit will always be the axis of rotation.
*/
const uint8_t iTerimino  = 0b0000-1111;
const uint8_t oTetrimino = 0b0110-0110;
const uint8_t jTetrimino = 0b0100-0111;
const uint8_t LTetrimino = 0b0001-0111;
const uint8_t sTetrimino = 0b0110-0011;
const uint8_t zTetrimino = 0b0011-0110;
const uint8_t tTetrimino = 0b0010-0111;

//Falling tetrimino
uint8_t positionX = 0;//Bottom left is origin.
uint8_t positionY = 0;//Position is of the axis of rotation.
uint8_t rotation  = 0;


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
  pinModeFast(upKey, INPUT_PULLUP);
  pinModeFast(leftKey, INPUT_PULLUP);
  pinModeFast(rightKey, INPUT_PULLUP);
  pinModeFast(downKey, INPUT_PULLUP);
}

void loop()
{
  uint32_t timeNow = millis();
  if(timeNow - timeLastUpdateInMiliseconds >= timeUpdate)
  {
    //FallBlocks();
    timeLastUpdateInMiliseconds = timeNow;
  }


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
