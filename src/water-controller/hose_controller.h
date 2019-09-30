

#include <Arduino.h>

/**
* built with https://www.homedepot.com/p/Orbit-4-Outlet-Integrated-Watering-System-56545/205584851
* Orbit
* 4-Outlet Integrated Watering System
*/
String HOSE_NAME = "front_yard";
String OPEN_HOSE = "open_hose";
String CLOSE_HOSE = "close_hose";

#define NUM_HOSES 4
//should be open or closed in this amount of time
#define HOSE_POWER_TIME 1500

class HoseController{
public:
  HoseController(int circuit_power_pin, int power_direction_pin, int * hose_pins ){
    this->circuit_power_pin = circuit_power_pin;
    pinMode(this->circuit_power_pin,OUTPUT);
    this->power_direction_pin = power_direction_pin;
    pinMode(this->power_direction_pin,OUTPUT);
    this->power_direction = LOW;
    for(int i = 0; i < NUM_HOSES; i++){
      this->hose_pins[i] = hose_pins[i];
      pinMode(this->hose_pins[i],OUTPUT);
    }
    close_all_hoses();
  }

  void close_all_hoses();
  void set_hose_status(String * status);
private:
  int hose_pins[NUM_HOSES];
  int circuit_power_pin;
  int power_direction_pin;
  int power_direction;
  void turn_off_power();
  void turn_on_power();
  void set_power_open();
  void set_power_close();
  void set_hoses_to_power_direction();
  void set_hose_inverse(int hose_number);
  void wait_hose_time();
  void set_hose(int hose_number, bool to_open);
  void open_hose(int hose_number);
  void close_hose(int hose_number);

};

void HoseController::turn_on_power(){
    digitalWrite(circuit_power_pin,HIGH);
}

void HoseController::turn_off_power(){
    digitalWrite(circuit_power_pin,LOW);
}

//When power direction_pin is low, the relay outputs sinks to gnd
//when its is high, relay outputs 5v
void HoseController::set_power_open(){
    power_direction = HIGH;
    digitalWrite(power_direction_pin,HIGH);
}

void HoseController::set_power_close(){
    power_direction = LOW;
    digitalWrite(power_direction_pin,LOW);
}

void HoseController::set_hoses_to_power_direction(){
  for(int i = 0; i < NUM_HOSES; i++){
    digitalWrite(hose_pins[i],power_direction);
  }
}

void HoseController::set_hose_inverse(int hose_number){
  digitalWrite(hose_pins[hose_number], power_direction == HIGH ? LOW : HIGH);
}

void HoseController::wait_hose_time(){
  delay(HOSE_POWER_TIME);
}

void HoseController::set_hose(int hose_number, bool to_open){
  turn_off_power();
  delay(30);
  if(to_open){
    set_power_open();
  }else{
    set_power_close();
  }

  set_hoses_to_power_direction();
  set_hose_inverse(hose_number);
  delay(30);
  turn_on_power();
  wait_hose_time();
  turn_off_power();


}

void HoseController::open_hose(int hose_number){
  set_hose(hose_number,true);
      Serial.println("opening hose " + String(hose_number));
}

void HoseController::close_hose(int hose_number){
    set_hose(hose_number,false);
    Serial.println("closing hose " + String(hose_number));
}

void HoseController::close_all_hoses(){
  for(int i = 0; i < NUM_HOSES; i++){
    close_hose(i);
  }
}

void HoseController::set_hose_status(String * status){
  for(int i = 0; i < NUM_HOSES; i++){
    if(status[i] == OPEN_HOSE){
      open_hose(i);
    }else{
      close_hose(i);
    }

  }
}
