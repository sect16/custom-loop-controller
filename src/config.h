/*
  Before changing anything in this file, consider to copy file "config_override_example.h" to file "config_override.h" and to do your changes there.
  Doing so, you will
  - keep your credentials secret
  - most likely never have conflicts with new versions of this file
  Any define in CAPITALS can be moved to "config_override.h".
  All defines having BOTH lowercase and uppercase MUST stay in "config.h". They define the mode the "esp32 fan controller" is running in.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <driver/gpio.h>
#include <esp32-hal-gpio.h>

#define INTERVALSHORT   1000
#define INTERVALMEDIUM  2000
#define INTERVALLONG    10000

#define useWIFI
#define useMQTT
#define useTFT
  #ifdef useTFT
    // --- choose which display to use. Activate only one. -----------------------------------------------
    #define DRIVER_ILI9341       // Generic 2.8 inch color TFT based in ILI9341, 320x240
  #endif
#endif

// --- Fan parameters ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
#define PWMPIN               GPIO_NUM_17  // Single pin output to all fans
#define PWMFREQ              25000
#define PWMCHANNEL           0
#define PWMRESOLUTION        8
#define FANMAXRPM1           2500         // only used for showing at how many percent fan is running
#define FANMAXRPM2           2500         // only used for showing at how many percent fan is running
#define FANMAXRPM3           2500         // only used for showing at how many percent fan is running

// fanTacho
#define TACHOPIN1                             GPIO_NUM_16 // Fan 1
#define TACHOPIN2                             GPIO_NUM_32 // Fan 2
#define TACHOPIN3                             GPIO_NUM_33 // Fan 3
#define TACHOPIN4                             GPIO_NUM_25 // Water Flow Meter
#define TACHOINPUTS                           4    // Total number of TACHO inputs for iterating
#define TACHOUPDATECYCLE                      1000 // how often tacho speed shall be determined, in milliseconds
#define NUMBEROFINTERRUPSINONESINGLEROTATION  2    // Number of interrupts ESP32 sees on tacho signal on a single fan rotation. All the fans I've seen trigger two interrups.
#define PWMMINIMUM                            52   // Lowest pwm value the temperature controller should use to set fan minimum speed.
#define PWMINITIAL                            0    // initial pwm fan speed on startup (0 <= value <= 255)
#define PWMSTEP                               1    // PWM steps per second (1 <= value <= 255)

// --- NTC thermistor parameters ----------------------------------------------------------------------------------------------------------------------------

// Default temperature values on startup
#define INITIALTEMPERATUREMAX 40.0
#define INITIALTEMPERATUREMIN 36.0
#define INITIALTEMPERATUREOFFSET -5.6

// which analog pin to connect
#define THERMISTORPIN1 34
#define THERMISTORPIN2 35
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 13
#define NMEDIAN 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3435
// the value of the 'other' resistor
#define SERIESRESISTOR 10000
// Temperature in Kelvin for 25 degree Celsius
#define KELVINNOMINAL 298.15

// --- tft parameters----------------------------------------------------------------------------------------------------------------------------------

#ifdef useTFT
#define TFT_CS                GPIO_NUM_5    //diplay chip select
#define TFT_DC                GPIO_NUM_4    //display d/c
#define TFT_RST               GPIO_NUM_22   //display reset
#define TFT_MOSI              GPIO_NUM_23   //diplay MOSI
#define TFT_CLK               GPIO_NUM_18   //display clock
#define LED_ON                HIGH          // override it in file "config_override.h"

#ifdef DRIVER_ILI9341
#define TFT_LED               GPIO_NUM_15   //display background LED
#define TFT_MISO              GPIO_NUM_19   //display MISO
#define TFT_ROTATION          3 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
#ifdef DRIVER_ST7735
#define TFT_ROTATION          1 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
// #define TFT_PWM_FREQUENCY     1000
// #define TFT_PWM_CHANNEL       5
// #define TFT_PWM_BIT           8
#define TFT_PWM_DUTY          30 // 1-255 LED brightness

#endif

// --- wifi parameters ---------------------------------------------------------------------------------------------------------------------------------

#ifdef useWIFI
#define HOSTNAME      "CustomHostname"                 // override it in file "config_override.h"
#define WIFI_SSID     "YourWifiSSID"           // override it in file "config_override.h"
#define WIFI_PASSWORD "YourWifiPassword"       // override it in file "config_override.h"
//#define WIFI_KNOWN_APS_COUNT 2
//#define WIFI_KNOWN_APS \
//  { "00:11:22:33:44:55", "Your AP 2,4 GHz"}, \
//  { "66:77:88:99:AA:BB", "Your AP 5 GHz"}
#endif

// --- OTA Update ---------------------------------------------------------------------------------------------------------------------------
#define useOTAUpdate
// #define useOTA_RTOS     // not recommended because of additional 10K of heap space needed

#if !defined(useWIFI) && defined(useOTAUpdate)
static_assert(false, "\"#define useOTAUpdate\" is only possible with \"#define useWIFI\"");
#endif
#if !defined(ESP32) && defined(useOTA_RTOS)
static_assert(false, "\"#define useOTA_RTOS\" is only possible with ESP32");
#endif
#if defined(useOTA_RTOS) && !defined(useOTAUpdate)
static_assert(false, "You cannot use \"#define useOTA_RTOS\" without \"#define useOTAUpdate\"");
#endif

#define useSerial
#define useTelnetStream

// --- Home Assistant MQTT discovery --------------------------------------------------------------------------------------------------------
/* If you are using Home Assistant, you can activate auto discovery of the climate/fan and sensors.
   Please also see https://github.com/KlausMu/esp32-fan-controller/wiki/06-Home-Assistant
   If needed, e.g. if you are using more than one esp32 fan controller, please adjust mqtt settings further down in this file */
