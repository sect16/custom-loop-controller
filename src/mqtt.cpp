#include <Arduino.h>
#include <ArduinoOTA.h>
#if defined(ESP32)
  #include <WiFi.h>
#endif
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "mqtt.h"
#include "sensorNTC.h"
#include "temperatureController.h"
#include "tft.h"
#include "wifiCommunication.h"

#ifdef useMQTT
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
// https://github.com/knolleary/pubsubclient
// https://gist.github.com/igrr/7f7e7973366fc01d6393

unsigned long reconnectInterval = 5000;
// in order to do reconnect immediately ...
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;
#ifdef useHomeassistantMQTTDiscovery
unsigned long timerStartForHAdiscovery = 1;
#endif

void callback(char *topic, byte *payload, unsigned int length);

WiFiClient wifiClient;

PubSubClient mqttClient(MQTT_SERVER, MQTT_SERVER_PORT, callback, wifiClient);

bool checkMQTTconnection();

void mqtt_setup() {
#ifdef useHomeassistantMQTTDiscovery
  // Set buffer size to allow hass discovery payload
  mqttClient.setBufferSize(1280);
#endif
}

void mqtt_loop() {
  if (!mqttClient.connected()) {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      checkMQTTconnection();
    }
  }

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

bool checkMQTTconnection() {
  if (wifiIsDisabled || !WiFi.isConnected() || !mqttClient.connected()) {
    if (!wifiIsDisabled && WiFi.isConnected() && !mqttClient.connected()) {
      #if !defined(useHomeassistantMQTTDiscovery)
      if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS)) {
      #else
      if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS, HASSFANSTATUSTOPIC, 0, 1, HASSSTATUSOFFLINEPAYLOAD)) {
      #endif
        Log.printf("  Successfully connected to MQTT broker\r\n");
        mqttClient.subscribe(MQTTCMNDTEMPMAX);
        mqttClient.subscribe(MQTTCMNDTEMPMIN);
        mqttClient.subscribe(MQTTCMNDTEMPOFFSET);
        mqttClient.subscribe(MQTTCMNDMANUAL);
        mqttClient.subscribe(MQTTCMNDPWMMANUAL);
        mqttClient.subscribe(MQTTCMNDPWMMINIMUM);
        mqttClient.subscribe(MQTTCMNDFANMODE);
        mqttClient.subscribe(MQTTCMNDPWMSTEP);
        #if defined(useOTAUpdate)
        mqttClient.subscribe(MQTTCMNDOTA);
        #endif
        mqttClient.subscribe(MQTTCMNDRESTART);
        #if defined(useHomeassistantMQTTDiscovery)
        mqttClient.subscribe(HASSSTATUSTOPIC);
        #endif
      } else {
        Log.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
      }
    } else {
      Log.printf("  No connection to MQTT server, because WiFi is not connected.\r\n");
    }
    return false;
  }
  return true;
}

bool publishMQTTMessage(const char *topic, const char *payload, boolean retained) {
  if (wifiIsDisabled)
    return false;

  if (checkMQTTconnection()) {
    //  Log.printf("Sending mqtt payload to topic \"%s\": %s\r\n", topic, payload);

    if (mqttClient.publish(topic, payload, retained)) {
      // Log.printf("Publish ok\r\n");
      return true;
    } else {
      Log.printf("Publish failed\r\n");
    }
  } else {
    Log.printf("  Cannot publish mqtt message, because checkMQTTconnection failed (WiFi or mqtt is not connected)\r\n");
  }
  return false;
}

bool publishMQTTMessage(const char *topic, const char *payload) {
  return publishMQTTMessage(topic, payload, false);
}

