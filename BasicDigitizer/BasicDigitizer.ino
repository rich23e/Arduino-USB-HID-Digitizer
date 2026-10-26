#include <Digitizer.h>

void setup() { Digitizer.begin(); }

void loop()
{
  Digitizer.press(16384, 16384);
  delay(500);
  Digitizer.release();
  delay(2500);
}
