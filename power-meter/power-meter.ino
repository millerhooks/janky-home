#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
//#include "SdFat.h"
//SdFat SD;


enum menu_state {
  reset_energy,
  menu_state_end
};

Adafruit_SSD1306 display(128,64,&Wire,-1);
Adafruit_INA219 ina219;

unsigned long previousMicros = 0;
unsigned long interval = 200000;
const int chipSelect = 10;
unsigned long currentMicros;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float energy = 0;
float maxCurrent = 0;
//File TimeFile;
//File VoltFile;
//File CurFile;

unsigned long startMicros;

#define ACTION_PIN 3
menu_state global_state = reset_energy;
void action(){

  if(global_state == reset_energy){
    energy = 0;
    startMicros = micros();
    previousMicros = startMicros;
    maxCurrent = 0;
  }
}


void setup() {
 // SD.begin(chipSelect);
  ina219.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.setRotation(2);
  pinMode(ACTION_PIN, INPUT);

  delay(1000);

  startMicros = micros();
  previousMicros = startMicros;

}
unsigned long previousDisplayMicros = 0;
void loop() {
 // Serial.println("running!");
  currentMicros = micros();

  ina219values();



  if (currentMicros - previousDisplayMicros >= interval)
  {


  previousDisplayMicros = currentMicros;


   /* TimeFile = SD.open("TIME.txt", FILE_WRITE);
    if (TimeFile) {
      TimeFile.println(currentMicros);
      TimeFile.close();
    }

    VoltFile = SD.open("VOLT.txt", FILE_WRITE);
    if (VoltFile) {
      VoltFile.println(loadvoltage);
      VoltFile.close();
    }

    CurFile = SD.open("CUR.txt", FILE_WRITE);
    if (CurFile) {
      CurFile.println(current_mA);
      CurFile.close();
    }*/
    displaydata();
  }
  previousMicros = currentMicros;
  if(digitalRead(ACTION_PIN)){
    action();
  }
  delayMicroseconds(20);
}

void displaydata() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(loadvoltage);
  display.setCursor(35, 0);
  display.println("V");
  display.setCursor(50, 0);
  display.println(current_mA);
  display.setCursor(95, 0);
  display.println("mA");
  display.setCursor(0, 10);
  display.println(loadvoltage * current_mA);
  display.setCursor(65, 10);
  display.println("mW");
  display.setCursor(0, 20);
  display.println(((double)energy) / ( 1000000.0  * 60.0 * 60.0));
  display.setCursor(65, 20);
  display.println("mWh");
  
  display.setCursor(0, 40);
  display.println(((double)(currentMicros - startMicros)) / 1000000);
  display.setCursor(65, 40);
  display.println("seconds");
  
  display.setCursor(0, 30);
  display.println("Max");
  display.setCursor(25, 30);
  display.println((maxCurrent);
  display.setCursor(65, 30);
  display.println("mA");
display.display();
}

void ina219values() {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  if(current_mA > 0){
    energy += loadvoltage * current_mA * (currentMicros - previousMicros);  
    if(current_mA > maxCurrent){
      maxCurrent = current_mA;
    }
  }

}
