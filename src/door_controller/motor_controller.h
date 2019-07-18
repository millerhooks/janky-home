
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
                  int motor_direction_pin, int lock_closed_pin);
  virtual ~MotorController(){

  }
  void closeLockState();
  void openLockState();
  void updateState();
  bool isLocked();
private:
  unsigned long started_state = 0;
  unsigned long minimum_run_time = 150; //in ms

  bool locking;
  double resistor_one;
  double resistor_two;
  double resistor_sense;
  double max_operating_amps; //this is the current draw that tells us the motor is straining and so it has reached its finished replace
                             //care must be taken because this value changes with the voltage being sent to the battery.
  int lock_closed_pin;
  int op_amp_output_pin; //the pin where the op amp outputs a value we can use to caclulate current draw
  int motor_power_pin; //controls the mosfet that powers the motor
  int motor_direction_pin; //the pin that controls a transistor to determine the direction of rotation via a pair of relays
  int getCurrentMonitorReading(); //returns an analog value between 1 and 4095 (we have a 12bit adc on the esp32)
  
  double getPowerConsumption(); //returns power consumption in amps
};

MotorController::MotorController(double resistor_one, double resistor_two, double resistor_sense,
                                 double max_operating_amps, int op_amp_output_pin, int motor_power_pin,
                                 int motor_direction_pin, int lock_closed_pin){
  this->locking = false;
  this->resistor_one = resistor_one;
  this->resistor_two = resistor_two;
  this->resistor_sense = resistor_sense;
  this->max_operating_amps = max_operating_amps;
  this->op_amp_output_pin = op_amp_output_pin;
  this->lock_closed_pin = lock_closed_pin;
  pinMode(this->lock_closed_pin, INPUT_PULLDOWN);
  
  this->motor_power_pin = motor_power_pin;
  pinMode(this->op_amp_output_pin, OUTPUT);
  
  this->motor_direction_pin = motor_direction_pin;
  pinMode(this->op_amp_output_pin, OUTPUT);
 
}

double MotorController::getPowerConsumption(){
  double voltage_op_amp = (((double) getCurrentMonitorReading()) / 4095.0) * 3.3;
  double amps = (voltage_op_amp * resistor_one) / (resistor_sense * resistor_two);
  return amps;
}

int MotorController::getCurrentMonitorReading(){
  return analogRead(this->op_amp_output_pin);
}

void MotorController::updateState(){
  if(((this->getPowerConsumption() > max_operating_amps) && ((millis() - this->started_state) > this->minimum_run_time )) || (this->locking && this->isLocked())){
    digitalWrite(this->motor_power_pin,LOW);
    digitalWrite(this->motor_direction_pin,LOW);
    this->locking = false;
  }
}

void MotorController::openLockState(){
  //when movement begins
  this->locking = true;
  started_state = millis();
  digitalWrite(this->motor_direction_pin,LOW);
  digitalWrite(this->motor_power_pin,HIGH);

}

bool MotorController::isLocked(){
  return digitalRead(this->lock_closed_pin);
}
void MotorController::closeLockState(){
  //when movement begins
  started_state = millis();

  digitalWrite(this->motor_direction_pin,HIGH);
  digitalWrite(this->motor_power_pin,HIGH);

}
