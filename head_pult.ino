
#include <AdafruitI2CIOHandler.h>
#include "utils.h"

// подключаем "внешний" энкодер, для работы с расширителями пинов например


#define DT1       0
#define CLK1      1

#define DT2       2
#define CLK2      3

#define DT3       4
#define CLK3      5

#define DT        6
#define CLK       7

#define DT4       8
#define CLK4      9

#define DT5       10
#define CLK5      11

#define DT6       12
#define CLK6      13

#define DT7       14
#define CLK7      15

#define LED       13

#include "Button.h"



//button rec(A1, pinType::analog);
button play(3);
button rec(2);

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  for (int i = 0; i < 8; i++)
  {
    enc[i].setEncType(EB_STEP4_LOW);
  }
  if (!mcp.begin_I2C()) {
    //   Serial.println("Error.");
    while (1)
    {
      blink(LED, 200, 200);
    }
  }
  // ускоряю обмен, чтобы енкодеры можно было крутить быстрее
  Wire.setClock(400000L);

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);

  pinMode(INT_PIN, INPUT);

  mcp.setupInterrupts(true, false, LOW);
  for (int i = 0; i < 16; i++) {
    mcp.setupInterruptPin(i, CHANGE);
  }

  for (int i = 0; i < 8; i++)
  {
    enc[i].setEncISR(true);
  }

  //  attachInterrupt(INT_PIN, ISR_H, CHANGE);

  for (int i = 0; i <= 3; i++) // настройка выходов и входов
  {
    pinMode(colPins[i], OUTPUT);
    pinMode(rowPins[i], INPUT_PULLUP); //входы с внутренней подтяжкой
    digitalWrite(colPins[i], HIGH);
  }

  enc[1].setEncReverse(true);
  enc[2].setEncReverse(true);
  enc[7].setEncReverse(true);
  
//  VirtEncoder left1[3] = {enc[1], enc[2], enc[7]};


  Serial.println(".");
}



//int old[16] = {0};

int moveSpeed = 8;

