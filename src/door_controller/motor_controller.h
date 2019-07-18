
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
                  int motor_direction_pin);
  virtual ~MotorController(){

  }
  void closeLockState();
  void openLockState();
  void updateState();

private:
  unsigned long started_state = 0;
  unsigned long minimum_run_time = 150; //in ms
  double resistor_one;
  double resistor_two;
  double resistor_sense;
  double max_operating_amps; //this is the current draw that tells us the motor is straining and so it has reached its finished replace
                             //care must be taken because this value changes with the voltage being sent to the battery.
  int op_amp_output_pin; //the pin where the op amp outputs a value we can use to caclulate current draw
  int motor_power_pin; //controls the mosfet that powers the motor
  int motor_direction_pin; //the pin that controls a transistor to determine the direction of rotation via a pair of relays
  int getCurrentMonitorReading(); //returns an analog value between 1 and 4095 (we have a 12bit adc on the esp32)

  double getPowerConsumption(); //returns power consumption in amps
};

MotorController::MotorController(double resistor_one, double resistor_two, double resistor_sense,
                                 double max_operating_amps, int op_amp_output_pin, int motor_power_pin,
                                 int motor_direction_pin){
  this->resistor_one = resistor_one;
  this->resistor_two = resistor_two;
  this->resistor_sense = resistor_sense;
  this->max_operating_amps = max_operating_amps;
  this->op_amp_output_pin = op_amp_output_pin;
  this->motor_power_pin = motor_power_pin;
  this->motor_direction_pin = motor_direction_pin;

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
  if(this->getPowerConsumption() > max_operating_amps && (millis() - this->started_state) > this->minimum_run_time ){
    digitalWrite(this->motor_power_pin,LOW);
    digitalWrite(this->motor_direction_pin,LOW);
  }
}

void MotorController::openLockState(){
  //when movement begins
  started_state = millis();
  digitalWrite(this->motor_direction_pin,LOW);
  digitalWrite(this->motor_power_pin,HIGH);

}

void MotorController::closeLockState(){
  //when movement begins
  started_state = millis();
  digitalWrite(this->motor_direction_pin,HIGH);
  digitalWrite(this->motor_power_pin,HIGH);

}
