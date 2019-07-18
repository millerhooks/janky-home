

#include <Arduino.h>

#include <WiFi.h>

#include <HTTPClient.h>
#include <Esp.h>
#include "motor_controller.h"
#include "door_sensor.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */
#define SLEEPTIME uS_TO_S_FACTOR * TIME_TO_SLEEP
RTC_DATA_ATTR int bootCount = 0;
char * WLAN_SSID = "ssid";
char * WLAN_PASSWD = "pass";
struct RtcData{
  uint32_t crc32;   // 4 bytes
  int32_t channel;  // 4 byte,   8 in total
  uint8_t bssid[6]; // 6 bytes, 14 in total
  uint8_t padding;  // 2 byte,  16 in total
};

RTC_DATA_ATTR RtcData rtcData;

uint32_t calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while( length-- ) {
    uint8_t c = *data++;
    for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if( c & i ) {
        bit = !bit;
      }

      crc <<= 1;
      if( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }

  return crc;
}

#define RESISTOR_ONE 100.0
#define RESISTOR_TWO 1700.0
#define RESISTOR_SENSE 0.1
#define MAX_OPERATING_AMPS 1.5
#define OP_AMP_OUTPUT_PIN 10
#define MOTOR_POWER_PIN 11
#define MOTOR_DIRECTION_PIN 12
#define DOOR_LOCKED_PIN 13
#define MAGNET_PIN 5

char * central_server = "http://10.0.0.59";

MotorController door_motor(RESISTOR_ONE, RESISTOR_TWO, RESISTOR_SENSE,
                  MAX_OPERATING_AMPS, OP_AMP_OUTPUT_PIN, MOTOR_POWER_PIN,
                  MOTOR_DIRECTION_PIN, DOOR_LOCKED_PIN);
DoorSensor door_sensor(MAGNET_PIN);

String makeStatusRequest(){
  //Get the door sensor information to send to the request


  HTTPClient http;
  bool door_open = door_sensor.isOpen();
  bool door_locked = door_motor.isLocked();
  
  http.begin(central_server);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/plain");             //Specify content-type header

  String message = "door_open:" + String(door_open) + "|" + "door_locked:" + String(door_locked);
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
  
void setup() {

  bool rtcValid = false;

    // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    if( crc == rtcData.crc32 ) {
      rtcValid = true;
    }

  btStop();

  WiFi.mode( WIFI_OFF );
  //WiFi.forceSleepBegin();
  //delay( 1 );

  Serial.begin(115200);
  Serial.println("STarting!!!");

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));


  IPAddress ip( 10, 0, 0, 188 );
  IPAddress gateway( 10, 0, 0, 1);
  IPAddress subnet( 255, 255, 255, 0 );

//  WiFi.forceSleepWake();
//  delay( 1 );

  WiFi.mode( WIFI_STA );


  WiFi.persistent( false );
  // Bring up the WiFi connection
  WiFi.mode( WIFI_STA );
  WiFi.config( ip, gateway, subnet );
  if( rtcValid ) {
    // The RTC data was good, make a quick connection
    Serial.println("Reusing RTC DATA!");
    WiFi.begin( WLAN_SSID, WLAN_PASSWD, rtcData.channel, rtcData.bssid, true );
  }
  else {
    // The RTC data was not valid, so make a regular connection
    WiFi.begin( WLAN_SSID, WLAN_PASSWD );
      int wifiStatus = WiFi.status();
    if(wifiStatus == WL_CONNECTED){
      rtcData.channel = WiFi.channel();
      memcpy( rtcData.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
      rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );

    }

  }


  int retries = 0;
  int wifiStatus = WiFi.status();
  while( wifiStatus != WL_CONNECTED ) {
    retries++;
    if( retries == 100 ) {
      // Quick connect is not working, reset WiFi and try regular connection
      WiFi.disconnect();
      delay( 10 );
     // WiFi.forceSleepBegin();
     // delay( 10 );
    //  WiFi.forceSleepWake();
     // delay( 10 );
      WiFi.begin( WLAN_SSID, WLAN_PASSWD );
    }
    if( retries == 600 ) {
      // Giving up after 30 seconds and going back to sleep
      WiFi.disconnect( true );
      delay( 1 );
      WiFi.mode( WIFI_OFF );
      ESP.deepSleep( SLEEPTIME );
      return; // Not expecting this to be called, the previous call will never return.
    }
    delay( 50 );
    wifiStatus = WiFi.status();
  }

  if(retries != 0){
    rtcData.channel = WiFi.channel();
    memcpy( rtcData.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );


  }

  //TODO: do stuff here

  HTTPClient http;

  Serial.println("[HTTP] begin...\n");
  // configure traged server and url
  //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
  http.begin("http://example.com/index.html"); //HTTP
  http.addHeader("Content-Type", "text/plain"); //Specify content-type header
  Serial.println("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();


  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  delay(10); //without this delay i was not seeing this output

  WiFi.disconnect( true );
  delay( 1 );

// WAKE_RF_DISABLED to keep the WiFi radio disabled when we wake up
  ESP.deepSleep( SLEEPTIME);

  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
