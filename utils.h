#pragma once

#include <EEPROM.h>
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>


static Adafruit_MCP23X17 mcp;

#include <EncButton.h>
VirtEncoder enc[8];
const VirtEncoder *LeftEnc[3] = {&enc[6], &enc[1], &enc[7]};
const VirtEncoder *RightEnc[3] = {&enc[2], &enc[4], &enc[0]};

int old[8]{0};

int *leftOld[3] = {&old[6], &old[1], &old[7]};
int *rightOld[3] = {&old[2], &old[4], &old[0]};

#define INT_PIN 12

#define NORM_ON 30  // время горения светодиода
#define NORM_OFF 3000 // время потушенного светодиода

static uint16_t onLed = NORM_ON;
static uint16_t offLed = NORM_OFF;

bool savePattern = false;

struct konechnost{
	uint8_t top;
	uint8_t hand;
	uint8_t arm;
	void set(const VirtEncoder **enc)
	{
		top = enc[0]->counter; 
		hand = enc[1]->counter;
		arm = enc[2]->counter;
	}
};


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
const char hexaKeys[ROWS][COLS] = {
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
};

const byte rowPins[ROWS] = {7, 6, 5, 4}; //    input
const byte colPins[COLS] = {11, 10, 9, 8}; //  out




void digitalWriteFast(uint8_t pin, bool x) {
  // раскомментируй, чтобы отключать таймер
  /*switch (pin) {
    case 3: bitClear(TCCR2A, COM2B1);
      break;
    case 5: bitClear(TCCR0A, COM0B1);
      break;
    case 6: bitClear(TCCR0A, COM0A1);
      break;
    case 9: bitClear(TCCR1A, COM1A1);
      break;
    case 10: bitClear(TCCR1A, COM1B1);
      break;
    case 11: bitClear(TCCR2A, COM2A1);   // PWM disable
      break;
    }*/
  if (pin < 8) {
    bitWrite(PORTD, pin, x);
  } else if (pin < 14) {
    bitWrite(PORTB, (pin - 8), x);
  } else if (pin < 20) {
    bitWrite(PORTC, (pin - 14), x);    // Set pin to HIGH / LOW
  }
  return;
}



bool digitalReadFast(uint8_t pin) {
  if (pin < 8) {
    return bitRead(PIND, pin);
  } else if (pin < 14) {
    return bitRead(PINB, pin - 8);
  } else if (pin < 20) {
    return bitRead(PINC, pin - 14);    // Return pin state
  }
}




/*Функция опроса клавиатуры*/
char GetKey4x4()
{
  static unsigned long timer; // для подавления дребезга
  static char olda; //старый код нажатой клавиши
  char a = 0; //код нажатой клавиши
  if ((timer + 50) > millis()) return 0; // пауза для подавления дребезга
  for (byte p = 0; p <= 3; p++) // последовательно выставляем по одной строке в LOW
  {
    digitalWriteFast(colPins[p], LOW);
    for (byte m = 0; m <= 3; m++) // и считываем столбцы, анализируя, где LOW происходит
    {
      if (!digitalReadFast(rowPins[m]))
      {
        a = hexaKeys[p][m]; // считываем соотвествующий символ для комбинации столбца и строки
      }
    }
    digitalWriteFast(colPins[p], HIGH); // возвращем строку в HIGH и крутим дальше
  }
  timer = millis();
  if (a == olda) return 0; //маскируем удержание
  olda = a;
  return a;
}

bool doubleSwitch = 0;



void switchDoubleFunction(int port)
{
	// если выскокий уровень на пине
    if (analogRead(port))
    {
    // если до этого тумблер был выключен
      if (doubleSwitch == 0)
      {
        delay(5);
		// условие антидребезга
        if (analogRead(port))
        {
          doubleSwitch = 1;  // тумблер считается включенным
//          Serial.println("SwitchON");
		  onLed = 100;
		  offLed = 100;
		  savePattern = true;
        }
      }
    }
    else
    {
      if (doubleSwitch == 1)
      {
        delay(5);
        if (!analogRead(port))
        {
          doubleSwitch = 0;
 //         Serial.println("SwitchOFF");
		  onLed = NORM_ON;
		  offLed = NORM_OFF;
		  savePattern = false;
        }
      }
    }
}

