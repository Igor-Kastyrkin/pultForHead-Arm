
#include "button.h"

int button::debounce(byte last)
    {
//      Serial.println("debounce");
      byte current;
      if(Type == 0) current = digitalRead(Pin);
	  else if(Type == 1){
	    current = analogRead(Pin)/1000;
	  }
      if (last != current) //если состояние изменилось
      {
        delay(5);
		if(Type == 0)  current = digitalRead(Pin);
		else if(Type == 1)
		{
		  current = analogRead(Pin)/1000;
		}
      }
      return current;
    }

    bool button::clicked(void)
    {

//      Serial.println("clicked");
      bool curButton = debounce (last);
//      Serial.println(curButton, HEX);
//      Serial.println(last, HEX);
//      Serial.println("  ");
//      delay(560);
      bool isBtnClick = 0;
      if (last == HIGH && curButton == LOW) //условие нажатия
      {
        isBtnClick = 1;
      }
      last = curButton;
      return isBtnClick;
    }
