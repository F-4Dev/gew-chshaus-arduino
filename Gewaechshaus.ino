// Temperatursteuerung Dachluke Schrittmotors
#include <HTU21D.h>
int humidity = 0;
int temperature = 0;
HTU21D sensor;
#include <Stepper.h>
bool stepMotor = false; // Dachluke
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

// ldr
int ldrPin = A2;
int mainLledPin = 2;
int lightLevel = 0;

// Füllstand des Behälters
int waterLevelPin = A3; // putty
int waterLevel = 0;
bool waterAllert = false;

// Tuer
#include <Servo.h>
Servo myservo;

// oled
#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// EEPROM
#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  // Temperatursteuerung Dachluke Schrittmotors
  sensor.begin();
  myStepper.setSpeed(rolePerMinute);
  // oled
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setPowerSave(0);
  // water level
  pinMode(waterLevelPin, INPUT); // Putty
  // ldr
  // pinMode();
  // Tuer
  myservo.attach(7);//connect pin 9 with the control line(the middle line of Servo) 
  myservo.write(90);// move servos to center position -> 90°
}

void loop() {
  sensorTemperature();
  openWindow(temperature); // Stepper
  printSerial();
  printOled();

  // oled



  // EEPROM
  // writeToEEPROM();
  delay(2000);
}

void printSerial() {
  Serial.print("t: ");
  Serial.println(temperature);
  Serial.print("h: ");
  Serial.println(humidity);
}

void openWindow(double temperature) {

  if (temperature > 25 && stepMotor == false) {
    Serial.println("Open window");
    myStepper.step(stepsPerRevolution);
    delay(500);
    stepMotor = true;
  } else if (temperature < 25 && stepMotor) {
    Serial.println("Close Window");
    myStepper.step(-stepsPerRevolution);
    delay(500);
    stepMotor = false;
  }
}


void sensorTemperature() {
  if (sensor.measure()) {
    temperature = sensor.getTemperature();
    humidity = sensor.getHumidity();
  }
}

void printOled() {
  u8x8.setCursor(0,1);
  u8x8.print(temperature);
}

void sensorWaterlevel() {
  waterLevel = analogRead(waterLevelPin);
  Serial.println(analogRead(waterLevelPin));
}

// void writeToEEPROM() {
//   byte byteWaterlevel = waterLevel;
// }




// www.elegoo.com
//2018.12.19

/* After including the corresponding libraries,
   we can use the "class" like "Servo" created by the developer for us.
   We can use the functions and variables created in the libraries by creating 
   objects like the following "myservo" to refer to the members in ".".*/


//it created an object called myservo.
/*  you can see Servo as a complex date type(Including functions and various data types)
    and see myservo as variables.               */

void setup(){
  /*"attach" and "write" are both functions,
     and they are members contained in the complex structure of "Servo". 
     We can only use them if we create the object "myservo" for the complex structure of "Servo".
  */
  myservo.attach(9);//connect pin 9 with the control line(the middle line of Servo) 
  myservo.write(90);// move servos to center position -> 90°
} 
void loop(){
  myservo.write(90);// move servos to center position -> 90°
  delay(1000);
  myservo.write(60);// move servos to center position -> 60°
  delay(1000);
  myservo.write(90);// move servos to center position -> 90°
  delay(1000);
  myservo.write(150);// move servos to center position -> 120°
  delay(1000);
}


