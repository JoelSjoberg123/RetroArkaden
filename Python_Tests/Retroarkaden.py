rowLenIn8 = 2#8 leds/row/matrix
colLen = 24

imgOnlyFalling = [0]
for i in range(rowLenIn8 * colLen - 1):
  imgOnlyFalling.append(0)

iTetrimino = 0b0000_1111
oTetrimino = 0b0110_0110
jTetrimino = 0b0100_0111
LTetrimino = 0b0001_0111
sTetrimino = 0b0110_0011
zTetrimino = 0b0011_0110
tTetrimino = 0b0010_0111


#Tetrimino information
currentTetrimino = 0
nextTetrimino = 0
positionX = 5 #Bottom left is origin. Due to where the axis is of the tetriminos, x is never 0 when one exists.
positionY = 7 #Position is of the axis of rotation.
rotation  = 0 #Rotation is around the axis of rotation, going counter-clockwise.


def UpdateImgOnlyFalling():
  for i in range(rowLenIn8 * colLen):
    imgOnlyFalling[i] = 0
  

  #Goes through currentTetrimino which is 8 bits.
  for i in range(8):  
    filled = currentTetrimino & (0b1000_0000 >> i)
    if not filled:
        continue
    x = 0
    y = 0
    if rotation == 0:
      x = positionX + (i % 4) - 2 #since positionX is 2 more than first bit.
      y = positionY - (i // 4) + 1 #since positionY is 1 less than first bit.
    elif rotation == 1: 
      x = positionX + (i // 4) - 1 #since positionX is 1 more than first bit.
      y = positionY + (i % 4) - 2 #since positionY is 2 more than first bit.
    elif rotation == 2: 
      x = positionX - (i % 4) + 2 #since positionX is 2 less than first bit.
      y = positionY + (i // 4) - 1 #since positionY is 1 more than first bit.
    else:
      x = positionX - (i // 4) + 1 #since positionX is 1 less than first bit.
      y = positionY - (i % 4) + 2 #since positionY is 2 less than first bit.
    
    if positionY < 0:
    
      return False
  
    imgOnlyFalling[(23-y) * rowLenIn8 + (x // 8)] |= 0b1000_0000 >> (x % 8)
  
  return True
def PrintScreenWithFallingTetrimino():
  for i in range(colLen):
    print(f"{colLen - i}\t", sep = "", end = "")
    first = str(bin(imgOnlyFalling[i * rowLenIn8])[2:])
    for j in range(8 - len(first)):
      first = "0" + first

    second = str(bin(imgOnlyFalling[i * rowLenIn8 + 1])[2:])
    for j in range(8 - len(second)):
      second = "0" + second
    print(first, second, sep = "")


while True:
  currInput = input("What tetrimino?: ")
  match currInput:
    case "i":
        currentTetrimino = iTetrimino
    case "o":
        currentTetrimino = oTetrimino
    case "j":
        currentTetrimino = jTetrimino
    case "l":
        currentTetrimino = LTetrimino
    case "s":
        currentTetrimino = sTetrimino
    case "z":
        currentTetrimino = zTetrimino
    case "t":
        currentTetrimino = tTetrimino
    case _:
        continue

  positionX = int(input("posistionX = "))
  positionY = int(input("posistionY = "))
  rotation = int(input("rotation = "))
  UpdateImgOnlyFalling()
  PrintScreenWithFallingTetrimino()
  if input("quit? y/n ") == "y":
     break