extern int pwmValue;
extern bool modeIsOff;

// Function to initialize the PWM fan
void initPWMfan();

// Function to set the PWM value
// Input: an integer representing the PWM value, a boolean to force the update
void setPWMvalue(int pwmValue, bool force = false);

// Function to set the fan mode
// Input: a boolean indicating whether the mode is off, a boolean to force the update
void setFanMode(bool isModeOff, bool force = false);