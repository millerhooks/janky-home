

#include <Arduino.h>


class DoorSensor{
public:
  DoorSensor(int magnet_pin);
  bool isOpen();
private:
  int magnet_pin;
};

DoorSensor::DoorSensor(int magnet_pin){
  this->magnet_pin = magnet_pin;
  pinMode(this->magnet_pin, INPUT_PULLDOWN);
}

bool DoorSensor::isOpen(){
  return digitalRead(this->magnet_pin);
}
