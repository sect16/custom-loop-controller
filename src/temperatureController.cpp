#include <Arduino.h>
#include <esp32-hal-log.h>
#include <math.h>

#include "config.h"
#include "fanPWM.h"
#include "log.h"
#include "mqtt.h"
#include "sensorNTC.h"
#include "tft.h"
#include <Preferences.h>

Preferences preferences;

float maxTemperature;
float minTemperature;
float offsetTemperature;
bool manual;
int manualPWMvalue;
int pwmMinimumValue;

void initTemperatureController(void) {
    preferences.begin("hiveMon", false);
    maxTemperature = preferences.getFloat("maxTemp", INITIALMAXTEMPERATURE);
    minTemperature = preferences.getFloat("minTemp", INITIALMINTEMPERATURE);
    offsetTemperature = preferences.getFloat("offset", INITIALOFFSETTEMPERATURE);
    pwmMinimumValue = preferences.getFloat("minPWM", PWMMINIMUMVALUE);
    preferences.end();
    manualPWMvalue = PWMMINIMUMVALUE;
    manual = false;
    Log.printf("Setting values:\r\nmaxTemperature = %f, minTemperature = %f, offsetTemperature = %f\r\n", maxTemperature, minTemperature, offsetTemperature);
}

const char* getManual(void) {
    if (manual) return "ON";
    else return "OFF";
}

void setFanPWMbasedOnTemperature(void) {
    int newPWMvalue = 255;
    // Log.printf("Temp = %f Max = %f Min = %f Offet = %f \r\n", getActualTemperature(), getMaxTemperature(), getMinTemperature(), getOffsetTemperature());
    if (manual) {
        Log.printf("manualPWMvalue = %d\r\n", manualPWMvalue);
        newPWMvalue = manualPWMvalue;
    } else if (temperature[0] == NAN || temperature[0] <= 0.0) {
        Log.printf("WARNING: no temperature value available. Cannot do temperature control. Will set PWM fan to 0.\r\n");
        newPWMvalue = 0;
    } else if (temperature[0] <= minTemperature) {
        // Temperature is below minimum temperature. Run fan at minimum speed.
        newPWMvalue = 0;
    } else {
        float pwm = fmin(255, (((temperature[0] - minTemperature) * 100) * ((255 - pwmMinimumValue) / ((maxTemperature - minTemperature) * 100))) + pwmMinimumValue);
        newPWMvalue = (int)pwm;
    }
    // Log.printf("newPWMvalue = %d\r\n", newPWMvalue);
    setPWMvalue(newPWMvalue, false);
}

void setMaxTemperature(float aMaxTemperature, bool force) {
    if ((maxTemperature != aMaxTemperature) || force) {
        maxTemperature = aMaxTemperature;
        preferences.begin("hiveMon", false);
        // Remove all preferences under the opened namespace
        //preferences.clear();

        // Or remove the counter key only
        //preferences.remove("counter");

        // Get the counter value, if the key does not exist, return a default value of 0
        // Note: Key name is limited to 15 chars.
        
        preferences.putFloat("maxTemp", aMaxTemperature);  // max 15 letters NameSpace
        preferences.end();
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_minTemp();
        mqtt_publish_stat_maxTemp();
        #endif
    }
}

void setMinTemperature(float aMinTemperature, bool force) {
    if ((minTemperature != aMinTemperature) || force) {
        minTemperature = aMinTemperature;
        preferences.begin("hiveMon", false);    
        preferences.putFloat("minTemp", aMinTemperature);  // max 15 letters NameSpace
        preferences.end();
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_minTemp();
        mqtt_publish_stat_maxTemp();
        #endif
    }
}

void setOffsetTemperature(float aOffsetTemperature, bool force) {
    if ((offsetTemperature != aOffsetTemperature) || force) {
        offsetTemperature = aOffsetTemperature;
        preferences.begin("hiveMon", false);
        preferences.putFloat("offset", aOffsetTemperature);  // max 15 letters NameSpace
        preferences.end();;
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_offsetTemp();
        #endif
    }
}

void setManual(bool state, bool force) {
    if ((manual != state) || force) {
        manual = state;
        mqtt_publish_stat_manual();
    }
}

void setManualPWM(int pwm, bool force) {
    if ((manualPWMvalue != pwm) || force) {
        manualPWMvalue = (int)fmin(255, pwm);
        mqtt_publish_stat_manualPWM();
    }
}

void setMinimumPWM(int pwm, bool force) {
    if ((pwmMinimumValue != pwm) || force) {
        pwm = (int)fmin(255, pwm);
        preferences.begin("hiveMon", false);    
        preferences.putFloat("minPWM", pwm);  // max 15 letters NameSpace
        preferences.end();
        pwmMinimumValue = pwm;
        mqtt_publish_stat_minimumPWM();
    }
}