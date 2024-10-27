#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

const int relay_pin = 11; // Connect relay to Arduino D11

LiquidCrystal_I2C lcd(0x3F, 16, 2);
char password[5];  // Extra space for null terminator
char initial_password[5], new_password[5];
char key_pressed = 0;
int i = 0;

const byte rows = 4;
const byte columns = 4;
char hexaKeys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte row_pins[rows] = {9, 8, 7, 6};
byte column_pins[columns] = {5, 4, 3, 2};
Keypad keypad_key = Keypad(makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);

void unlockDoor() {
  unsigned long unlockStartTime = millis();  // Record the start time
  
  while (millis() - unlockStartTime < 500) {
    digitalWrite(relay_pin, HIGH);
    delay(500);  // Small delay to stabilize the loop
  }
  
  digitalWrite(relay_pin, LOW);

  // Log the unlocking event with timestamp
  Serial.println("Door Unlocked");
}


void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(relay_pin, OUTPUT);
  Serial.begin(9600);   // Serial monitor communication
  Serial1.begin(9600);  // Serial1 for communication with HC-05

  lcd.print(" MOUNT DYNAMICS ");
  lcd.setCursor(0, 1);
  lcd.print("Electronic Lock ");
  delay(3000);
  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0, 1);
  initialPassword();
}

void loop() {
    if (Serial1.available() > 1) {
    char receivedChar = Serial1.read();
    Serial.print("Received: ");
    Serial.println(receivedChar);

    // Check if the received character is '1'
    if (receivedChar == '0') {
      digitalWrite(relay_pin, HIGH); // Turn on the relay
      Serial.println("Relay turned ON");
    } else {
      digitalWrite(relay_pin, LOW); // Turn off the relay
      Serial.println("Relay turned OFF");
    }

    // Real-time monitoring of relay state
    if (digitalRead(relay_pin) == HIGH) {
      Serial.println("Relay is currently ON");
    } else {
      Serial.println("Relay is currently OFF");
    }

    delay(100);  // Add a small delay to allow time for processing
  }


  digitalWrite(relay_pin, HIGH);

  key_pressed = getKey();
  if (key_pressed == '#')
    changePassword();
  if (key_pressed) {
    password[i++] = key_pressed;
    lcd.print('*');
  }
  if (i == 4) {
    delay(200);
    if (checkPassword()) {
      lcd.clear();
      lcd.print("Pass Accepted");
      unlockDoor();
      lcd.clear();
      lcd.print("Enter Password:");
      i = 0;
    } else {
      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0, 1);
      lcd.print("Try Again");
      delay(2000);
      lcd.clear();
      lcd.print("Enter Password");
      i = 0;
    }
  }

}


// Remaining functions remain unchanged...

char getKey() {
  char key = keypad_key.getKey();
  return key;
}

void changePassword() {
  int j = 0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(0, 1);

  while (j < 4) {
    char key = getKey();
    if (key) {
      new_password[j++] = key;
      lcd.print('*');
    }
  }

  delay(500);

  if (strncmp(new_password, initial_password, 4) != 0) {
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(1000);
  } else {
    j = 0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0, 1);

    while (j < 4) {
      char key = getKey();
      if (key) {
        initial_password[j] = key;
        lcd.print('*');
        EEPROM.write(j, key);
        j++;
      }
    }

    lcd.clear();
    lcd.print("Pass Changed");
    delay(1000);
  }

  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0, 1);
}

void initialPassword() {
  for (int j = 0; j < 4; j++) {
    initial_password[j] = EEPROM.read(j);
  }
}

bool checkPassword() {
  return strncmp(password, initial_password, 4) == 0;
}
