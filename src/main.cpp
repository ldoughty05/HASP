/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   4/10/2025
File:   main.cpp
UNL AerospaceExperimental Payloads Team

Purp:   Main file for the Teensy4.1 which is responsible for taking a
        temperature reading from 8 MCP9700A temperature sensors,
        recording a time stamp, collecting data from the Rx5 port,
        and then saving all of that data to the onboard SD card.
--------------------------------------------------------------------*/
#include <Arduino.h>
#include <SD.h>
#include "thermometer.h"
#include "utils.h"

#define NUM_TEMP_SENSORS 8 // Number of temperature sensors
#define UART_BUFFER_SIZE 64 // Size of the UART buffer
#define DATA_FILE_NAME_BASE "HASP_data" // gets a random number and .txt appended.
#define RANDOM_SEED_PIN A0 // Pin used for random seed generation. Value is very volatile since it isn't connected to anything.

double temperaturesCelsius[NUM_TEMP_SENSORS];
char UARTBuffer[UART_BUFFER_SIZE]; 
/* Number of characters before we assume something went wrong and we
   missed a newline, so just save what we have. 
*/
File dataFile;

void collectDataFromUART(char *buffer){
  int bufferIndex = 0;
  char receivedByte = '\0'; // Initialize the received byte variable

  /* Wait line of data from UART */
  while (receivedByte != '\n' && bufferIndex < UART_BUFFER_SIZE - 1) { // busy wait until newline or buffer is full.
    if (Serial1.available()){
      receivedByte = Serial1.read(); // Read a byte from the UART
      buffer[bufferIndex] = receivedByte;
      bufferIndex++;   
    }
  }

  /* When we have a full line of data */
  buffer[bufferIndex] = '\0'; // Null-terminate the string
  Serial.print("Buffer Length: ");
  Serial.println(measureStringLength(buffer)); // Print the length of the buffer
}

void printToSD(File sdfile, unsigned long timestamp, double* temperatures, int numTemperatureReadings, char* UARTBuffer){
  // UARTBuffer is null terminated, so we don't need its length as a parameter.
  if (sdfile) {
    sdfile.print("(Timestamp: ");
    sdfile.print(timestamp);
    sdfile.print(") Temperature Readings: [");
    for (int i = 0; i < numTemperatureReadings; i++) {
      sdfile.print(temperatures[i]);
      if (i < numTemperatureReadings - 1) sdfile.print(", "); // The "if" just gets rid of the trailing comma.
    }
    sdfile.println("], UART Data: [");
    sdfile.print(UARTBuffer);
    sdfile.println("]");
    sdfile.flush(); // Ensure all data is written to the SD card
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

void setup() {
  Serial.begin(9600); // From Serial Monitor
  Serial1.begin(9600); // From UART1
  pinMode(LED_BUILTIN, OUTPUT);
  /* No need to set analog pins to output. */

  while (!Serial && !Serial1) {
    // Cant connect to UART or the serial monitor.
    blink(100);
  };

  while (!SD.begin(BUILTIN_SDCARD)){
    // Can't connect to the SD card.
    blink(250);
  };
  char fileName[24];
  randomSeed(analogRead(RANDOM_SEED_PIN));
  sprintf(fileName, "%s%d.txt", DATA_FILE_NAME_BASE, random(1000, 9999));
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Created file at epoch: ");
    dataFile.println(millis());
    dataFile.flush();
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED to indicate SD card is ready.
  }
}

// print timestamp, all temperatures, all from Rx



void loop() {
  unsigned long timestamp = millis(); // Time since program started.
  updateTemperaturesArray(temperaturesCelsius, NUM_TEMP_SENSORS);
  collectDataFromUART(UARTBuffer); // Buffers bytes from UART until a newline or buffer fills up. // Should hold here for some time.
  printToSD(dataFile, timestamp, temperaturesCelsius, NUM_TEMP_SENSORS, UARTBuffer);
  blink(500);

  /* PROBLEM: if for some reason we stop getting data from UART, it will be stuck on that line and we won't get temperature readings either. Add a timeout */
}