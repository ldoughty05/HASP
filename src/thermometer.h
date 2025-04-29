/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   4/10/2025
File:   thermometer.h
UNL AerospaceExperimental Payloads Team

Purp:   Takes analog readings from MCP9700A temperature sensors and
        converts them to Celsius.
--------------------------------------------------------------------*/
#define TEMP1 A10 // Teensy pin 24
#define TEMP2 A11 // Teensy pin 25
#define TEMP3 A12 // Teensy pin 26
#define TEMP4 A13 // Teensy pin 27
#define TEMP5 A9  // Teensy pin 21
#define TEMP6 A8  // Teensy pin 20
#define TEMP7 A7  // Teensy pin 19
#define TEMP8 A6  // Teensy pin 18
#define NUM_TEMP_SENSORS 8 // Number of temperature sensors


/*
@brief: Sets the values of each item in the temperaturesCelcius array to the reading from the corresponding
             temperature sensor. The temperature sensors are connected to the analog pins defined in thermometer.h.
@param temperaturesCelsius: The array to store the temperature readings in.
@param size: (optional) The number of temperature sensors to read from. Defaults to NUM_TEMP_SENSORS.
*/
void updateTemperaturesArray(double *temperaturesCelsius, int size);
