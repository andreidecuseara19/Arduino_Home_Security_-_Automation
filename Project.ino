#include <Keypad.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter; // initialize BH1750 object

// Define to which pin of the Arduino the 1-Wire bus is connected:
#define ONE_WIRE_BUS 11
// Create a new instance of the oneWire class to communicate with any OneWire device:
OneWire oneWire(ONE_WIRE_BUS);
// Pass the oneWire reference to DallasTemperature library:
DallasTemperature sensors(&oneWire);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(10, 9, A0, A1, A2, A3);

const int ROW_NUM = 4;
const int COLUMN_NUM = 3;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROW_NUM] = {8, 7, 6, 5};
byte colPins[COLUMN_NUM] = {4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROW_NUM, COLUMN_NUM);

const String password = "1234";
String input_password;

enum State {
  ACCESS_DENIED,
  ACCESS_GRANTED,
  MENU,
};

State currentState = ACCESS_DENIED;

int selectedOption = 0;

const int numOptions = 4;

const char *menuOptions[] = {
  "Temperature Monitor",
  "Relay Control",
  "Light Monitor",
  "Sensor 3 Input"
};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lightMeter.begin();
  lcd.begin(16, 2);
  input_password.reserve(32);

  sensors.begin();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter password:");
}

//Loop function
void loop() {
  switch (currentState) {
    case ACCESS_DENIED:
      handleAccessDenied();
      break;

    case ACCESS_GRANTED:
      handleAccessGranted();
      break;

    case MENU:
      handleMenu();
      break;
  }
}

//Refresh pentru LCD atunci cand se face scroll in meniu
void clearAndUpdateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select option:");
  for (int i = 0; i < numOptions; i++) {
    lcd.setCursor(0, i + 1);
    if (i == selectedOption) {
      lcd.print(">");
      lcd.setCursor(1, i + 1);
      lcd.print(menuOptions[i]);
    }
  }
}

//Wrong Password was input
void handleAccessDenied() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);
    lcd.setCursor(input_password.length(), 1);
    lcd.print('*');

    if (key == '*') {
      input_password = "";
      lcd.setCursor(0, 1);
      lcd.print("                ");
    } else if (key == '#') {
      if (password == input_password) {
        Serial.println("password is correct");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access granted");
        currentState = MENU;
        clearAndUpdateDisplay();
      } else {
        Serial.println("password is incorrect, try again");
        lcd.setCursor(0, 1);
        lcd.print("Access denied    ");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password:");
      }
      input_password = "";
    } else {
      input_password += key;
    }
  }
}

//Right Password was input
void handleAccessGranted() {
  Serial.println("Access granted");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access granted");
  currentState = MENU;
  clearAndUpdateDisplay();
}

//Menu access
void handleMenu() 
{
  char key = keypad.getKey();

  if (key) 
  {
    if (key == '2') 
    {
      if (selectedOption > 0) 
      {
        selectedOption--;
      }
      clearAndUpdateDisplay();
    }
    else if (key == '8') 
    {
      if (selectedOption < numOptions - 1) {
        selectedOption++;
      }
      clearAndUpdateDisplay();
    } 
    else if (key == '#') 
      {
        Serial.println(selectedOption);
        // Handle menu option based on selectedOption
        if (selectedOption == 0) 
        {
          Serial.println("Case 0 accesed");
          //Temperature Monitoring
          lcd.clear();
          lcd.setCursor(2,0);
          lcd.print("Temperature:");
          lcd.setCursor(1,1);
          // Send the command for all devices on the bus to perform a temperature conversion:
          sensors.requestTemperatures();

          //Digital temperature read
          // Fetch the temperature in degrees Celsius for device index:
          float tempDigital = sensors.getTempCByIndex(0); // the index 0 refers to the first device

          Serial.println(tempDigital);
          lcd.print(tempDigital);
          lcd.print("* Digital");
        }
        else if (selectedOption == 1) 
        {
          Serial.println("Case 1 accesed");
          // Relay Control selected, add your code here
        }
        else if (selectedOption == 2) 
        {
          Serial.println("Case 2 accesed");
          // Light Level Monitoring
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("Light Level:");
          lcd.setCursor(5, 1);
                    
          float lux = lightMeter.readLightLevel();
          Serial.print("Light: ");
          Serial.print(lux);
          Serial.println(" lx");
          
          // Update the LCD screen with the light level
          lcd.print(lux);
          lcd.print(" lx");
        }
        else if (selectedOption == 3) 
        {
          Serial.println("Case 3 accesed");
          // Sensor 3 Input selected, add your code here
        }
      }
    } 
    else if (key == '*') 
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access granted");
      currentState = ACCESS_GRANTED;
    }
}
