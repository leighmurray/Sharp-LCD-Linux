#include "SharpLCD.hpp"
#include <iostream>

SharpLCD lcd;

int main(void) {
  int numbers = 12;
  uint8_t line[numbers];

  for (int count = 0; count < numbers; count++)
  {
    line[count] = count;
  }

  while (true) {
    lcd.clearScreen();
    usleep(1000);

    lcd.writeLine(0xAA, line);
    usleep(1000);
  }
  return 0;
}