byte pin;


void readMCU(void)
{
  if (!digitalReadFast(INT_PIN)) {
    pin = mcp.getLastInterruptPin();
    // проверка на четность
    const bool isOddPin = pin & 1;
    if (isOddPin == 0) {
      //деление на 2
      enc[pin >> 1].tickISR(mcp.digitalRead(pin + 1), mcp.digitalRead(pin));
    } else {
      // деление на 2
      enc[pin >> 1].tickISR(mcp.digitalRead(pin), mcp.digitalRead(pin - 1));
    }
    //    Serial.println(enc[0].counter);
    mcp.clearInterrupts();  // clear
  }


}

void blink(uint8_t led, uint16_t on, uint16_t off)
{
	static uint32_t t = 0;
	static bool ledStatus = LOW;
	if(millis()-t>(on))
	{
		if(ledStatus==HIGH)
		{
			t = millis();
			ledStatus = LOW;
			digitalWriteFast(led, ledStatus);
	    }
	}
	
	
	if(millis()-t>off)
	{
		if(ledStatus==LOW)
		{
			t = millis();
			ledStatus = HIGH;
			digitalWriteFast(led, ledStatus);
	    }
	}
}

bool isLeft(uint8_t ch)
{
	return (ch=='4'||ch=='5'||ch=='7'||ch=='8'||ch=='*'||ch=='0');
	
}

bool isRight(uint8_t ch)
{
	return (ch=='6'||ch=='B'||ch=='9'||ch=='C'||ch=='#'||ch=='D');
}

void myPattern(uint8_t num, konechnost &arm)
{
	// для левой руки
	if(isLeft(num))
	{
	    String outPut = "";
        enc[6].counter = arm.top;
        outPut = "F" + String(enc[6].counter) + ":";
        Serial.print(outPut);
        enc[1].counter = arm.hand;
        outPut = "A" + String(enc[1].counter) + ":";
        Serial.print(outPut);
        enc[7].counter = arm.arm;
        outPut = "B" + String(enc[7].counter) + ":";
        Serial.print(outPut);
	}
	else if(isRight(num))
	{
	    String outPut = "";
        enc[2].counter = arm.top;
        outPut = "E" + String(enc[2].counter) + ":";
        Serial.print(outPut);
        enc[4].counter = arm.hand;
        outPut = "C" + String(enc[4].counter) + ":";
        Serial.print(outPut);
        enc[0].counter = arm.arm;
        outPut = "D" + String(enc[0].counter) + ":";
        Serial.print(outPut);
	}
}

konechnost readEEPROM(const uint8_t addr)
{
	uint16_t newAddr = addr*3;
	konechnost arm;
    if(newAddr > EEPROM.length())
	{
		Serial.println("EEPROM overflow");
		return arm;
	}
	arm.top = EEPROM.read(newAddr);
	delay(5);
	arm.hand = EEPROM.read(newAddr+1);
	delay(5);
    arm.arm = EEPROM.read(newAddr+2);
    delay(5);  	
	return arm;
}

void writeEEPROM(const konechnost &arm, const uint8_t addr)
{
	uint16_t newAddr = addr * 3;
  	EEPROM.write(newAddr,     arm.top);
	delay(5);
  	EEPROM.write(newAddr + 1, arm.hand);
	delay(5);
  	EEPROM.write(newAddr + 2, arm.arm);
	delay(5);
	return;
}

void selectPattern(konechnost side, const uint8_t choise)
{
    if(savePattern)
	{
        if(isLeft(choise)) side.set(LeftEnc);
        else if(isRight(choise)) side.set(RightEnc);
		writeEEPROM(side, choise);
	}else{
	  side = readEEPROM(choise);
//	      left.set(45,90,45);
      myPattern(choise, side);
	}
}
 