#if defined(useMQTT)
#define useHomeassistantMQTTDiscovery
#endif
#if defined(useHomeassistantMQTTDiscovery) && !defined(useMQTT)
static_assert(false, "You have to use \"#define useMQTT\" when having \"#define useHomeassistantMQTTDiscovery\"");
#endif

// --- mqtt ---------------------------------------------------------------------------------------------------------------------------------
/*
  ----- IMPORTANT -----
  ----- MORE THAN ONE INSTANCE OF THE ESP32 FAN CONTROLLER -----
  If you want to have more than one instance of the esp32 fan controller in your network, every instance has to have it's own unique mqtt topcics (and IDs and name in HA, if you are using HA)
  For this the define UNIQUE_DEVICE_FRIENDLYNAME and UNIQUE_DEVICE_NAME is used. You can keep it unchanged if you have only one instance in your network.
  Otherwise you can change it to e.g. "Fan Controller 2" and "esp32_fan_controller_2"
*/
#ifdef useMQTT
#define UNIQUE_DEVICE_FRIENDLYNAME "Fan Controller"       // override it in file "config_override.h"
#define UNIQUE_DEVICE_NAME         "esp32_fan_controller" // override it in file "config_override.h"

#define MQTT_SERVER            "IPAddressOfYourBroker"    // override it in file "config_override.h"
#define MQTT_SERVER_PORT       1883                       // override it in file "config_override.h"
#define MQTT_USER              "MQTTUser"                         // override it in file "config_override.h"
#define MQTT_PASS              "MQTTPassword"                         // override it in file "config_override.h"
#define MQTT_CLIENTNAME        UNIQUE_DEVICE_NAME

/*
For understanding when "cmnd", "stat" and "tele" is used, have a look at how Tasmota is doing it.
https://tasmota.github.io/docs/MQTT
https://tasmota.github.io/docs/openHAB/
https://www.openhab.org/addons/bindings/mqtt.generic/
https://www.openhab.org/addons/bindings/mqtt/
https://community.openhab.org/t/itead-sonoff-switches-and-sockets-cheap-esp8266-wifi-mqtt-hardware/15024
for debugging:
mosquitto_sub -h localhost -t "esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/climate/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/fan/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/sensor/esp32_fan_controller/#" -v
*/