bool mqtt_publish_stat_mode() {
  return publishMQTTMessage(MQTTSTATFANMODE, modeIsOff ? MQTTFANMODEOFFPAYLOAD : MQTTFANMODEFANONLYPAYLOAD);
};
bool mqtt_publish_stat_tempMax() {
  return publishMQTTMessage(MQTTSTATTEMPMAX, ((String)temperatureMax).c_str());
};
bool mqtt_publish_stat_tempMin() {
  return publishMQTTMessage(MQTTSTATTEMPMIN, ((String)temperatureMin).c_str());
};
bool mqtt_publish_stat_tempOffset() {
  return publishMQTTMessage(MQTTSTATTEMPOFFSET, ((String)temperatureOffset).c_str());
};
bool mqtt_publish_stat_pwmManual() {
  return publishMQTTMessage(MQTTSTATPWMMANUAL, ((String)pwmManual).c_str());
};
bool mqtt_publish_stat_manual() {
  return publishMQTTMessage(MQTTSTATMANUAL, ((String)getManual()).c_str());
};
bool mqtt_publish_stat_pwmMinimum() {
  return publishMQTTMessage(MQTTSTATPWMMINIMUM, ((String)pwmMinimum).c_str());
};
bool mqtt_publish_stat_pwmStep() {
  return publishMQTTMessage(MQTTSTATPWMSTEP, ((String)pwmStep).c_str());
};

#ifdef useHomeassistantMQTTDiscovery
bool mqtt_publish_hass_discovery() {
  Log.printf("Will send HA discovery now.\r\n");
  bool error = false;
  error = !publishMQTTMessage(HASSCLIMATEDISCOVERYTOPIC, HASSCLIMATEDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORTEMPERATUREHOTDISCOVERYTOPIC, HASSSENSORTEMPERATUREHOTDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORTEMPERATURECOLDDISCOVERYTOPIC, HASSSENSORTEMPERATURECOLDDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBEROFFSETDISCOVERYTOPIC, HASSNUMBEROFFSETDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSWITCHMANUALDISCOVERYTOPIC, HASSSWITCHMANUALDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMMANUALDISCOVERYTOPIC, HASSNUMBERPWMMANUALDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMMINIMUMDISCOVERYTOPIC, HASSNUMBERPWMMINIMUMDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM1DISCOVERYTOPIC, HASSSENSORRPM1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM2DISCOVERYTOPIC, HASSSENSORRPM2DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM3DISCOVERYTOPIC, HASSSENSORRPM3DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM4DISCOVERYTOPIC, HASSSENSORRPM4DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORPWMDISCOVERYTOPIC, HASSSENSORPWMDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMSTEPDISCOVERYTOPIC, HASSNUMBERPWMSTEPDISCOVERYPAYLOAD);
  if (!error)
    delay(1000);
  // publish that we are online. Remark: offline is sent via last will retained message
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, "", true);
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, HASSSTATUSONLINEPAYLOAD);

  if (!error) {
    delay(1000);
  }
  // MODE????

  // that's not really part of the discovery message, but this enables the climate slider in HA and immediately provides all values
  error = error || !mqtt_publish_stat_mode();
  error = error || !mqtt_publish_stat_manual();
  error = error || !mqtt_publish_stat_pwmManual();
  error = error || !mqtt_publish_stat_tempMin();
  error = error || !mqtt_publish_stat_tempMax();
  error = error || !mqtt_publish_stat_tempOffset();
  error = error || !mqtt_publish_stat_pwmMinimum();
  error = error || !mqtt_publish_stat_pwmStep();
  error = error || !mqtt_publish_tele1();
  error = error || !mqtt_publish_tele2();
  error = error || !mqtt_publish_tele3();
  error = error || !mqtt_publish_tele4();
  if (!error) {
    // will not resend discovery as long as timerStartForHAdiscovery == 0
    Log.printf("Will set timer to 0 now, this means I will not send discovery again.\r\n");
    timerStartForHAdiscovery = 0;
  } else {
    Log.printf("Some error occured while sending discovery. Will try again.\r\n");
  }
  return !error;
}
#endif