void loop() {
  switchDoubleFunction(A2); // для двух рук?
//  static unsigned long mil1 = 0; // для проверки скорости
  static unsigned long mil2 = 0; // для не частой передачи данных по bluetooth
  static bool recMode = 0;
  // опрос расширителя порта
  readMCU();

  const int resist = analogRead(A0);
  moveSpeed = map(resist, 0, 1024, 1, 20);

  for (int i = 0; i < 8; i++)
  {
    enc[i].tick();
  }

  if (millis() - mil2 > 40)
  {

    for (int i = 0; i < 8; i++)
    {

      if (old[i] != enc[i].counter)
      {
//		String 		s = (String)"enc[" + (String)(int)i + (String)"] = " + (String)enc[i].counter;

//	    Serial.println(s);
        /*    Serial.print(micros() - mil1);
              Serial.print('\t');
              Serial.print(pin / 2);
              Serial.print('\t');
              Serial.println(enc[i].counter);
        */
        //      if (enc[i].counter < 0)
        //        enc[i].counter = 0;
        enc[i].counter += (enc[i].counter - old[i]) * moveSpeed;
	//	s = (String)"enc[" + (String)(int)i + (String)"] = " + (String)enc[i].counter;
   // 	Serial.println(s);
        String output = "";
        switch (i)
        {
          case 0:
            if (enc[i].counter < 0) enc[i].counter = 0;
            if (enc[i].counter > 97) enc[i].counter = 97;
            output = "D" + String(enc[i].counter) + ":";
            //          Serial.print("D");  // правое предплечье
            Serial.print(output);
            break;

          case 1:
            //          Serial.print("A");  // левое плечо
            if (enc[i].counter < 0) enc[i].counter = 0;
            if (enc[i].counter > 130) enc[i].counter = 130;
            output = "A" + String(enc[i].counter) + ":";
            Serial.print(output);
            break;

          case 2:
            //          Serial.print("E");  // левая рука вращение
            if (enc[i].counter < 0 - 35)  enc[i].counter = 0 - 35;
            if (enc[i].counter > (130 - 15)) enc[i].counter = (130 - 15);
            output = "E" + String(enc[i].counter + 35) + ":";
            Serial.print(output);
            break;

          case 3:
            //          Serial.print("H");  // голова по горизонтали
			if(enc[i].counter > 90) enc[i].counter = 90; 
			if(enc[i].counter < -90) enc[i].counter = -90; 
            output = "H" + String(enc[i].counter) + ":";
            Serial.print(output);
            break;

          case 4:
            //          Serial.print("C");  // правое плечо
            if (enc[i].counter < 0) enc[i].counter = 0;
            if (enc[i].counter > 127) enc[i].counter = 127;
            output = "C" + String(enc[i].counter) + ":";
            Serial.print(output);
            break;

          case 5:
            //          Serial.print("V"); // голова по вертикали
			if(enc[i].counter > 90) enc[i].counter = 90; 
			if(enc[i].counter < -90) enc[i].counter = -90; 
            output = "V" + String(enc[i].counter) + ":";
            Serial.print(output);
            break;

          case 6:
            //          Serial.print("E");  // правая рука вращение
            if (enc[i].counter < 0 - 35) enc[i].counter = 0 - 35;
            if (enc[i].counter > 130 - 15) enc[i].counter = 130 - 15;
            output = "F" + String(enc[i].counter + 35) + ":";
            Serial.print(output);
            break;

          case 7:
            //          Serial.print("B");  // левое плечо
            if (enc[i].counter < 0) enc[i].counter = 0;
            if (enc[i].counter > 97) enc[i].counter = 97;
            output = "B" + String(enc[i].counter) + ":";
            Serial.print(output);

            break;
        }
        old[i] = enc[i].counter;
      }
    }
    mil2 = millis();
  }

  char a = GetKey4x4(); // опрос клавиатуры
  if (a != 0)// при нажатии - вывод кнопки в порт
  {
    String outPut = "";
    const int scale1 = -45;
    const int scale2 = 45;
    konechnost left;
    konechnost right;



    switch (a)
    {
      case '1':        // Поворот головы влево
        // если один раз нажали кнопку после нулевого положения то второй раз угол увеличтися в 2 раза
        if (enc[3].counter == scale2) {
          enc[3].counter = 2 * scale2;
        }
        //  если угол больше scale но меньше предельного то поворачиваем до конца влево
        else if ((enc[3].counter > scale2) && (enc[3].counter < (scale2 * 2))) {
          enc[3].counter = scale2 * 2;
        }
        // в других случаях всегда поворачиваем на минимально заданный угоол
        //		  if(enc[3].counter == (scale*2)) enc[3].counter = scale;
        else enc[3].counter = scale2;
        outPut = "H" + String(enc[3].counter) + ":";
        Serial.print(outPut);
		old[3] = enc[3].counter;
        break;

      case '2':       // голову в середину
        enc[3].counter = 0;
        outPut = "H" + String(enc[3].counter) + ":";
        Serial.print(outPut);
        enc[5].counter = 0;
        outPut = "V" + String(enc[5].counter) + ":";
        Serial.print(outPut);
		old[3] = enc[3].counter;
		old[5] = enc[5].counter;
        break;

      case '3':       // голову вправо
        if (enc[3].counter == scale1) enc[3].counter  = scale1 * 2;
        else if ((enc[3].counter < scale1) && (enc[3].counter > (scale1 * 2))) enc[3].counter = scale1 * 2;
        else enc[3].counter = scale1;
        outPut = "H" + String(enc[3].counter) + ":";
        Serial.print(outPut);
		old[3] = enc[3].counter;
        break;

      case '4':  // left arm1
      case '5':
	  case '7':
	  case '8':
	  case '*':
	  case '0':
        selectPattern(left, a);
		// чтобы слишком часто в цикле не опрашивать в след раз
		for(int i = 0; i<3;i++) *leftOld[i] = LeftEnc[i]->counter;
//		old[6] = enc[6].counter;
//		old[1] = enc[1].counter;
//		old[7] = enc[7].counter;
        break;
		
	  case '6':
	  case 'B':
	  case '9':
	  case 'C':
	  case '#':
	  case 'D':
        selectPattern(right, a);
		for(int i = 0; i<3;i++) *rightOld[i] = RightEnc[i]->counter;
	    break;
    }
  }

  if (rec.clicked())
  {
    Serial.print("R:");  //rec
    if (recMode)
    {
      recMode = false;
      onLed = NORM_ON;
      offLed = NORM_OFF;
    }
    else
    {
      recMode = true;
      onLed = 800;
      offLed = 1000;
    }
  }
  if (play.clicked()) Serial.print("P:"); //play

//  mil1 = micros();

  blink(LED, onLed, offLed);
}