#define MQTTCMNDTEMPACTUAL        UNIQUE_DEVICE_NAME "/cmnd/TEMPACTUAL"
#define MQTTSTATTEMPACTUAL        UNIQUE_DEVICE_NAME "/stat/TEMPACTUAL"
#define MQTTCMNDTEMPMAX           UNIQUE_DEVICE_NAME "/cmnd/TEMPMAX"
#define MQTTSTATTEMPMAX           UNIQUE_DEVICE_NAME "/stat/TEMPMAX"
#define MQTTCMNDTEMPMIN           UNIQUE_DEVICE_NAME "/cmnd/TEMPMIN"
#define MQTTSTATTEMPMIN           UNIQUE_DEVICE_NAME "/stat/TEMPMIN"
#define MQTTCMNDTEMPOFFSET        UNIQUE_DEVICE_NAME "/cmnd/TEMPOFFSET"
#define MQTTSTATTEMPOFFSET        UNIQUE_DEVICE_NAME "/stat/TEMPOFFSET"
#define MQTTCMNDMANUAL            UNIQUE_DEVICE_NAME "/cmnd/MANUAL"
#define MQTTSTATMANUAL            UNIQUE_DEVICE_NAME "/stat/MANUAL"
#define MQTTCMNDPWMMANUAL         UNIQUE_DEVICE_NAME "/cmnd/PWMMANUAL"
#define MQTTSTATPWMMANUAL         UNIQUE_DEVICE_NAME "/stat/PWMMANUAL"
#define MQTTCMNDPWMMINIMUM        UNIQUE_DEVICE_NAME "/cmnd/PWMMINIMUM"
#define MQTTSTATPWMMINIMUM        UNIQUE_DEVICE_NAME "/stat/PWMMINIMUM"
#define MQTTCMNDPWMSTEP           UNIQUE_DEVICE_NAME "/cmnd/PWMSTEP"
#define MQTTSTATPWMSTEP           UNIQUE_DEVICE_NAME "/stat/PWMSTEP"
// https://www.home-assistant.io/integrations/climate.mqtt/#mode_command_topic
// https://www.home-assistant.io/integrations/climate.mqtt/#mode_state_topic
// note: it is not guaranteed that fan stops if pwm is set to 0
#define MQTTCMNDFANMODE           UNIQUE_DEVICE_NAME "/cmnd/MODE"   // can be "off" and "fan_only"
#define MQTTSTATFANMODE           UNIQUE_DEVICE_NAME "/stat/MODE"
#define MQTTFANMODEOFFPAYLOAD     "off"
#define MQTTFANMODEFANONLYPAYLOAD "fan_only"
#define MQTTCMNDRESTART           UNIQUE_DEVICE_NAME "/cmnd/RESTART"

#if defined(useOTAUpdate)
#define MQTTCMNDOTA            UNIQUE_DEVICE_NAME "/cmnd/OTA"
#endif

#define MQTTTELESTATE1         UNIQUE_DEVICE_NAME "/tele/STATE1"
#define MQTTTELESTATE2         UNIQUE_DEVICE_NAME "/tele/STATE2"
#define MQTTTELESTATE3         UNIQUE_DEVICE_NAME "/tele/STATE3"
#define MQTTTELESTATE4         UNIQUE_DEVICE_NAME "/tele/STATE4"

#if defined(useHomeassistantMQTTDiscovery)
/* see
   https://www.home-assistant.io/integrations/mqtt
   https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
   https://www.home-assistant.io/integrations/mqtt/#discovery-messages
   https://www.home-assistant.io/integrations/mqtt/#birth-and-last-will-messages
*/
#define HASSSTATUSTOPIC                       "homeassistant/status"    // can be "online" and "offline"
#define HASSSTATUSONLINEPAYLOAD               "online"
#define HASSSTATUSOFFLINEPAYLOAD              "offline"
/*
   When HA sends status online, we have to resent the discovery. But we have to wait some seconds, otherwise HA will not recognize the mqtt messages.
   If you have HA running on a weak mini computer, you may have to increase the waiting time. Value is in ms.
   Remark: the whole discovery process will be done in the following order:
   discovery, delay(1000), status=online, delay(1000), all inital values
*/
#define WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT   1000

// see https://www.home-assistant.io/integrations/climate.mqtt/#availability_topic
#define HASSFANSTATUSTOPIC                    UNIQUE_DEVICE_NAME "/stat/STATUS" // can be "online" and "offline"

