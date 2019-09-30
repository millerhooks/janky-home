

#include <Arduino.h>



#include <HTTPClient.h>
#include <Esp.h>
#include "hose_controller.h"


#include "wifi_connector.h"

#define CIRCUIT_POWER_PIN 23
#define POWER_DIRECTION_PIN 13
const int HOSE_PIN_1 = 32;
const int HOSE_PIN_2 = 33;
const int HOSE_PIN_3 = 25;
const int HOSE_PIN_4 = 26;



//used for sensors
#define POWER_PIN 14


char * central_server = "http://10.0.0.59:5000/set_hose_status";

int hoses[4] = {HOSE_PIN_1,HOSE_PIN_2,HOSE_PIN_3,HOSE_PIN_4};
HoseController controller(CIRCUIT_POWER_PIN,POWER_DIRECTION_PIN, hoses);

String makeStatusRequest(){
  //Get the door sensor information to send to the request


  HTTPClient http;



  http.begin(central_server);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/json");             //Specify content-type header

  String message = "{\"hose_name\":\"" + String(HOSE_NAME) + "\"}";
  Serial.println(message);
  int responseCode = http.POST(message);   //send POST request

  if(responseCode>0){

    String response = http.getString();                       //Get the response to the request
    //So basically we need to return a few values here
    //the lock can be in one of a few states
    //either it can be in lock, unlock , do nothing
    return response;

  }else{

    Serial.print("Error on sending POST: ");
    Serial.println(responseCode);
    return "error";
  }

  http.end();  //Free resources


}

String hose_status[4] = {CLOSE_HOSE,CLOSE_HOSE,CLOSE_HOSE,CLOSE_HOSE};
void convert_result(String result){
  int start_index = 0;
  hose_status[0] = result.substring(start_index,result.indexOf(","));
  start_index = result.indexOf(",") + 1;
  hose_status[1] = result.substring(start_index,result.indexOf(",",start_index));
  Serial.println(hose_status[1]);
  start_index = result.indexOf(",",start_index) + 1;
  hose_status[2] = result.substring(start_index,result.indexOf(",",start_index));
  Serial.println(hose_status[2]);
  start_index = result.indexOf(",",start_index) + 1;
  hose_status[3] = result.substring(start_index);
  Serial.println(hose_status[3]);
}


void setup() {



  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  connectWifi();

  String result = makeStatusRequest();
  if(result == "error"){
    controller.close_all_hoses(); //no response assume we should close stuff :)
    digitalWrite(POWER_PIN,LOW);

  }else{
    Serial.println(result);
    convert_result(result);
    controller.set_hose_status(hose_status);

    digitalWrite(POWER_PIN,LOW);


  }
 disconnectAndSleep();

}

void loop() {
  //This is not going to be called
}
