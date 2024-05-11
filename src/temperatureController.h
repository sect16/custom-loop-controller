extern float maxTemperature;
extern float minTemperature;
extern float offsetTemperature;
extern bool manual;
extern int manualPWMvalue;
extern int pwmMinimumValue;

// Temperature Controller Functions

// Initialization
void initTemperatureController(void);

// Fan PWM based on temperature
void setFanPWMbasedOnTemperature(void);

// Fan parameters
void setManual(bool state, bool force);
void setManualPWM(int pwm, bool force);
void setMinimumPWM(int pwm, bool force);

// Temperature getters
const char *getManual(void);

// Temperature setters
void setMaxTemperature(float aMaxTemperature, bool force);
void setMinTemperature(float aMinTemperature, bool force);
void setOffsetTemperature(float aOffsetTemperature, bool force);