// Temperature telemetry
bool mqtt_publish_tele1() {
  bool error = false;
  // maximum message length 128 Byte
  String payload = "{";/**/
  for (int i = 0; i < sizeof(temperature) / sizeof(float); i++) {
      if (payload != "{") payload += ",";
      if (i == 0) payload += "\"TempHot\":";
      else payload += "\"TempCold\":";
      payload += temperature[i];
  }
  payload += "}";
  if (payload != "{}") error = !publishMQTTMessage(MQTTTELESTATE1, payload.c_str());
  return !error;
}

// Tacho/PWM telemetry
bool mqtt_publish_tele2() {
  bool error = false;
  // maximum message length 128 Byte
  String payload = "{";
  // Fan
  for (int i = 0; i < sizeof(rpm) / sizeof(int); i++) {
      if (payload != "{") payload += ",";
      payload += "\"rpm" + (String)i + "\":" + rpm[i];
  }
  if (payload != "{") payload += ",";
  payload += "\"pwm\":";
  payload += pwmValue;
  payload += "}";
  if (payload != "{}") error = !publishMQTTMessage(MQTTTELESTATE2, payload.c_str());
  return !error;
}

// WiFi telemetry
bool mqtt_publish_tele3() {
  bool error = false;
  // maximum message length 128 Byte
  String payload = "";
  // WiFi
  payload = "";
  payload += "{\"wifiRSSI\":";
  payload += WiFi.RSSI();
  payload += ",\"wifiChan\":";
  payload += WiFi.channel();
  payload += ",\"wifiSSID\":";
  payload += WiFi.SSID();
  payload += ",\"wifiBSSID\":";
  payload += WiFi.BSSIDstr();
#if defined(WIFI_KNOWN_APS)
  payload += ",\"wifiAP\":";
  payload += accessPointName;
#endif
  payload += ",\"IP\":";
  payload += WiFi.localIP().toString();
  payload += ",\"Hostname\":";
  payload += WiFi.getHostname();
  payload += "}";
  error = !publishMQTTMessage(MQTTTELESTATE3, payload.c_str());
  return !error;
}

// ESP32 telemetry
bool mqtt_publish_tele4() {
  bool error = false;
  // maximum message length 128 Byte
  String payload = "";
  // ESP32 stats
  payload = "";
  payload += "{\"up\":";
  payload += String(millis());
  payload += ",\"heapSize\":";
  payload += String(ESP.getHeapSize());
  payload += ",\"heapFree\":";
  payload += String(ESP.getFreeHeap());
  payload += ",\"heapMin\":";
  payload += String(ESP.getMinFreeHeap());
  payload += ",\"heapMax\":";
  payload += String(ESP.getMaxAllocHeap());
  payload += "}";
  error = !publishMQTTMessage(MQTTTELESTATE4, payload.c_str());
  return !error;
}

