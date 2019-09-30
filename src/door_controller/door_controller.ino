

#include <Arduino.h>



#include <HTTPClient.h>
#include <Esp.h>
#include "motor_controller.h"
#include "door_sensor.h"

#include "wifi_connector.h"

//powers everythign including this board, 555 wakes it back up
#define MOSFET_POWER_PIN 4

#define RESISTOR_ONE 68.0
#define RESISTOR_TWO 470.0
#define RESISTOR_SENSE 0.1
#define MAX_OPERATING_AMPS .95
#define OP_AMP_OUTPUT_PIN 32
#define MOTOR_POWER_PIN 25
#define MOTOR_DIRECTION_PIN 33
#define DOOR_LOCKED_PIN 27
#define DOOR_UNLOCKED_PIN 12

#define DOOR_HALL_SENSOR_PIN 26

#define DOOR_NAME "back_door"
#define OPEN_LOCK "open_lock"
#define CLOSE_LOCK "close_lock"
#define NONE_LOCK "none_lock"

//Used to power the open / closed switch on the lock and the hall sensor
#define POWER_PIN 14


char * central_server = "http://10.0.0.59:5000/set_door_status";

MotorController door_motor(RESISTOR_ONE, RESISTOR_TWO, RESISTOR_SENSE,
                  MAX_OPERATING_AMPS, OP_AMP_OUTPUT_PIN, MOTOR_POWER_PIN,
                  MOTOR_DIRECTION_PIN, DOOR_LOCKED_PIN, DOOR_UNLOCKED_PIN);
DoorSensor door_sensor(DOOR_HALL_SENSOR_PIN);

String makeStatusRequest(){
  //Get the door sensor information to send to the request


  HTTPClient http;
  int door_open = door_sensor.isOpen();
  int door_locked = door_motor.isLocked();
  int door_unlocked = door_motor.isOpen();
  if(door_open){
  //  Serial.println("door is open");
  }else{
    //    Serial.println("door is closed");
  }

  if(door_locked){
    //Serial.println("door is locked");
  }else{
    //Serial.println("door is not locked");
  }
  http.begin(central_server);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/json");             //Specify content-type header

  String message = "{\"door_open\":" + String(door_open) + "," + "\"door_locked\":" + String(door_locked) + "," + "\"door_unlocked\":" + String(door_unlocked)  + "," + "\"door_name\":\"" + String(DOOR_NAME) +"\"" + "," + "\"power_readings\":\"" + door_motor.getPowerReadings()  + "\"}";
  //Serial.println(message);
  int responseCode = http.POST(message);   //send POST request

  if(responseCode>0){

    String response = http.getString();                       //Get the response to the request
    //So basically we need to return a few values here
    //the lock can be in one of a few states
    //either it can be in lock, unlock , do nothing
    return response;

  }else{

  //  Serial.print("Error on sending POST: ");
  //  Serial.println(responseCode);
    return "error";
  }

  http.end();  //Free resources


}


void setup() {
  pinMode(MOSFET_POWER_PIN,OUTPUT);
  digitalWrite(MOSFET_POWER_PIN,HIGH);

  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  connectWifi();

  String result = makeStatusRequest();
//  Serial.println(result);
  if(result == OPEN_LOCK){
    door_motor.openLockState();
    while(!door_motor.isOpen()){
        //could in theory do shit here while it opens
    }
    door_motor.stopMotor();
    makeStatusRequest();
  }else if (result == CLOSE_LOCK){
    door_motor.closeLockState();
    while(!door_motor.isLocked()){
        //could in theory do shit here while it opens
    }
  //  Serial.println("door should be locked");
    door_motor.stopMotor();
    makeStatusRequest();
  }else{

  }
  digitalWrite(POWER_PIN,LOW);
  //Serial.println(result);

  disconnectAndSleep();

}

void loop() {
  //This is not going to be called
}
