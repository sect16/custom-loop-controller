#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"

int pwmValue = 0;
int pwmTarget = 0;
bool modeIsOff = false;
unsigned long lastChange = 0;
void setPWMvalue(int pwmTarget, bool force);
void setFanMode(bool aModeIsOff, bool force);
int pwmStep;

// https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
void initPWMfan(void){
  // configure LED PWM functionalitites
  ledcSetup(PWMCHANNEL, PWMFREQ, PWMRESOLUTION);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWMPIN, PWMCHANNEL);
  pwmValue = PWMINITIAL;
  setPWMvalue(pwmValue, true);
  setFanMode(false, true);
  Log.printf("  Fan PWM sucessfully initialized.\r\n");
}

void setFanSpeed(void){
  ledcWrite(PWMCHANNEL, pwmValue);
}

void setPWMvalue(int pwm, bool force) {
  pwmTarget = pwm;
  unsigned long currentMillis = millis();
  if (pwmTarget > 255) {pwmTarget = 255;};
  if (modeIsOff) {
    pwmValue = 0;
    }
  else if (force) {
    pwmValue = pwmTarget;
  } 
  else if ((currentMillis - lastChange) > 1000) {
    lastChange = currentMillis;
    if (pwmTarget > pwmValue) {
      if (pwmValue < PWMMINIMUM) pwmValue = PWMMINIMUM;
      else pwmValue = pwmValue + pwmStep;
    } else if (pwmTarget < pwmValue) {
      if (pwmValue < PWMMINIMUM) pwmValue = 0; 
      else pwmValue = pwmValue - pwmStep;
  }
  } 
  setFanSpeed();
  #ifdef useMQTT
  mqtt_publish_tele2();
  #endif
}

void setFanMode(bool aModeIsOff, bool force) {
  if ((modeIsOff != aModeIsOff) || force) {
    modeIsOff = aModeIsOff;
    #ifdef useMQTT
    mqtt_publish_stat_mode();
    #endif
    switchOff_screen(modeIsOff);
  }
  if (modeIsOff) {
    setPWMvalue(0, true);
  } else {
    setPWMvalue(PWMINITIAL, true);
  }
}