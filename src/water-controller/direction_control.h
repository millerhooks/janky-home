

#include <Arduino.h>

/**
* Controls the direction in which current flows.
*/
class DirectionPowerControl : public PowerControl {
public:
  DirectionPowerControl(int direction_pin, int power_pin) : PowerControl(power_pin) {
      this->direction_pin = direction_pin;
        pinMode(this->magnet_pin, OUTPUT);
  }
  void setForwardCurrent();
  void setReverseCurrent();

private:
  int direction_pin;
};

DoorSensor::DoorSensor(int magnet_pin){
  this->magnet_pin = magnet_pin;
  pinMode(this->magnet_pin, INPUT_PULLDOWN);
}

int DoorSensor::isOpen(){
  return !digitalRead(this->magnet_pin);
}

class PowerControl{
public:
  PowerControl(int power_pin);
  void startPower();
  void shutDown(); //turn off power pin
private:

}
