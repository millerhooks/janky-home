
#include <Arduino.h>


#include "esp_timer.h"
#include "utils.h"

//plans at https://www.circuitlab.com/editor/#?id=86q73972rx4q







/**
*
*
*/
class MotorController{
public:

  MotorController(double resistor_one, double resistor_two, double resistor_sense,
                  double max_operating_amps, int op_amp_output_pin, int motor_power_pin,
                  int motor_direction_pin, int lock_closed_pin, int lock_open_pin);
  virtual ~MotorController(){

  }
  void closeLockState();
  void openLockState();
  bool updateState();
  int isLocked();
  int isOpen();
  void stopMotor();
  String getPowerReadings(){
    return powerReadings;
  }
private:
  unsigned long started_state = 0;
  unsigned long minimum_run_time = 50; //in ms
  unsigned long maximum_run_time = 1750;

  int locking = false;
  int unlocking = false;
  double resistor_one;
  double resistor_two;
  double resistor_sense;
  double max_operating_amps; //this is the current draw that tells us the motor is straining and so it has reached its finished replace
                             //care must be taken because this value changes with the voltage being sent to the battery.
  int lock_closed_pin;
  int lock_open_pin;
  int op_amp_output_pin; //the pin where the op amp outputs a value we can use to caclulate current draw
  int motor_power_pin; //controls the mosfet that powers the motor
  int motor_direction_pin; //the pin that controls a transistor to determine the direction of rotation via a pair of relays
  int getCurrentMonitorReading(); //returns an analog value between 1 and 4095 (we have a 12bit adc on the esp32)
  String powerReadings;

  double getPowerConsumption(); //returns power consumption in amps
};

MotorController::MotorController(double resistor_one, double resistor_two, double resistor_sense,
                                 double max_operating_amps, int op_amp_output_pin, int motor_power_pin,
                                 int motor_direction_pin, int lock_closed_pin, int lock_open_pin){
  this->locking = false;
  this->resistor_one = resistor_one;
  this->resistor_two = resistor_two;
  this->resistor_sense = resistor_sense;
  this->max_operating_amps = max_operating_amps;
  this->op_amp_output_pin = op_amp_output_pin;
  pinMode(this->op_amp_output_pin, INPUT);

  this->lock_closed_pin = lock_closed_pin;
  pinMode(this->lock_closed_pin, INPUT_PULLDOWN);

  this->lock_open_pin = lock_open_pin;
  pinMode(this->lock_open_pin, INPUT_PULLDOWN);

  this->motor_power_pin = motor_power_pin;
  pinMode(this->motor_power_pin, OUTPUT);

  this->motor_direction_pin = motor_direction_pin;
  pinMode(this->motor_direction_pin, OUTPUT);

}
int num = 0;
double MotorController::getPowerConsumption(){
  double voltage_op_amp = (((double) getCurrentMonitorReading()) / 4095.0) * 3.3;
  double amps = (voltage_op_amp * resistor_one) / (resistor_sense * resistor_two);
  if(num % 10000 == 0){
  //  Serial.println("Power Consumption (Amps) " +  String(amps));
    powerReadings += String(amps) + "\n";
  }

  return amps;
}

int MotorController::getCurrentMonitorReading(){
  return analogRead(this->op_amp_output_pin);
}



void MotorController::openLockState(){
  //when movement begins
  powerReadings = "";
  this->locking = false;
  this->unlocking = true;
  started_state = millis();
  digitalWrite(this->motor_direction_pin,LOW);
  digitalWrite(this->motor_power_pin,HIGH);

}

void MotorController::stopMotor(){
    digitalWrite(this->motor_power_pin,LOW);
    digitalWrite(this->motor_direction_pin,LOW);
    this->locking = false;
    this->unlocking = false;
}


int MotorController::isLocked(){
  int i = 0;
  while(i < 15){
    if(!digitalRead(this->lock_closed_pin)){
      return false;
    }
    i++;
  }
 return digitalRead(this->lock_closed_pin);
}


int MotorController::isOpen(){
 return digitalRead(this->lock_open_pin);
}

void MotorController::closeLockState(){
  //when movement begins
  powerReadings = "";
  started_state = millis();
  this->locking = true;
  this->unlocking = false;
  digitalWrite(this->motor_direction_pin,HIGH);
  digitalWrite(this->motor_power_pin,HIGH);

}
