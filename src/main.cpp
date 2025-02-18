/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   2/17/2025
File:   main.cpp

Purp: Taking readings from the ICM-20948 IMU sensor and storing them 
to microSD.

Derived from SparkFun 9DoF IMU Breakout - ICM 20948 Example999_Portable.ino
by Owen Lyke @ SparkFun Electronics, 2019
--------------------------------------------------------------------*/

#include <Arduino.h>
#include "ICM_20948.h"
#include "Wire.h"
#include "imu.h"

#define SERIAL_PORT Serial
#define WIRE_PORT Wire
#define I2C_ADDRESS ICM_20948_I2C_ADDR_AD1

///////////////////////////////////////////////////////////////
/* Interface Functions (Dont ask me what they do) */
///////////////////////////////////////////////////////////////
ICM_20948_Status_e my_write_i2c(uint8_t reg, uint8_t *data, uint32_t len, void *user)
{
  WIRE_PORT.beginTransmission(I2C_ADDRESS);
  WIRE_PORT.write(reg);
  WIRE_PORT.write(data, len);
  WIRE_PORT.endTransmission();

  return ICM_20948_Stat_Ok;
}

ICM_20948_Status_e my_read_i2c(uint8_t reg, uint8_t *buff, uint32_t len, void *user)
{
  WIRE_PORT.beginTransmission(I2C_ADDRESS);
  WIRE_PORT.write(reg);
  WIRE_PORT.endTransmission(false); // Send repeated start

  uint32_t num_received = WIRE_PORT.requestFrom(I2C_ADDRESS, len);
  if (num_received == len)
  {
    for (uint32_t i = 0; i < len; i++)
    {
      buff[i] = WIRE_PORT.read();
    }
  }

  return ICM_20948_Stat_Ok;
}

const ICM_20948_Serif_t mySerif = { // "Serial Interface" (serif)
	my_write_i2c, // write
	my_read_i2c,  // read
	NULL,
};

//////////////
/* MAIN CODE*/
//////////////
ICM_20948_Device_t myICM;

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	WIRE_PORT.begin();
	WIRE_PORT.setClock(400000);

	// Initialize myICM
	ICM_20948_init_struct(&myICM);

	// Link the serif ("Serial Interface")
	ICM_20948_link_serif(&myICM, &mySerif);

	// Ensure the IMU is connected.
	findIMU(&myICM);

	// Here we are doing a SW reset to make sure the device starts in a known state
  ICM_20948_sw_reset(&myICM);
  delay(250);

	// Set sampling modes and full scale ranges.
	configureSensors(&myICM);
	
	// Set up filter to get rid of noisey data.
	setupLowPassFilter(&myICM);

	// Wake the sensor up
	ICM_20948_sleep(&myICM, false);
	ICM_20948_low_power(&myICM, false);
}

void loop() {
  delay(1000);

  ICM_20948_AGMT_t agmt = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0}};
  if (ICM_20948_get_agmt(&myICM, &agmt) == ICM_20948_Stat_Ok)
  {
    printRawAGMT(agmt);
  }
  else
  {
    Serial.println("Uh oh");
  }
}



