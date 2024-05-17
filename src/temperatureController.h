extern float temperatureMax;
extern float temperatureMin;
extern float temperatureOffset;
extern bool manual;
extern int pwmManual;
extern int pwmMinimum;
extern int pwmStep;

// Temperature Controller Functions

// Initialization
void initTemperatureController(void);

// Fan PWM based on temperature
void setFanPWMbasedOnTemperature(void);

// Fan parameters
void setManual(bool state, bool force);
void setPwmManual(int pwm, bool force);
void setPwmMinimum(int pwm, bool force);
void setPwmStep(int pwm, bool force);

// Temperature getters
const char *getManual(void);

// Temperature setters
void setTemperatureMax(float aTemperatureMax, bool force);
void setTemperatureMin(float aTemperatureMin, bool force);
void setTemperatureOffset(float aTemperatureOffset, bool force);