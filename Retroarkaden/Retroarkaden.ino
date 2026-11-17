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
bool imgOnlyFalling[rowLen * colLen] = {};
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
const uint8_t iTetrimino = 0b0000-1111;
const uint8_t oTetrimino = 0b0110-0110;
const uint8_t jTetrimino = 0b0100-0111;
const uint8_t LTetrimino = 0b0001-0111;
const uint8_t sTetrimino = 0b0110-0011;
const uint8_t zTetrimino = 0b0011-0110;
const uint8_t tTetrimino = 0b0010-0111;

//Tetrimino information
uint8_t currentTetrimino = 0;
uint8_t nextTetrimino = 0;
uint8_t positionX = 0; //Bottom left is origin. Due to where the axis is of the tetriminos, x is never 0 when one exists.
uint8_t positionY = 0; //Position is of the axis of rotation.
uint8_t rotation  = 0; //Rotation is around the axis of rotation, going counter-clockwise.


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

  //Read unconnected analog pin to get randomseed.
  pinModeFast(A5, INPUT);
  randomSeed(analogRead(A5));
  nextTetrimino = GenerateNextTetrimino();
}

void loop()
{
  uint32_t timeNow = millis();
  if(timeNow - timeLastUpdateInMiliseconds >= timeUpdate)
  {
    if(currentTetrimino == 0)
    {
      currentTetrimino = nextTetrimino;
      nextTetrimino = GenerateNextTetrimino();
    }else
    {
      FallBlocks();
    }
    timeLastUpdateInMiliseconds = timeNow;
  }
  HandleInputs();
  PrintScreenWithFallingTetrimino();
}

void HandleInputs()
{
  //Do rotation

  //It's input_pullup, so 1(True) is off.
  bool left  = !digitalReadFast(leftKey);
  bool right = !digitalReadFast(rightKey);
  if(!(left && right))
  {
    int x = 0;
    if(left)
    {
      x = -1;
    }
    else if(right)
    {
      x = 1;
    }
    positionX += x;
    UpdateImgOnlyFalling();
    if(true/*CheckIfWorks()*/)
    {
      positionX -= x;
    }
  }
  bool down = !digitalReadFast(downKey);
  if(down)
  {
    positionY -= 1;
    //Add checks if stopped.
  }
}

void FallBlocks()
{
  
}

void UpdateImgOnlyFalling()
{
  for(int i = 0; i < rowLen * colLen; i++)
  {
    imgOnlyFalling[i] = 0;
  }

  //Goes through currentTetrimino which is 8 bits.
  for(int i = 0; i < 8; i++)
  {
    bool filled = currentTetrimino & (0b1000-0000 >> i);
    if(!filled){continue;}
    uint8_t x = 0;
    uint8_t y = 0;
    if(rotation == 0)
    {
      x = positionX + (i % 4) - 2; //since positionX is 2 more than first bit.
      y = positionY - (i / 4) + 1; //since positionY is 1 less than first bit.
    }
    else if (rotation == 1) {
      x = positionX + (i / 4) - 1; //since positionX is 1 more than first bit.
      y = positionY + (i % 4) - 2; //since positionY is 2 more than first bit.
    }
    else if (rotation == 2) {
      x = positionX - (i % 4) + 2; //since positionX is 2 less than first bit.
      y = positionY + (i / 4) - 1; //since positionY is 1 more than first bit.
    }
    else
    {
      x = positionX - (i / 4) + 1; //since positionX is 1 less than first bit.
      y = positionY - (i % 4) + 2; //since positionY is 2 less than first bit.
    }

    imgOnlyFalling[x + y * rowLen] = 1;
  }
}

uint8_t GenerateNextTetrimino()
{
  switch (random(7)) {
    case 0:
      return iTetrimino;
      break;
    case 1:
      return oTetrimino;
      break;
    case 2:
      return jTetrimino;
      break;
    case 3:
      return LTetrimino;
      break;
    case 4:
      return sTetrimino;
      break;
    case 5:
      return zTetrimino;
      break;
    case 6:
      return tTetrimino;
      break;
  }
}

void PrintScreenWithFallingTetrimino()
{
  //Add falling tetrimino to image.
  PrintScreen();
  //Remove falling tetrimino from image.
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