void callback(char *topic, byte *payload, unsigned int length) {
  // handle message arrived
  std::string strPayload(reinterpret_cast<const char *>(payload), length);

  Log.printf("MQTT message arrived [%s] %s\r\n", topic, strPayload.c_str());

  String topicReceived(topic);
  String topicCmndTempActual(MQTTCMNDTEMPACTUAL);
  String topicCmndTempMax(MQTTCMNDTEMPMAX);
  String topicCmndTempMin(MQTTCMNDTEMPMIN);
  String topicCmndTempOffset(MQTTCMNDTEMPOFFSET);
  String topicCmndFanMode(MQTTCMNDFANMODE);
  String topicCmndManual(MQTTCMNDMANUAL);
  String topicCmndPwmManual(MQTTCMNDPWMMANUAL);
  String topicCmndPwmMinimum(MQTTCMNDPWMMINIMUM);
  String topicCmndRestart(MQTTCMNDRESTART);
  String topicCmndPwmStep(MQTTCMNDPWMSTEP);
  #if defined(useOTAUpdate)
  String topicCmndOTA(MQTTCMNDOTA);
  #endif
  #if defined(useHomeassistantMQTTDiscovery)
  String topicHaStatus(HASSSTATUSTOPIC);
  #endif
  if (topicReceived == topicCmndTempMax) {
    Log.printf("Setting tempMax via mqtt\r\n");
    float num_float = ::atof(strPayload.c_str());
    Log.printf("new tempMax: %.2f\r\n", num_float);
    setTemperatureMax(num_float, false);
  } else if (topicReceived == topicCmndTempMin) {
    Log.printf("Setting tempMin via mqtt\r\n");
    float num_float = ::atof(strPayload.c_str());
    Log.printf("new tempMin: %.2f\r\n", num_float);
    setTemperatureMin(num_float, false);
  } else if (topicReceived == topicCmndTempOffset) {
    Log.printf("Setting tempOffset via mqtt\r\n");
    float num_float = ::atof(strPayload.c_str());
    Log.printf("new tempOffset: %.2f\r\n", num_float);
    setTemperatureOffset(num_float, false);
  } else if (topicReceived == topicCmndFanMode) {
    Log.printf("Setting HVAC mode from HA received via mqtt\r\n");
    if (strPayload == MQTTFANMODEFANONLYPAYLOAD) {
      Log.printf("  Will turn fan into \"fan_only\" mode\r\n");
      setFanMode(false, true);
    } else if (strPayload == MQTTFANMODEOFFPAYLOAD) {
      Log.printf("  Will switch fan off\r\n");
      setFanMode(true, true);
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
    #if defined(useOTAUpdate)
  } else if (topicReceived == topicCmndOTA) {
    if (strPayload == "ON") {
      Log.printf("MQTT command TURN ON OTA received\r\n");
      ArduinoOTA.begin();
    } else if (strPayload == "OFF") {
      Log.printf("MQTT command TURN OFF OTA received\r\n");
      ArduinoOTA.end();
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
    #endif
  } else if (topicReceived == topicCmndManual) {
    if (strPayload == "ON") {
      Log.printf("MQTT command TURN ON MANUAL received\r\n");
      setManual(true, false);
    } else if (strPayload == "OFF") {
      Log.printf("MQTT command TURN OFF MANUAL received\r\n");
      setManual(false, false);
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
  } else if (topicReceived == topicCmndPwmManual) {
    Log.printf("Setting pwmManual via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmManual: %d\r\n", num_int);
    setPwmManual(num_int, false);
    setFanPWMbasedOnTemperature();
  } else if (topicReceived == topicCmndPwmMinimum) {
    Log.printf("Setting pwmMinimum via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmMinimum: %d\r\n", num_int);
    setPwmMinimum(num_int, false);
  } else if (topicReceived == topicCmndRestart) {
    Log.printf("Software reset\r\n");
    sleep(1);
    ESP.restart();
  } else if (topicReceived == topicCmndPwmStep) {
    Log.printf("Setting pwmStep via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmStep: %d\r\n", num_int);
    setPwmStep(num_int, false);
  #if defined(useHomeassistantMQTTDiscovery)
  } else if (topicReceived == topicHaStatus) {
    if (strPayload == HASSSTATUSONLINEPAYLOAD) {
      Log.printf("HA status online received. This means HA has restarted. Will send discovery again in some seconds as defined in config.h\r\n");
      // set timer so that discovery will be resent after some seconds (as defined in config.h)
      timerStartForHAdiscovery = millis();
      // Very unlikely. Can only happen if millis() overflowed max unsigned long every approx. 50 days
      if (timerStartForHAdiscovery == 0) {
        timerStartForHAdiscovery = 1;
      }
    } else if (strPayload == HASSSTATUSOFFLINEPAYLOAD) {
      Log.printf("HA status offline received. Nice to know. Currently we don't react to this.\r\n");
    } else {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
  #endif
  }
}
#endif