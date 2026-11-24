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


const uint8_t rowLenIn8 = 2;//8 leds/row/matrix
const uint8_t colLen = 24;
uint8_t img[rowLenIn8 * colLen] = {};
uint8_t imgOnlyFalling[rowLenIn8 * colLen] = {};
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
uint32_t score = 0;
uint8_t baseScoreRow = 10;
uint8_t baseScoreFallingToStatic = 1;
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

  pinMode(activateDecoder, OUTPUT);
  for(int i = 0; i < amountOfDecoderPins; i++)
  {
    pinModeFast(decoderPins[i], OUTPUT);
  }
  pinModeFast(upKey, INPUT_PULLUP);
  pinModeFast(leftKey, INPUT_PULLUP);
  pinModeFast(rightKey, INPUT_PULLUP);
  pinModeFast(downKey, INPUT_PULLUP);

  SetupNewRound();
}
void SetupNewRound()
{
  //Read unconnected analog pin to get randomseed.
  pinModeFast(A5, INPUT);
  randomSeed(analogRead(A5));
  UpdateNextTetrimino();
  positionX = 5;
  positionY = 19;

  score = 0;

  for(int i = 0; i < rowLenIn8 * colLen; i++)
  {
    img[i] = 0;
  }
}

void loop()
{
  uint32_t timeNow = millis();
  if(timeNow - timeLastUpdateInMiliseconds >= timeUpdate)
  {
    if(currentTetrimino == 0)
    {
      if(!CheckIfFullRow())
      {
        if(LostGame())
        {
          SetupNewRound();
          return;
        }
        UpdateNextTetrimino();
      }
    }else
    {
      FallBlocks();
    }
    timeLastUpdateInMiliseconds = timeNow;
  }
  HandleInputs();
  PrintScreenWithFallingTetrimino();
}

bool LostGame()
{
  //If any block is up on the spawning row, it's over.
  return ((img[rowLenIn8 * 4] & 0b1111-1111) | (img[rowLenIn8 * 4 + 1] & 0b1100-0000)) != 0;
}

void HandleInputs()
{
  //It's input_pullup, so in the case of 1(True), the gate is open.
  bool rotate = !digitalReadFast(upKey);
  if(rotate)
  {
    if(currentTetrimino == oTetrimino)
    {
      //Nothing
    }else if(currentTetrimino == iTetrimino || currentTetrimino == sTetrimino || currentTetrimino == zTetrimino)
    {
      rotation = (rotation + 1) % 2;
    }else
    {//jTetrimino LTetrimino tTetrimino
      rotation = (rotation + 1) % 4;
    }
  }

  
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
    if(!FallingBlockNotClippingStaticTetriminoesAndNotTooFarRight())
    {
      positionX -= x;
    }
  }
  bool down = !digitalReadFast(downKey);
  if(down)
  {
    FallBlocks();
  }
}

void FallBlocks()
{
  positionY -= 1;
  bool succedd = UpdateImgOnlyFalling();
  if(!succedd || !FallingBlockNotClippingStaticTetriminoesAndNotTooFarRight())//Gone through ground and gone into ground tetriminos respectively.
  {
    positionY += 1;
    UpdateImgOnlyFalling();
    SetFallingTetriminoToStatic();
  }
}

void SetFallingTetriminoToStatic()
{
  for(int i = 0; i < colLen * rowLenIn8; i++)
  {
    img[i] |= imgOnlyFalling[i];
  }
  rotation = 0;
  positionX = 5;
  positionY = 19;
  score += baseScoreFallingToStatic;
}

bool CheckIfFullRow()
{
  uint8_t numRowsCleared = 0;
  for(int row = colLen - 1; row >= 0; row--)
  {
    if(img[row * rowLenIn8] == 0b1111-1111 && img[row * rowLenIn8 + 1] & 0b1100-0000 == 0b1100-0000)
    {
      numRowsCleared++;
      for(int i = row - 1; i >= 0; i--)
      {
        img[i * rowLenIn8    ] = img[i * rowLenIn8 - 2];
        img[i * rowLenIn8 + 1] = (img[i * rowLenIn8 - 1] & 0b1100-0000) | (img[i * rowLenIn8 + 1] & 0b0011-1111);
      }
      img[0] = 0;
      img[1] &= 0b0011-1111;
    }
  }
  if(numRowsCleared > 0)
  {
    score += baseScoreRow * (2 * numRowsCleared - 1);
    return true;
  }
  return false;
}
void UpdateDisplayedScore()
{
  for(int i = 0; i < rowLenIn8 * 2; i++)
  {
    img[i] = 0;
    img[i] |= 0b1111-1111 & score >> (8 * 4 - i);
  }
}

bool FallingBlockNotClippingStaticTetriminoesAndNotTooFarRight()
{
  for(int i = 0; i < rowLenIn8 * 8 * colLen; i++)
  {
    if((i % (rowLenIn8 * 8) > 9 && (imgOnlyFalling[i / 8] & (0b1000-0000 >> (i % 8))) == 1) || (imgOnlyFalling[i] & img[i] != 0))
    {
      return false;
    }
  }
  return true;
}

//True succedes, false fails. AKA: Falling block through ground.
bool UpdateImgOnlyFalling()
{
  for(int i = 0; i < rowLenIn8 * colLen; i++)
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
    if(positionY < 0)
    {
      return false;
    }
    /*
    23 - y since array is top down, while coordinates are bottom up.
    x / 8 since if x is 9 or 10, you need the next array.
    x % 8 since if x is 9 or 10, you start on the next array.
    */
    imgOnlyFalling[(23 - y) * rowLenIn8 + (x / 8)] |= 0b1000-0000 >> (x % 8);
  }
  return true;
}

void UpdateNextTetrimino()
{
  currentTetrimino = nextTetrimino;
  nextTetrimino = GenerateNextTetrimino();
  for(int i = 0; i < 8; i++)
  {
    bool filled = nextTetrimino & (0b1000-0000 >> i);
    int x = 13 + (i % 4) - 2; //since x=14 is 2 more than first bit.
    int y = 1 - (i / 4) + 1; //since y=0 is 1 less than first bit.
    if(!filled)
    {
      img[(23 - y) * rowLenIn8 + (x / 8)] &= 0b0111-1111 >> (x % 8);
    }else
    {
      img[(23 - y) * rowLenIn8 + (x / 8)] |= 0b1000-0000 >> (x % 8);
    }
     
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
  for(int row = 0; row < colLen; row++)
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

  int startByte = row * rowLenIn8;

  for(int i = 0; i < rowLenIn8 * 8; i++)
  {
    digitalWriteFast(serialShiftOut, bitRead(img[startByte], i));
    digitalWriteFast(doShift, HIGH);
    //Wait
    digitalWriteFast(doShift, LOW);
  }
}
