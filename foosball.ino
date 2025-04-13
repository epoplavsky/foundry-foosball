//Display libraries
#include <Arduino.h>
#include <TM1637Display.h>

//ToF libraries
#include <Wire.h>
#include "Adafruit_VL6180X.h"

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

int away=0;
int home=0;
const int scoreToWin = 7;

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

TM1637Display display(CLK, DIO);
Adafruit_VL6180X vl = Adafruit_VL6180X();

void resetGame()
{
  away = 0;
  home = 0;
}

void win() //flashes score on winning team's side
{
  int cycles = 10;
  int cycleDelay = 500;
  for(int i = 0; i < cycles; i++)
    {
      displayScore();
      delay(cycleDelay);
      display.clear();
      delay(cycleDelay);
    }

}

void lose() //disables display on losing teams side while winner's flashes
{
  display.clear();
}
void awayScore()
{
  away++;
  int cycles = 5;
  int cycleDelay = 250;
  for(int i = 0; i < cycles; i++) //flashes away score
    {
      displayScore();
      delay(cycleDelay);
      data[2] = 0x00;
      data[3] = 0x00;
      display.setSegments(data);
      delay(cycleDelay);
    }
}

void homeScore()
{
  home++;
  int cycles = 5;
  int cycleDelay = 250;
  for(int i = 0; i < cycles; i++) //flashes home score
    {
      displayScore();
      delay(cycleDelay);
      data[0] = 0x00;
      data[1] = 0x00;
      display.setSegments(data);
      delay(cycleDelay);
    }

}

void displayScore()
{
  


  if (home < 10) //Displays home score
  {
    data[0] = 0x00;
    data[1] = display.encodeDigit(home);
  }
  else
  {
    data[0] = display.encodeDigit(1);
    data[1] = display.encodeDigit(0);
  }

  if (away < 10) //Displays away score
  {
    data[2] = 0x00;
    data[3] = display.encodeDigit(away);
  }
  else
  {
    data[2] = display.encodeDigit(1);
    data[3] = display.encodeDigit(0);
  }
  display.setSegments(data);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  while (! vl.begin()) //loops until ToF sensor is found
    Serial.println("Failed to find sensor");
    delay(100); //will not find the sensor without this delay

  Serial.println("Sensor found!");

  display.setBrightness(6);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t range = vl.readRange();
  Serial.print("Range: "); Serial.println(range);
  //displayScore();
  if (range < 10 && home  < 7) //ball rolls through the return pipe and max score has not been reached
    homeScore();
  

  if(home >= scoreToWin)
    win();
  else if (away >= scoreToWin)
    lose();
  else
    displayScore();
    
  //delay(0);


}