// The define HOMEASSISTANTDEVICE will be reused in all discovery payloads for the climate/fan and the sensors. Everything should be contained in the same device.
#define HOMEASSISTANTDEVICE                        "\"dev\":{\"name\":\"" UNIQUE_DEVICE_FRIENDLYNAME "\", \"model\":\"" UNIQUE_DEVICE_NAME "\", \"identifiers\":[\"" UNIQUE_DEVICE_NAME "\"], \"manufacturer\":\"Chin Pin Hon\"}"
#define HASSTEMPCLASS                              "\"unit_of_measurement\":\"°C\",\"device_class\":\"temperature\""
#define HASSSENSORCLASS                            " }}\",\"state_class\":\"measurement\",\"expire_after\":\"30\""
#define HASSICON                                   "\",\"~\":\"" UNIQUE_DEVICE_NAME "\",\"icon\":\""
#define HASSOBJECT                                 "\",\"object_id\":\"" UNIQUE_DEVICE_NAME
#define HASSSTATE1                                 "\",\"state_topic\":\"~/tele/STATE1\",\"value_template\":\"{{ "
#define HASSSTATE2                                 "\",\"state_topic\":\"~/tele/STATE2\",\"value_template\":\"{{ "
#define HASSUNIQUEID                               "\",\"unique_id\":\"" UNIQUE_DEVICE_NAME
#define HASSSTAT                                   "\",\"state_topic\":\"~/stat/"
#define HASSNAME                                   "{\"name\":\""
#define HASSCMND                                   "\",\"command_topic\":\"~/cmnd/"
// climate
// see https://www.home-assistant.io/integrations/climate.mqtt/
#define HASSCLIMATEDISCOVERYTOPIC                  "homeassistant/climate/" UNIQUE_DEVICE_NAME "/config"
#define HASSCLIMATEDISCOVERYPAYLOAD                "{\"name\":null,\"unique_id\":\"" UNIQUE_DEVICE_NAME "_temperaturehot" HASSOBJECT "" HASSICON "mdi:fan\",\"min_temp\":10,\"max_temp\":50,\"temp_step\":0.1,\"precision\":0.1,\"temperature_high_command_topic\":\"~/cmnd/TEMPMAX\",\"temperature_high_state_topic\":\"~/stat/TEMPMAX\",\"temperature_high_state_template\":\"{{ round(2) }}\",\"temperature_low_command_topic\":\"~/cmnd/TEMPMIN\",\"temperature_low_state_topic\":\"~/stat/TEMPMIN\",\"temperature_low_state_template\":\"{{ round(2) }}\",\"current_temperature_topic\":\"~/tele/STATE1\",\"value_template\":\"{{ value_json.TempHot }}\",\"modes\":[\"off\",\"fan_only\"],\"mode_command_topic\":\"~/cmnd/MODE\",\"mode_state_topic\":\"~/stat/MODE\",\"availability_topic\":\"~/stat/STATUS\"," HOMEASSISTANTDEVICE "}"

// sensors
// see https://www.home-assistant.io/integrations/sensor.mqtt/
#define HASSSENSORTEMPERATUREHOTDISCOVERYTOPIC     "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/temphot/config"
#define HASSSENSORTEMPERATURECOLDDISCOVERYTOPIC    "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/tempcold/config"
#define HASSSENSORPWMDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/pwm/config"
#define HASSSENSORRPM1DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm1/config"
#define HASSSENSORRPM2DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm2/config"
#define HASSSENSORRPM3DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm3/config"
#define HASSSENSORRPM4DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm4/config"
#define HASSSENSORTEMPERATUREHOTDISCOVERYPAYLOAD   HASSNAME "Coolant" HASSUNIQUEID "_temperaturehot" HASSOBJECT "_temperaturehot" HASSICON "mdi:coolant-temperature" HASSSTATE1 "value_json.TempHot | round(1)" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORTEMPERATURECOLDDISCOVERYPAYLOAD  HASSNAME "Reservoir" HASSUNIQUEID "_temperaturecold" HASSOBJECT "_temperaturecold" HASSICON "mdi:coolant-temperature" HASSSTATE1 "value_json.TempCold | round(1)" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORPWMDISCOVERYPAYLOAD              HASSNAME "PWM" HASSUNIQUEID "_PWM" HASSOBJECT "_PWM" HASSICON "mdi:wind-power" HASSSTATE2 "value_json.pwm" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRPM1DISCOVERYPAYLOAD             HASSNAME "Fan1 RPM" HASSUNIQUEID "_RPM1" HASSOBJECT "_RPM1" HASSICON "mdi:fan" HASSSTATE2 "value_json.rpm0" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRPM2DISCOVERYPAYLOAD             HASSNAME "Fan2 RPM" HASSUNIQUEID "_RPM2" HASSOBJECT "_RPM2" HASSICON "mdi:fan" HASSSTATE2 "value_json.rpm1" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRPM3DISCOVERYPAYLOAD             HASSNAME "Fan3 RPM" HASSUNIQUEID "_RPM3" HASSOBJECT "_RPM3" HASSICON "mdi:fan" HASSSTATE2 "value_json.rpm2" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRPM4DISCOVERYPAYLOAD             HASSNAME "Water Flow" HASSUNIQUEID "_RPM4" HASSOBJECT "_RPM4" HASSICON "mdi:waves-arrow-right" HASSSTATE2 "value_json.rpm3 / 100" HASSSENSORCLASS "," HOMEASSISTANTDEVICE ",\"step\":0.1,\"min\":0.1,\"max\":20,\"unit_of_measurement\":\"L/min\",\"device_class\":\"volume_flow_rate\"}"

