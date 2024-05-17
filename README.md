![Build](https://github.com/KlausMu/esp32-fan-controller/actions/workflows/build-platformio.yml/badge.svg)

# ESP32 fan controller with MQTT support
I could not find a product in the market for my gaming desktop custom loop water cooling system. I tried to do the same with several products such as Corsair Commander however it was not reliable. Unfortunately my mainboard did not support external temperature sensors which made it even more difficult to manage temperatures.

So i had to customize this project describes to operate:
* 3 x pwm controlled fan,  
* 1 x water flow meter and
* 2 x NTC 10K temperature sensors.

Main features are:
* Change between manual and temperature based PWM control using Home Assistant
* RPM, flow, temperature statictics are fed into Home Assitant
* Linear fan curve based on lower and upper temperature limits set using Home Assistant
* Ability to set an offset value for temperature calibration
* measurement of 3 fan speed via tacho signal
* measurement of water temperature using NTC 10K thermistors
* support of MQTT
* support of OTA (over the air updates of firmware). Please see <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/07-OTA---Over-the-air-updates">Wiki: 07 OTA Over the air updates</a>

* TFT display for showing status information (Only 320x240 supported) using I2C
* Removed touch display for setting pwm or target temperature

<b>For more information please see the <a href="https://github.com/sect16/custom-loop-controller/wiki">Wiki</a></b>

## Integration in Home Assistant
With mqtt discovery, you can integrate the fan controller with almost no effort in Home Assistant.
<a href="https://github.com/sect16/custom-loop-controller/assets/89375638/0654a639-1b35-4dd5-8576-122511d86f72"><img src="https://github.com/sect16/custom-loop-controller/assets/89375638/0654a639-1b35-4dd5-8576-122511d86f72"></a> <a href="https://github.com/sect16/custom-loop-controller/assets/89375638/e1037dc7-9e3f-4240-8aa1-35e372fba79c"><img src="https://github.com/sect16/custom-loop-controller/assets/89375638/e1037dc7-9e3f-4240-8aa1-35e372fba79c"></a>

Please see <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/05-Home-Assistant">Wiki: 05 Home Assistant</a>

## Operation modes
You can operate the ESP32 fan controller mainly in two different modes, depending on your needs:
mode | description | how to set PWM | how to set upper/lower temperature
------------ | ------------- | ------------- | ------------- |
Manual Mode On | fan speed directly set via PWM signal | MQTT |  N/A |
Manual Mode Off | Automatic temperature control<br>fan speed is automatically set depending on difference between upper/lower temperature | N/A | MQTT |

TFT panel shows status information for each the fan, water temperatures and water flow meter. Tested only with 320x240).

<a href="https://github.com/sect16/custom-loop-controller/assets/89375638/2751ee6c-7eb1-4dcc-8c1b-ae502f95f026"><img src="https://github.com/sect16/custom-loop-controller/assets/89375638/2751ee6c-7eb1-4dcc-8c1b-ae502f95f026"></a>

## Wiring diagram
![Wiring diagram](https://github.com/sect16/custom-loop-controller/assets/89375638/438657b7-2fbb-4622-b48d-e912e815d183)

## Part list
Function | Parts | Remarks | approx. price
------------ | ------------- | ------------- | -------------
<b>mandatory</b>
microcontroller | ESP32 | e.g. from  <a href="https://www.az-delivery.de/en/products/esp32-developmentboard">AZ-Delivery</a> | 8 EUR
fan | 4 pin fan (4 pin means pwm controlled), 5V or 12V | tested with a standard CPU fan and a Noctua NF-F12 PWM<br>for a list of premium fans see https://noctua.at/en/products/fan | 20 EUR for Noctua
measuring tacho signal of fan | resistor 3.3 k&#8486;;
power supply | - 3.3V for ESP32, 5V or 12V for fan (depending on fan)
temperature sensor | - NTC 10K<br>- 2 pullup resistors 10 k&#8486;; |
TFT display | 2.8 inch 320x240, ILI9341 | 

## Software installation
I am using <a href="https://platformio.org/">PlatformIO IDE</a>.

For installing PlatformIO IDE, follow this <a href="https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation">guide</a>. It is as simple as:
* install VSCode (Visual Studio Code)
* install PlatformIO as an VSCode extension
* clone this repository or download it
* use "open folder" in VSCode to open this repository
* check settings in "config.h"
* upload to ESP32
