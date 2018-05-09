
/*
  A program for controlling a single stepper motor driving a rotary table.
  Uses a 4x4 matrix keypad for entry of degrees and direction to move the table.
  Serial I2C display, I2C Keyboard adapter.

  Herb Blair
  Carrollton, TX 75006

  March 2018
*/

#include <Keypad_I2C.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <LCD.h>
#include <avr/pgmspace.h>
#define I2CADDR    0x27  // Define I2C Address where the PCF8574A
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
#define Keypad_ADDR 0x25

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'D', '#', '0', '*'},
  {'C', '9', '8', '7'},
  {'B', '6', '5', '4'},
  {'A', '3', '2', '1'}

};

byte rowPins[ROWS] = {3, 2, 1, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad

Keypad_I2C customKeypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, Keypad_ADDR, PCF8574 );


int GetMode();
void tablesetup();
void  software_Reset();

LiquidCrystal_I2C  lcd(I2CADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

//setup vars
const int stp = 5;  //connect pin 5 to PUL+
const int dir = 4;  // connect pin 4 to DIR+
const int ena = 3; //enable pin // connect pin 3 to ENA+
const int StepsPerRotation = 400; //Set Steps per rotation of stepper
const int TableRatio = 90; //ratio of rotary table
const long Multiplier = 100;
//(StepsPerRotation * TableRatio) / 360;
const int stepdelay = 1;
float Degrees = 0;                //Degrees from Serial input
float ToMove = 0;                 //Steps to move
float Divisions;
float Divisonscount = 1;
float current = 0;
int Mode = 0;



float num = 0.00;
char customKey;
//*********************************************************************************
void setup()
{
  Serial.begin(115200);
  customKeypad.begin( );
  lcd.begin (20, 4);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(ena, OUTPUT);
  lcd.backlight();
  lcd.print(" HW Blair & Company");
  lcd.setCursor(0, 1);
  lcd.print("Rotary Table Indexer");
  lcd.setCursor(0, 2);
  lcd.print("    Rev D 2018");
  delay(2000);
  lcd.home();
  Mode = GetMode();
  // int Divisions;
}

//*********************************************************************************
int GetMode()
{
  lcd.home();
  lcd.clear();
  int Mode = 0;
  lcd.setCursor(0, 0); lcd.print("    Select Mode");

  lcd.setCursor(0, 1);
  lcd.print("Press A  Divisions");

  lcd.setCursor(0, 2);
  lcd.print("Press B  Degrees");
  lcd.setCursor(0, 3);
  lcd.print("Press C SETUP");


  while (Mode == 0)

  {

    customKey = customKeypad.getKey();
   
    if (customKey == 'A')
    {
      Mode = 1;
    }
    if (customKey == 'B')
    {
      Mode = 2;
    }
    if (customKey == 'C')
    {
      Mode = 3;
    }
    if (customKey == 'D')
    {
      Mode = 4;
    }
  }
lcd.clear();
  return Mode;
}








//*********************************************************************************

float GetNumber()
{
  float num = 0.00;
  float decimal = 0.00;
  float decnum = 0.00;
  int counter = 0;
  lcd.clear();
  lcd.setCursor(0, 0);



  lcd.print("Enter # of Degrees");
  lcd.setCursor(0, 2);
  lcd.print("Then Press Enter [#]");
  lcd.setCursor(8, 3);
  bool decOffset = false;

  while (customKey != '#')
  {
    switch (customKey)
    {
      case NO_KEY:
        break;

      case '.':
        if (!decOffset)
        {
          decOffset = true;
        }
        lcd.print(customKey);

        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        if (!decOffset)
        {
          num = num * 10 + (customKey - '0');
          lcd.print(customKey);
        }
        else if ((decOffset) && (counter <= 1))
        {
          num = num * 10 + (customKey - '0');
          lcd.print(customKey);
          counter++;
        }
    }
    decnum = num / pow(10, counter);
    customKey = customKeypad.getKey();
  }

  return decnum;
}

//*****************************************************************************************
void loop() {


  if (Mode == 1)
  {
    Divisions = GetDivisions();
    Degrees = (360 / Divisions);
  }
  if (Mode == 2)
  {
    Degrees = GetNumber();
  }
  if (Mode == 3)
  {
 Serial.println ("Mode 3");
  void tablesetup();
  }
  if (Mode == 4)
  {
  lcd.clear();
  lcd.print("mode 4");
   
   
 void  Software_reset();
  }

  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print("FWD[A] REV[B] RST[D]");
  customKey = customKeypad.getKey();
  while (customKey != 'D')
  {
    lcd.setCursor(0, 0); lcd.print("DPM: "); lcd.print(Degrees);

    lcd.setCursor(0, 1);
    lcd.print("POS: "); lcd.print(current);
    Divisonscount = (current / Degrees) + 1;
    lcd.setCursor(0, 2); lcd.print("POS COUNT ");
    lcd.print(Divisonscount);

    customKey = customKeypad.getKey();
    if (customKey == 'A')
    {
      if (current >= 360)
      {
        current = (current + Degrees) - 360;
        Divisonscount  ++;
      } else {
        current = current + Degrees;
      }
      ToMove = Degrees * Multiplier;
      lcd.setCursor(0, 0);

      lcd.print("WAIT Moving Table");
      rotation(ToMove, 2);
      lcd.setCursor(0, 2); lcd.print("                   ");
      lcd.setCursor(0, 0); lcd.print("                   ");

    }
    if (customKey == 'B')
    {
      if (current <= 0)
      {
        current = 360 + (current - Degrees);

      } else {
        current = current - Degrees;
      }
      ToMove = Degrees * Multiplier;
      lcd.setCursor(0, 0);
      lcd.print("Moving Table");
      rotation(ToMove, 0);
      lcd.setCursor(0, 2); lcd.print("                   ");
      lcd.setCursor(0, 0); lcd.print("                   ");
    }
    if (customKey == 'D')
    {
      software_Reset();
    }
  }
  lcd.clear();
}


//*********************************************************************************
float GetDivisions()
{
  float num = 0.00;
  char key = customKeypad.getKey();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Enter # Divisions"); lcd.setCursor(0, 2); lcd.print("Then press [#].");
  //lcd.setCursor(0, 2); lcd.print("Reset [D]");
  lcd.setCursor(8, 3);

  while (customKey != '#')
  {
    switch (customKey)
    {
      case NO_KEY:
        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        num = num * 10 + (customKey - '0');
        lcd.print(customKey);
        break;

      case 'D':
        software_Reset();
        break;
    }
    customKey = customKeypad.getKey();
    // num = 360/num;
  }
  return num;
}
//*********************************************************************************
void rotation(float tm, int d)
{
  digitalWrite(ena, HIGH);

  if (d == 0)
  {

    digitalWrite(dir, LOW);
  }
  else
  {
    digitalWrite(dir, HIGH);
  }

  for (int i = 0; i < tm; i++)
  {
    digitalWrite(stp, HIGH);
    delay(stepdelay);
    digitalWrite(stp, LOW);
    delay(stepdelay);
  }

  digitalWrite(ena, LOW);
}
//*********************************************************************************
// SETUP
void tablesetup() {
  Serial.println("table setup");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TABLE SETUP");
delay(2000);
}
//*********************************************************************************
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}
