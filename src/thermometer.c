/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   4/10/2025
File:   thermometer.c
UNL AerospaceExperimental Payloads Team

Purp:   Takes analog readings from MCP9700A temperature sensors and
        converts them to Celsius.
--------------------------------------------------------------------*/
#include <Arduino.h>
#include "thermometer.h"

/*
  NOT IMPLEMENTED. No need.
*/
void setupTemperatureSensors() {};
int temperatureSensorRegisters[NUM_TEMP_SENSORS] = {TEMP1, TEMP2, TEMP3, TEMP4, TEMP5, TEMP6, TEMP7, TEMP8};

void updateTemperaturesArray(double *temperaturesCelsius, int size) {
  for (int i = 0; i < size; i++) { // For each temperature sensor:
    int temperatureReading = analogRead(temperatureSensorRegisters[i]); // range [0, 1023]
    float temperatureVoltage = (temperatureReading * 3.3) / 1023.0; // [0, 3.3V]
    temperaturesCelsius[i] = (temperatureVoltage - 0.5) * 100.0; // (Vout - <V at 0°C>) * <1 / Temp Coeff> = Ambient Temp
  }
}
/*
  Overload where size is not included. Defaults 'size' to NUM_TEMP_SENSORS.
*/
void updateTemperaturesArray(double *temperaturesCelsius) {
  updateTemperaturesArray(temperaturesCelsius, NUM_TEMP_SENSORS);
}


/* Analog Pins 
  ADC library exists. Processes up to 10 bits resolution.
  Voltage input range [0, 3.3V] mapping to readings from [0, 1023]. 
  Keep current as low as possible.
*/

/* MCP9700A Ratings
  - Operating Voltage: 2.3V to 5.5V
  - Output Voltage Range: 0.1V to 1.75V
  - Temperature Range: -40°C to +125°C

  - Temperature Coefficient: 0.01V/°C
  - Output Voltage at 0°C: 0.5V
*/