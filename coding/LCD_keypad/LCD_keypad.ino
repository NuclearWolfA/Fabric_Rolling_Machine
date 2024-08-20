

// include the library code:
#include <LiquidCrystal.h>

// Include Keypad library
#include <Keypad.h>
 
// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 4;
 
// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
 
// Connections to Arduino
byte rowPins[ROWS] = {8, 10, A3, A2};
byte colPins[COLS] = {A1, A0, 6, 7};
 
// Create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2, ct=9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String length;
char fabric;


void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  analogWrite(ct,150);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("  PrecisionRoll   ");
  delay(2000);
  lcd.clear();


  
}

void loop(){
  
  lcd.print("A:Cotton B:Silk ");
  lcd.setCursor(0, 1);
  lcd.print("C:Linen  D:Other");
  
  bool fabricSel = false;
  while (!fabricSel){
    // Get key value if pressed
    char customKey = customKeypad.getKey();
    
    if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D'){
      fabricSelect(customKey);

      bool confirmed = false;
      while(!confirmed){
        customKey = customKeypad.getKey();

        if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D'){
          fabricSelect(customKey);
        }


        if (customKey == '#'){
          confirmed = true;
        }
      }
      lcd.setCursor(0, 1);
      lcd.print("Length: ");
      bool entered = false;
      while (!entered){
        customKey = customKeypad.getKey();
        if (customKey == '#'){
          entered = true;
        }else if (customKey == '*'){
          length = "";
          lcd.setCursor(0, 1);
          lcd.print("                ");
          lcd.setCursor(0, 1);
          lcd.print("Length: ");

        }
        if ((customKey >= '0' && customKey <= '9')){
          String konv = String(customKey);
          length += konv;
          lcd.print(konv);

        }
      }
      fabricSel = true;  
    } 
  }
  lcd.clear();
  lcd.print("   Rolling...  ");
  delay(10000);










  lcd.clear();
  lcd.print("    Finished.");
  delay(5000);
  lcd.clear();
  

}


void fabricSelect(char customKey){
  // Clear LCD display and print character
  fabric = customKey;
  lcd.clear();
  switch (customKey){
    case 'A':
      lcd.print("Cotton");
      break;
    case 'B':
      lcd.print("Silk");
      break;
    case 'C':
      lcd.print("Linen");
      break;
    case 'D':
      lcd.print("Other");
      break;
  }
}

