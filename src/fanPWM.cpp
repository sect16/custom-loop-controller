#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"

int pwmValue = 0;
bool modeIsOff = false;
void setPWMvalue(int aPWMvalue, bool force);
void setFanMode(bool aModeIsOff, bool force);

// https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
void initPWMfan(void){
  // configure LED PWM functionalitites
  ledcSetup(PWMCHANNEL, PWMFREQ, PWMRESOLUTION);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWMPIN, PWMCHANNEL);
  pwmValue = INITIALPWMVALUE;
  setPWMvalue(pwmValue, true);
  setFanMode(false, true);
  Log.printf("  Fan PWM sucessfully initialized.\r\n");
}

void setFanSpeed(void){
  ledcWrite(PWMCHANNEL, pwmValue);
}

void setPWMvalue(int aPWMvalue, bool force) {
  // note: it is not guaranteed that fan stops if pwm is set to 0
  if (modeIsOff) {aPWMvalue = 0;}
  if ((pwmValue != aPWMvalue) || force) {
    pwmValue = aPWMvalue;
    if (pwmValue < 0) {pwmValue = 0;};
    if (pwmValue > 255) {pwmValue = 255;};
    setFanSpeed();
    #ifdef useMQTT
    mqtt_publish_tele2();
    #endif
  }
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
    setPWMvalue(INITIALPWMVALUE, true);
  }
}