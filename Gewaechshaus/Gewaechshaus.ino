// Temperatursteuerung Dachluke Schrittmotors
#include <HTU21D.h>
int humidity = 0;
int temperature = 0;
HTU21D sensor;
#include <Stepper.h>
bool stepMotor = false;               // Dachluke
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 8 through 11:
Stepper dachluke(stepsPerRevolution, 8, 10, 9, 11);

// ldr
int ldrPin = A2;
int yellowLedPin = 2;
int lightLevel = 0;

// Füllstand des Behälters
int waterLevelPin = A3;  // putty
int waterLevel = 0;
bool waterAllert = false;
int redLed = 4;

// oled
#include <Arduino.h>
#include <U8x8lib.h>
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);

// bewegungssensor
int pirPin = 12;  // Input for HC-S501
int pirValue;     // Place to store read PIR Value
int buzzer = 6;

// wasserpumpe
int blueLed = 3;

// servo
#include <Servo.h>
Servo tuer;
int tuerPos = 0;         // offen
bool tuerStatus = true;  // tuer standartmaessig offen damit der Summer nicht summt

// EEPROM
#include <EEPROM.h>

// IRemote
#include "IRremote.h"
#include "IR.h"

IRrecv irrecv(RECEIVER);  // create instance of 'irrecv'
decode_results results;   // create instance of 'decode_results'


void setup() {
  Serial.begin(9600);
  // Temperatursteuerung Dachluke Schrittmotors
  sensor.begin();
  dachluke.setSpeed(rolePerMinute);
  // oled
  u8x8.setPowerSave(0);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  // wasserstand
  pinMode(waterLevelPin, INPUT);
  pinMode(redLed, OUTPUT);
  // ldr
  pinMode(ldrPin, INPUT);
  pinMode(yellowLedPin, OUTPUT);
  // bewegungssensor
  pinMode(pirPin, INPUT);
  pinMode(buzzer, OUTPUT);
  // wasserpumpe
  pinMode(blueLed, OUTPUT);
  // servo
  tuer.attach(7);  // connect pin 7 with the control line(the middle line of Servo)
  // IRemote
  irrecv.enableIRIn();
}


void loop() {
  setTemperature();
  setWaterLevel();
  setLdr();
  bewaesserung();
  setBewegungsSensor();
  openWindow(temperature);  // Stepper
  setInput();
  doorLock();
  closeDoor();  // Wenn Power gedrueckt wird!

  printOled(temperature);
  delay(100);
}

// Funktion von Vincent :)
void print(int row = 0, String text = "                ", int start = 0) {
  u8x8.setCursor(start, row);
  u8x8.print(text);
}

void openWindow(double temperature) {
  if (temperature > 25 && stepMotor == false) {
    Serial.println("Open window");
    dachluke.step(stepsPerRevolution);
    stepMotor = true;
  } else if (temperature < 25 && stepMotor) {
    Serial.println("Close Window");
    dachluke.step(-stepsPerRevolution);
    stepMotor = false;
  }
}

void setWaterLevel() {
  waterLevel = analogRead(waterLevelPin);

  if (waterLevel < 300) {
    digitalWrite(redLed, HIGH);
  } else {
    digitalWrite(redLed, LOW);
  }
}

void setLdr() {
  lightLevel = analogRead(ldrPin);

  if (lightLevel < 250) {
    digitalWrite(yellowLedPin, HIGH);
  } else {
    digitalWrite(yellowLedPin, LOW);
  }
}

void setTemperature() {
  if (sensor.measure()) {
    temperature = sensor.getTemperature();
    digitalWrite(blueLed, HIGH);
    humidity = sensor.getHumidity();
  }
}

void bewaesserung() {
  digitalWrite(blueLed, LOW);
  if (humidity < 30 && waterLevel != 0) digitalWrite(blueLed, HIGH);
}

void printOled(int temperature) {
  print(1, "Tmp:");
  print(1, String(temperature), 4);
  print(1, "Hum:", 7);
  print(1, String(humidity), 11);
  print(2, "waterLvl:");
  print(2, String(waterLevel), 10);
  print(3, "lightLvl: ");
  print(3, String(lightLevel), 10);

  if (tuerStatus == false) print(7, "Door is closed");
  if (tuerStatus) print(7, "Door is open  ");
}

void setBewegungsSensor() {
  pirValue = digitalRead(pirPin);
  // Serial.println(pirValue);

  if (pirValue == 1 && tuerStatus == false) {
    digitalWrite(buzzer, HIGH);  // aus un nicht zu nerven!
    delay(2000);
  } else {
    digitalWrite(buzzer, LOW);
  }
}

String pinCode[] = { "1", "2", "4", "7" };
String inputCode[4];
int indexCode = 0;
bool allowInput = true;
String keyPressed;

void openDoor() {
  for (tuerPos = 0; tuerPos <= 90; tuerPos += 1) {
    tuer.write(tuerPos);
    delay(15);
  }
  tuerStatus = true;
}

void closeDoor() {
  if (tuerStatus && keyPressed == "POWER") {
    for (tuerPos = 90; tuerPos >= 0; tuerPos -= 1) {
      tuer.write(tuerPos);
      delay(15);
    }
    tuerStatus = false;
  }
}

void doorLock() {
  if (tuerStatus == false) {
    print(5, "pin: ");

    if (keyPressed != "" && allowInput && isANumber()) {
      inputCode[indexCode] = keyPressed;
      print(5, String(keyPressed.charAt(0)), indexCode + 5);
      indexCode++;
      keyPressed = "";
    }

    if (indexCode == 4) {
      allowInput = false;
      indexCode = 0;

      if (pinCode[0] == inputCode[0] && pinCode[1] == inputCode[1] && pinCode[2] == inputCode[2] && pinCode[3] == inputCode[3]) {
        delay(1000);
        print(5, "correct", 5);
        openDoor();
        u8x8.clear();
        String inputCode[4];
        allowInput = true;
        // print(6, "Door is open");
      } else {
        print(5, "wrong", 5);
        delay(1000);
        u8x8.clear();
        String inputCode[4];
        allowInput = true;
      }
    }
  }
}

void setInput() {
  int tmpValue;
  int i = 0;

  if (irrecv.decode(&results)) {
    for (int i = 0; i < 23; i++) {
      if ((keyValue[i] == results.value) && (i < KEY_NUM)) {
        Serial.println(keyBuf[i]);
        keyPressed = keyBuf[i];
        tmpValue = results.value;
      } else if (REPEAT == i) {
        results.value = tmpValue;
      }
    }
    irrecv.resume();
  }
}

bool isANumber() {
  if (keyPressed == "1" || keyPressed == "2" || keyPressed == "3" || keyPressed == "4" || keyPressed == "5" || keyPressed == "6" || keyPressed == "7" || keyPressed == "8" || keyPressed == "9" || keyPressed == "0") {
    return true;
  } else {
    return false;
  }
}

void writeToEEPROM() {
  // EEPROM.put
}
