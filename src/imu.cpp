/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   2/17/2025
File:   imu.cpp

Purp: Establishing I2C communication with the ICM-20948 IMU sensor 
  and printing readings.

Derived from SparkFun 9DoF IMU Breakout - ICM 20948 Example999_Portable.ino
by Owen Lyke @ SparkFun Electronics, 2019
--------------------------------------------------------------------*/

#include "ICM_20948.h"
#include "imu.h"

#define SERIAL_PORT Serial

/*
 * Verify whoami register to make sure the ICM-20948 is connected.
 */
void findIMU(ICM_20948_Device_t* myICM){
	while (ICM_20948_check_id(myICM) != ICM_20948_Stat_Ok)
  {
    Serial.println("whoami does not match. Halting...");
    delay(1000);
  }

	ICM_20948_Status_e stat = ICM_20948_Stat_Err;
  uint8_t whoami = 0x00;
  while ((stat != ICM_20948_Stat_Ok) || (whoami != ICM_20948_WHOAMI))
	// While setup goes wrong.
  {
    whoami = 0x00;
    stat = ICM_20948_get_who_am_i(myICM, &whoami);
    Serial.print("whoami does not match (0x");
    Serial.print(whoami, HEX);
    Serial.print("). Halting...");
    Serial.println();
    delay(1000);
  }
}

void configureSensors(ICM_20948_Device_t* myICM){
	// Set Gyro and Accelerometer to a particular sample mode
	ICM_20948_set_sample_mode(myICM,
		(ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous);
		// options: ICM_20948_Sample_Mode_Continuous or ICM_20948_Sample_Mode_Cycled.

	// Set full scale ranges for both acc and gyr
	ICM_20948_fss_t myfss;
	myfss.a = gpm2;   // (ICM_20948_ACCEL_CONFIG_FS_SEL_e)
	myfss.g = dps250; // (ICM_20948_GYRO_CONFIG_1_FS_SEL_e)
	ICM_20948_set_full_scale(myICM, (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myfss);
}

/*
 * Set up the digital low pass filter to get rid of noisey data.
 */
void setupLowPassFilter(ICM_20948_Device_t* myICM){
  // Set up DLPF configuration. (Digital Low-Pass Filter)
  ICM_20948_dlpcfg_t myDLPcfg;
  myDLPcfg.a = acc_d473bw_n499bw;
  myDLPcfg.g = gyr_d361bw4_n376bw5;
  ICM_20948_set_dlpf_cfg(myICM, (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);

	// Choose whether or not to use DLPF
	ICM_20948_enable_dlpf(myICM, ICM_20948_Internal_Acc, false);
	ICM_20948_enable_dlpf(myICM, ICM_20948_Internal_Gyr, false);
}

//////////////////////
/* Helper Functions */
//////////////////////

/*
 * Adds leading zeros so that it prints a 5-digit number.
 */
void printPaddedInt16b(int16_t val)
{
  if (val > 0)
  {
    SERIAL_PORT.print(" ");
    if (val < 10000)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 1000)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 100)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 10)
    {
      SERIAL_PORT.print("0");
    }
  }
  else
  {
    SERIAL_PORT.print("-");
    if (abs(val) < 10000)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 1000)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 100)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 10)
    {
      SERIAL_PORT.print("0");
    }
  }
  SERIAL_PORT.print(abs(val));
}

/* 
 * Print the raw readings of all four sensors to the serial port.
 */
void printRawAGMT(ICM_20948_AGMT_t agmt)
{
  SERIAL_PORT.print("RAW. Acc [ ");
  printPaddedInt16b(agmt.acc.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.acc.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.acc.axes.z);
  SERIAL_PORT.print(" ], Gyr [ ");
  printPaddedInt16b(agmt.gyr.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.gyr.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.gyr.axes.z);
  SERIAL_PORT.print(" ], Mag [ ");
  printPaddedInt16b(agmt.mag.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.mag.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.mag.axes.z);
  SERIAL_PORT.print(" ], Tmp [ ");
  printPaddedInt16b(agmt.tmp.val);
  SERIAL_PORT.print(" ]");
  SERIAL_PORT.println();
}

/*
 * Convert accelerometer reading to milli-Gs (where 1g = 9.8 m/s^2).
 */
float getAccMG(int16_t raw, uint8_t fss)
{
  switch (fss)
  {
  case 0:
    return (((float)raw) / 16.384);
    break;
  case 1:
    return (((float)raw) / 8.192);
    break;
  case 2:
    return (((float)raw) / 4.096);
    break;
  case 3:
    return (((float)raw) / 2.048);
    break;
  default:
    return 0;
    break;
  }
}

/*
 * Convert gyroscope reading to degrees per second.
 */
float getGyrDPS(int16_t raw, uint8_t fss)
{
  switch (fss)
  {
  case 0:
    return (((float)raw) / 131);
    break;
  case 1:
    return (((float)raw) / 65.5);
    break;
  case 2:
    return (((float)raw) / 32.8);
    break;
  case 3:
    return (((float)raw) / 16.4);
    break;
  default:
    return 0;
    break;
  }
}

/*
 * Convert magnetometer reading to micro-Teslas.
 */
float getMagUT(int16_t raw)
{
  return (((float)raw) * 0.15);
}

/*
 * Convert temperature reading to degrees Celsius.
 */
float getTmpC(int16_t raw)
{
  return (((float)raw) / 333.87);
}