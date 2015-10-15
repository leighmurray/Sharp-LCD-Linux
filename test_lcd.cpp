#include "SharpLCD.hpp"
#include <iostream>

SharpLCD lcd;

int main(void) {
  int numbers = 12;
  uint8_t line[numbers];

  for (int count = 0; count < numbers; count++)
  {
    line[count] = 0b00001111;
  }

  double interval = 0.01;

  while (true) {
    lcd.clearScreen();
    sleep(interval);
    lcd.writeLine(0x0A, line);
    sleep(interval);
  }
  return 0;
}
