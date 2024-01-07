#include <dht.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <BH1750.h>

#define DHT22_PIN 12    //Defines the pin used to read from the DHT22 sensor
dht DHT;       // Creates a DHT object
BH1750 lightMeter;    // initialize BH1750 object for the light meter
#define ONE_WIRE_BUS 11   // Define to which pin of the Arduino the 1-Wire bus is connected:
OneWire oneWire(ONE_WIRE_BUS);    // Create a new instance of the oneWire class to communicate with any OneWire device:
DallasTemperature sensors(&oneWire);    // Pass the oneWire reference to DallasTemperature library:
LiquidCrystal lcd(13, 9, A0, A1, A2, A3);   // initialize the library by associating any needed LCD interface pin with the arduino pin number it is connected to

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

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROW_NUM, COLUMN_NUM); //Defining the keypad object with the pin numbers it is connected to

const String password = "1234";   //Password needed for access
String input_password;

enum State {
  ACCESS_DENIED,
  ACCESS_GRANTED,
  MENU,
  AUTO
};

State currentState = ACCESS_DENIED;

int selectedOption = 0;

const int numOptions = 5;

const char *menuOptions[] = 
{
  "Temp Monitor",
  "Motor Control",
  "Light Monitor",
  "Temp & Hum In",
  "Automatic"
};

// Setting up the registries for motor control
void setup_timer()
{
  TCCR1A = 0x00; // Registry initiaization
  TCCR1A |= (1<<5); //Clear OC1B on compare match, set OC1B at BOTTOM (non-inverting mode)
  TCCR1A |= (1<<1); //WGM11,
  TCCR1A |= (1<<0); //WGM10 set to Fast PWM with OCR1A TOP
  TCCR1B = 0x00; // Registry initialization
  TCCR1B |= (1<<4) | (1<<3); //WGM13, WGM12 set to Fast PWM with OCR1A TOP
  TCCR1B |= (1<<0); //CS10 set to no prescaling
  OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
  OCR1B = 0; // For a starting duty cycle of 0%
  DDRB |= (1<<2); //DDRB |= 0b00000100 -- Setting pin PB2 corresponding to OC1B as output
}


void setup() 
{
  Serial.begin(9600);   //Initialize serial monitor

  setup_timer();    //Setup for Motor registers
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
void loop() 
{
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

    case AUTO:
      handleAuto();
      break;
  }
  delay(10);
}

//Refresh pentru LCD atunci cand se face scroll in meniu
void clearAndUpdateDisplay() 
{
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

// Automatic Operation Mode
void handleAuto()
{
  lcd.clear();
  lcd.print("Automatic Mode");
  Serial.println("Automatic Mode");
  sensors.requestTemperatures();
  float tempDigital = sensors.getTempCByIndex(0);
  Serial.println(tempDigital);
  
  if(tempDigital > 23.00 && tempDigital < 27.00)
  {
    OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
    OCR1B = 120; // For a duty cycle of 75%
  }
  else if(tempDigital >= 27.00)
  {
    OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
    OCR1B = 155; // For a duty cycle of 95%
  }
  else if(tempDigital <= 23.00)
  {
    OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
    OCR1B = 0; // For a duty cycle of 0%
  }
  
  delay(2000);
}

//Wrong Password was input
void handleAccessDenied() 
{
  char key = keypad.getKey();

  if (key) 
  {
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
void handleAccessGranted() 
{
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
  Serial.print("This is key: ");
  Serial.println(key);
  if (key!='*') 
  {
    switch (key) {
      case '2':
      {
        if (selectedOption > 0) 
        {
          selectedOption--;
        }
        clearAndUpdateDisplay();
        break;
      }
      case '8':
      {
        if (selectedOption < numOptions - 1) 
        {
          selectedOption++;
        }
        clearAndUpdateDisplay();
        break;
      }
      case '#':{
      
        Serial.print("selectedOption: ");
        Serial.println(selectedOption);
        // Handle menu option based on selectedOption
        switch (selectedOption) 
        {
          // Temperature Monitoring
          case 0:
          {
            Serial.println("Case 0 accessed");
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("Temperature:");
            lcd.setCursor(1, 1);
            sensors.requestTemperatures();
            float tempDigital = sensors.getTempCByIndex(0);
            Serial.println(tempDigital);
            lcd.print(tempDigital);
            lcd.print("* Digital");
            break;
          }

          // Motor Control selected
          case 1:
          {
            Serial.println("Case 1 accessed");
            
            Serial.println("factor 50%");
            TCCR1B |= (1<<0);
            OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
            OCR1B = 80; 
            delay(5000);
            Serial.println("factor 97%");
            OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
            OCR1B = 155; 
            delay(5000);
            Serial.println("off");
            //TCCR1B &= ~(1<<0);
            Serial.println("factor 0%");
            OCR1A = 160; // For a period of 10 microseconds and prescaler of 1
            OCR1B = 0;
            delay(5000); 
            break;
          }

          // Light Level Monitoring
          case 2:
          {
            Serial.println("Case 2 accessed");
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("Light Level:");
            lcd.setCursor(5, 1);
            float lux = lightMeter.readLightLevel();
            Serial.print("Light: ");
            Serial.print(lux);
            Serial.println(" lx");
            lcd.print(lux);
            lcd.print(" lx");
            break;
          }

          // Temperature & Humidity Monitoring
          case 3:
          {
            Serial.println("Case 3 accessed");
            int readData = DHT.read22(DHT22_PIN);

            float t = DHT.temperature;        // Read temperature from DHT22 sensor
            float h = DHT.humidity;           // Read humidity from DHT22 sensor

            Serial.print("Temperature = ");
            Serial.print(t);
            Serial.print("°C | ");
            Serial.print((t*9.0)/5.0+32.0);        // Convert celsius to fahrenheit
            Serial.println("°F ");
            Serial.print("Humidity = ");
            Serial.print(h);
            Serial.println("% ");
            Serial.println("");

            //Printing the sensor values to the lcd display
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Temp: ");
            lcd.print(t);
            lcd.setCursor(0, 1);
            lcd.print("Humidity: ");
            lcd.print(h);

            //delay(2000); // wait two seconds
            break;
          } 

          // Entering automatic mode and exiting the menu
          case 4:
          {
            currentState = AUTO;
            break;          
          }

          default:
          { 
            Serial.println("Wrong Key");
            break;
          }
        }
      }
        default: 
        {
          Serial.println("Default");
          break;
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
}
