
#pragma once
#include <Arduino.h>
//#include <Adafruit_PCF8574.h>
//#include <Wire.h>

 enum pinType{
  digital = 0,
  analog = 1
 };

class button {
    byte Pin;
	  int Type;
    byte last = 0;
    int debounce (byte last);

  public:
    button(byte pin, int type = pinType::digital)
    {
      Pin = pin;
      Type = type;
      pinMode(Pin, INPUT_PULLUP);
    };

    bool clicked(void);
};