// numbers
// see https://www.home-assistant.io/integrations/number.mqtt/
#define HASSNUMBEROFFSETDISCOVERYTOPIC             "homeassistant/number/" UNIQUE_DEVICE_NAME "/tempoffset/config"
#define HASSNUMBERPWMMANUALDISCOVERYTOPIC          "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmmanual/config"
#define HASSNUMBERPWMMINIMUMDISCOVERYTOPIC         "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmminimum/config"
#define HASSNUMBERPWMSTEPDISCOVERYTOPIC         "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmstep/config"
#define HASSNUMBEROFFSETDISCOVERYPAYLOAD           HASSNAME "Offset" HASSUNIQUEID "_tempoffset" HASSOBJECT "_tempoffset" HASSICON "mdi:thermometer-alert" HASSSTAT "TEMPOFFSET" HASSCMND "TEMPOFFSET" "\"," HOMEASSISTANTDEVICE ",\"step\":0.1,\"min\":-10,\"max\":10,\"precision\":0.1,\"mode\":\"box\"," HASSTEMPCLASS "}"
#define HASSNUMBERPWMMANUALDISCOVERYPAYLOAD        HASSNAME "Manual PWM" HASSUNIQUEID "_pwmmanual" HASSOBJECT "_pwmmanual" HASSICON "mdi:wind-power" HASSSTAT "PWMMANUAL" HASSCMND "PWMMANUAL" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":255,\"mode\":\"slider\"}"
#define HASSNUMBERPWMMINIMUMDISCOVERYPAYLOAD       HASSNAME "Minimum PWM" HASSUNIQUEID "_pwmminimum" HASSOBJECT "_pwmminimum" HASSICON "mdi:wind-power" HASSSTAT "PWMMINIMUM" HASSCMND "PWMMINIMUM" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":255,\"mode\":\"box\"}"
#define HASSNUMBERPWMSTEPDISCOVERYPAYLOAD       HASSNAME "PWM Step" HASSUNIQUEID "_pwmstep" HASSOBJECT "_pwmstep" HASSICON "mdi:wind-power" HASSSTAT "PWMSTEP" HASSCMND "PWMSTEP" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":1,\"max\":255,\"mode\":\"box\"}"

// switch
// see https://www.home-assistant.io/integrations/switch.mqtt/
#define HASSSWITCHMANUALDISCOVERYTOPIC             "homeassistant/switch/" UNIQUE_DEVICE_NAME "/manual/config"
#define HASSSWITCHMANUALDISCOVERYPAYLOAD           HASSNAME "Manual mode" HASSUNIQUEID "_manual" HASSOBJECT "_manual" HASSICON "mdi:toggle-switch" HASSSTAT "MANUAL" HASSCMND "MANUAL" "\"," HOMEASSISTANTDEVICE ",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"}"

#endif

// --- include override settings from seperate file ---------------------------------------------------------------------------------------------------------------
#if __has_include("config_override.h")
  #include "config_override.h"
#endif

#endif /*__CONFIG_H__*/