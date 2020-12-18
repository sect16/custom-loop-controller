#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "config.h"
#include "log.h"
#include "temperatureController.h"

#ifdef useTemperatureSensorBME280
// Standard pressure at sea level. Will be calibrated in initBME280
float calibratedPressureAtSeaLevel = 1013.25;

float lastTempSensorValues[4];

Adafruit_BME280 bme;

TwoWire I2Cone = TwoWire(0);
bool status_BME280 = 0;
#endif

void initBME280(void){
  #ifdef useTemperatureSensorBME280
  lastTempSensorValues[0] = NAN;
  lastTempSensorValues[1] = NAN;
  lastTempSensorValues[2] = NAN;
  lastTempSensorValues[3] = NAN;

  I2Cone.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  status_BME280 = bme.begin(BME280_addr, &I2Cone);

  if (!status_BME280) {
    log_printf(MY_LOG_FORMAT("  Could not find a valid BME280 sensor, check wiring!"));
  } else {
    log_printf(MY_LOG_FORMAT("  BME280 sucessfully initialized."));
    // Calibrate BME280 with actual pressure and given height. Will be used until restart of ESP32
    calibratedPressureAtSeaLevel = (bme.seaLevelForAltitude(heightOverSealevelAtYourLocation, bme.readPressure() / 100.0F));
    log_printf(MY_LOG_FORMAT("  BME280 was calibrated to %.1f m"), heightOverSealevelAtYourLocation);
  }
  #else
  log_printf(MY_LOG_FORMAT("    BME280 is disabled in config.h"));
  #endif
}

void updateBME280(void){
  #ifdef useTemperatureSensorBME280
  if (!status_BME280){
    log_printf(MY_LOG_FORMAT("BME280 sensor not initialized, trying again ..."));
    initBME280();
    if (status_BME280){
      log_printf(MY_LOG_FORMAT("success!"));
    } else {
      lastTempSensorValues[0] = NAN;
      #ifndef setActualTemperatureViaMQTT
      setActualTemperatureAndPublishMQTT(lastTempSensorValues[0]);
      #endif
      lastTempSensorValues[1] = NAN;
      lastTempSensorValues[2] = NAN;
      lastTempSensorValues[3] = NAN;
      return;
    }
  }
  lastTempSensorValues[0] = bme.readTemperature();
  #ifndef setActualTemperatureViaMQTT
  setActualTemperatureAndPublishMQTT(lastTempSensorValues[0]);
  #endif
  lastTempSensorValues[1] = bme.readPressure() / 100.0F;
  lastTempSensorValues[2] = bme.readAltitude(calibratedPressureAtSeaLevel);
  lastTempSensorValues[3] = bme.readHumidity();
  #endif
}