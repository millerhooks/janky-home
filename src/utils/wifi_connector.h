

#include <WiFi.h>

//file has to be copied in each time... can't use relative paths in arduino.

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 15        /* Time ESP32 will go to sleep (in seconds) */
#define SLEEPTIME uS_TO_S_FACTOR * TIME_TO_SLEEP
RTC_DATA_ATTR int bootCount = 0;
char * WLAN_SSID = "linksys2";
char * WLAN_PASSWD = "srilanka";
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

void connectWifi(){
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
}


void disconnectAndSleep(){
  esp_sleep_enable_timer_wakeup(SLEEPTIME);
  Serial.println("Going to sleep now");
  delay(10); //without this delay i was not seeing this output

  WiFi.disconnect( true );
  delay( 1 );

// WAKE_RF_DISABLED to keep the WiFi radio disabled when we wake up
  ESP.deepSleep( SLEEPTIME);

  Serial.println("This will never be printed");
}
