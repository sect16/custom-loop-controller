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

float temperatureMax;
float temperatureMin;
float temperatureOffset;
bool manual;
int pwmManual;
int pwmMinimum;

void initTemperatureController(void) {
    preferences.begin("hiveMon", false);
    temperatureMax = preferences.getFloat("tempMax", INITIALTEMPERATUREMAX);
    temperatureMin = preferences.getFloat("tempMin", INITIALTEMPERATUREMIN);
    temperatureOffset = preferences.getFloat("tempOffset", INITIALTEMPERATUREOFFSET);
    pwmMinimum = preferences.getFloat("pwmMin", PWMMINIMUM);
    pwmStep = preferences.getInt("pwmStep", PWMSTEP);
    preferences.end();
    // Set by default MIN PWM
    pwmManual = PWMMINIMUM;
    manual = false;
    Log.printf("Reading preferences and setting values:\r\nmaxTemperature = %f, minTemperature = %f, offsetTemperature = %f\r\n", temperatureMax, temperatureMin, temperatureOffset);
}

const char* getManual(void) {
    if (manual) return "ON";
    else return "OFF";
}

void setFanPWMbasedOnTemperature(void) {
    int pwmNew = 255;
    // Log.printf("Temp = %f Max = %f Min = %f Offet = %f \r\n", temperature[0], maxTemperature, minTemperature, offsetTemperature);
    if (manual) {
        Log.printf("manualPWMvalue = %d\r\n", pwmManual);
        setPWMvalue(pwmManual, true);
    } else if (temperature[0] == NAN || temperature[0] <= 0.0) {
        Log.printf("WARNING: no temperature value available. Cannot do temperature control. Will set PWM fan to 0.\r\n");
        setPWMvalue(PWMINITIAL, true);
    } else if (temperature[0] <= temperatureMin) {
        setPWMvalue(PWMINITIAL, true);
    } else if (temperature[0] <= temperatureMin + (temperatureMax-temperatureMin)/2) {
        pwmNew = (int)fmin(255, (((temperature[0] - temperatureMin) * 100) * ((255 - pwmMinimum) / ((temperatureMax - temperatureMin) * 100))) + pwmMinimum);
        setPWMvalue(pwmNew, false);
    } else {
        pwmNew = (int)fmin(255, (((temperature[0] - temperatureMin) * 100) * ((255 - pwmMinimum) / ((temperatureMax - temperatureMin) * 100))) + pwmMinimum);
        setPWMvalue(pwmNew, true);
    }
    // Log.printf("newPWMvalue = %d\r\n", newPWMvalue);
    
}

void setTemperatureMax(float aTemperatureMax, bool force) {
    if ((temperatureMax != aTemperatureMax) || force) {
        temperatureMax = aTemperatureMax;
        preferences.begin("hiveMon", false);
        // Remove all preferences under the opened namespace
        //preferences.clear();

        // Or remove the counter key only
        //preferences.remove("counter");

        // Get the counter value, if the key does not exist, return a default value of 0
        // Note: Key name is limited to 15 chars.
        
        preferences.putFloat("tempMax", aTemperatureMax);  // max 15 letters NameSpace
        preferences.end();
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_tempMin();
        mqtt_publish_stat_tempMax();
        #endif
    }
}

void setTemperatureMin(float aTemperatureMin, bool force) {
    if ((temperatureMin != aTemperatureMin) || force) {
        temperatureMin = aTemperatureMin;
        preferences.begin("hiveMon", false);    
        preferences.putFloat("tempMin", aTemperatureMin);  // max 15 letters NameSpace
        preferences.end();
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_tempMin();
        mqtt_publish_stat_tempMax();
        #endif
    }
}

void setTemperatureOffset(float aTemperatureOffset, bool force) {
    if ((temperatureOffset != aTemperatureOffset) || force) {
        temperatureOffset = aTemperatureOffset;
        preferences.begin("hiveMon", false);
        preferences.putFloat("tempOffset", aTemperatureOffset);  // max 15 letters NameSpace
        preferences.end();;
        setFanPWMbasedOnTemperature();
        #ifdef useMQTT
        mqtt_publish_stat_tempOffset();
        #endif
    }
}

void setManual(bool state, bool force) {
    if ((manual != state) || force) {
        manual = state;
        mqtt_publish_stat_manual();
    }
}

void setPwmManual(int pwm, bool force) {
    if ((pwmManual != pwm) || force) {
        pwmManual = (int)fmin(255, pwm);
        mqtt_publish_stat_pwmManual();
    }
}

void setPwmMinimum(int pwm, bool force) {
    if ((pwmMinimum != pwm) || force) {
        pwm = (int)fmin(255, pwm);
        preferences.begin("hiveMon", false);    
        preferences.putFloat("pwmMin", pwm);  // max 15 letters NameSpace
        preferences.end();
        pwmMinimum = pwm;
        mqtt_publish_stat_pwmMinimum();
    }
}

void setPwmStep(int pwm, bool force) {
    if ((pwmStep != pwm) || force) {
        pwm = (int)fmin(255, pwm);
        preferences.begin("hiveMon", false);    
        preferences.putFloat("pwmStep", pwm);  // max 15 letters NameSpace
        preferences.end();
        pwmStep = pwm;
        mqtt_publish_stat_pwmStep();
    }
}