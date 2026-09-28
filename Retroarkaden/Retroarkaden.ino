/*
img will be outputed to the monitor.
Will use shift register, get a pattern per row.
Will use decoder, get column  to activate.


*/
//Both are digital.
#define doShift 2
#define serialShiftOut 4

const int rowLen = 16;
const int colLen = 16;
bool img[rowLen * colLen] = {};


void setup()
{
  pinMode(doShift, OUTPUT);
  pinMode(serialShiftOut, OUTPUT);
  digitalWrite(doShift, LOW)
  for(int i = 0; i < rowLen * colLen; i++)
  {
    img[i] = 0;
  }
}

void loop()
{
  

}

void ShiftInOneRow(int row)
{
  //E.g: row = 0, start bit 0. row = 1, start bit 16(16 on first row, from 0 to 15).

  int startBit = row * colLen;

  for(int i = 0; i < colLen; i++)
  {
    digitalWrite(serialShiftOut, img[startBit + i]);
    digitalWrite(doShift, HIGH);
    //Maybe wait
    digitalWrite(doShift, LOW);
  }
}


