#include <Arduino.h>

#include "config.h"
#include "log.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "sensorNTC.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "temperatureController.h"
#include "tft.h"

#if defined(useOTAUpdate)
  // https://github.com/SensorsIot/ESP32-OTA
  #include "OTA.h"
  #if !defined(useOTA_RTOS)
    #include <ArduinoOTA.h>
  #endif
#endif
#if defined(useTelnetStream)
#include "TelnetStream.h"
#endif

unsigned long previousMillisShortCycle = 0;
unsigned long intervalShortCycle = INTERVALSHORT;
unsigned long previousMillisMediumCycle = 0;
unsigned long intervalMediumCycle = INTERVALMEDIUM;
unsigned long previousMillisLongCycle = 0;
unsigned long intervalLongCycle = INTERVALLONG;

void setup(){
  Serial.begin(115200);
  Serial.println("");
  Log.printf("Setting things up ...\r\n");
  #ifdef useWIFI
  wifi_setup();
  wifi_enable();
  #endif
  #if defined(useOTAUpdate)
  OTA_setup("ESP32fancontroller");
  // Do not start OTA. Save heap space and start it via MQTT only when needed.
  // ArduinoOTA.begin();
  #endif
  #if defined(useTelnetStream)
  TelnetStream.begin();
  #endif
  #ifdef useTFT
  initTFT();
  #endif
  initPWMfan();
  initTacho();
  initNTC();
  initTemperatureController();
  #ifdef useMQTT
  mqtt_setup();
  #endif


  Log.printf("Setup complete\r\n");
}

void loop(){
  // functions that shall be called as often as possible
  // these functions should take care on their own that they don't nee too much time
  updateTacho();
  #if defined(useOTAUpdate) && !defined(useOTA_RTOS)
  // If you do not use FreeRTOS, you have to regulary call the handle method
  ArduinoOTA.handle();
  #endif
  // mqtt_loop() is doing mqtt keepAlive, processes incoming messages and hence triggers callback
  #ifdef useMQTT
  mqtt_loop();
  #endif

  unsigned long currentMillis = millis();

  // Short interval functions
  if ((currentMillis - previousMillisShortCycle) >= intervalShortCycle) {
    previousMillisShortCycle = currentMillis;
    updateNTC();
    #ifdef useAutomaticTemperatureControl
    setFanPWMbasedOnTemperature();
    #endif
    #ifdef useTFT
    draw_screen();
    #endif
    #ifdef useHomeassistantMQTTDiscovery
    if (((currentMillis - timerStartForHAdiscovery) >= WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT) && (timerStartForHAdiscovery != 0)) {
      mqtt_publish_hass_discovery();
    }
    #endif
  }

  // Medium interval functions
  if ((currentMillis - previousMillisMediumCycle) >= intervalMediumCycle) {
    previousMillisMediumCycle = currentMillis;
    #ifdef useMQTT
    mqtt_publish_tele1();
    mqtt_publish_tele2();
    #endif
    // doLog();
  }

  // Long interval functions
  if ((currentMillis - previousMillisLongCycle) >= intervalLongCycle) {
    previousMillisLongCycle = currentMillis;
    #ifdef useMQTT
    mqtt_publish_tele3();
    mqtt_publish_tele4();
    #endif
    doLog();
  }
}