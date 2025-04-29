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

#define MODULE1_CURRENT 2
#define MODULE1_VOLTAGE 3
#define MODULE2_CURRENT 5
#define MODULE2_VOLTAGE 4

#define UART_BUFFER_SIZE 64 // Size of the UART buffer
#define DATA_FILE_NAME_BASE "HASP_data" // gets a random number and .txt appended.
#define RANDOM_SEED_PIN A0 // Pin used for random seed generation. Value is very volatile since it isn't connected to anything.

typedef struct {
  unsigned long timestamp; // Time since program started.
  double temperatures[NUM_TEMP_SENSORS]; // Array of temperature readings in Celcius.
  double module1Voltage; // Voltage from solar cell module 1.
  double module1Current; // Current from solar cell module 1 in mA.
  double module2Voltage; // Voltage from solar cell module 2. 
  double module2Current; // Current from solar cell module 2 in mA.
} PayloadPacket;

PayloadPacket payloadReadings;
char UARTBuffer[UART_BUFFER_SIZE]; 
File dataFile;

/*
  @brief: listens to the UART1 bus and stores the incoming bytes into the buffer until a newline is received or the buffer is full.
  @param UARTBus: The Serial object to read from. Corresponds to the UART bus.
  @param buffer: The buffer to store the incoming bytes. Buffer size is defined by the `#define UART_BUFFER_SIZE` in main.cpp.
*/
void collectDataFromUART(Stream& serialObject, char *buffer){
  int bufferIndex = 0;
  char receivedByte = '\0'; // Initialize the received byte variable

  /* Wait line of data from UART */
  while (receivedByte != '\n' && bufferIndex < UART_BUFFER_SIZE - 1) { // busy wait until newline or buffer is full.
    if (serialObject.available()){
      receivedByte = serialObject.read(); // Read a byte from the UART
      buffer[bufferIndex] = receivedByte;
      bufferIndex++;   
    }
  }

  /* When we have a full line of data */
  buffer[bufferIndex] = '\0'; // Null-terminate the string
  Serial.print("Buffer Length: ");
  Serial.println(measureStringLength(buffer)); // Print the length of the buffer
}

/*
  @brief: 
  @param voltageReading: The pointer to the variable to store the voltage reading in Volts.
  @param currentReading: The pointer to the variable to store the current reading in mA.
*/
void readFromSolarCell(uint8_t voltagePin, uint8_t currentPin, double* voltageReading, double* currentReading){
  int voltageReadingInt = analogRead(voltagePin); // Read the voltage from the pin
  int currentReadingInt = analogRead(currentPin); // Read the current from the pin

  *voltageReading = (float)(3.3 / 1023.0) * 2.0 * voltageReadingInt; // Convert to voltage
  *currentReading = (float)(3.3 / (1023.0 * 9.88)) * currentReadingInt * 1000; // Convert to current
}

/*
  @brief: Writes a line to the SD card with the timestamp, temperature readings, and UART data.
  @param sdfile: The file object for the SD card.
  @param payloadReadings: The payload packet containing the temperature readings, both solar cell voltages & currents, and timestamp.
  @param UARTBuffer: The string containing the UART data.
  @return: 1 if the write was successful, 0 if it failed.
*/
uint8_t printToSD(File sdfile, PayloadPacket payloadReadings, char* UARTBuffer){
  uint8_t failed = 0; // Flag to indicate if the SD card write failed.
  // UARTBuffer is null terminated, so we don't need its length as a parameter.
  if (sdfile) {
    sdfile.print("(Timestamp: ");
    sdfile.print(payloadReadings.timestamp);

    sdfile.print(") Temperature Readings: [");
    for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
      sdfile.print(payloadReadings.temperatures[i]);
      if (i < NUM_TEMP_SENSORS - 1) sdfile.print(", "); // The "if" just gets rid of the trailing comma.
    }
    sdfile.print("], Solar Cell 1: [Voltage: ");
    sdfile.print(payloadReadings.module1Voltage);
    sdfile.print("V, Current: ");
    sdfile.print(payloadReadings.module1Current);
    sdfile.print("mA], Solar Cell 2: [Voltage: ");
    sdfile.print(payloadReadings.module2Voltage);
    sdfile.print("V, Current: ");
    sdfile.print(payloadReadings.module2Current);

    sdfile.print("mA], UART Data: [");
    sdfile.print(UARTBuffer);
    sdfile.println("]");
    sdfile.flush(); // Ensure all data is written to the SD card
  } else {
    failed = 1; // Failed to write to SD card.
  }
  return !failed;
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
  sprintf(fileName, "%s%ld.txt", DATA_FILE_NAME_BASE, random(1000, 9999));
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
  payloadReadings.timestamp = millis(); // Time since program started.
  updateTemperaturesArray(payloadReadings.temperatures, NUM_TEMP_SENSORS);
  readFromSolarCell(MODULE1_VOLTAGE, MODULE1_CURRENT, &payloadReadings.module1Voltage, &payloadReadings.module1Current);
  readFromSolarCell(MODULE2_VOLTAGE, MODULE2_CURRENT, &payloadReadings.module2Voltage, &payloadReadings.module2Current);
  // collectDataFromUART(Serial1, UARTBuffer); // Buffers bytes from UART until a newline or buffer fills up. // Should hold here for some time.
  if(printToSD(dataFile, payloadReadings, UARTBuffer)){
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED to indicate SD card is ready.
    delay(500);
  } else {
    blink(250);
  };
  /* PROBLEM: if for some reason we stop getting data from UART, it will be stuck on that line and we won't get temperature readings either. Add a timeout */
}