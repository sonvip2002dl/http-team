/*************************************************
 *         © Keysight Technologies 2019 
 *
 * PROPRIETARY RIGHTS of Keysight Technologies are 
 * involved in the subject matter of this software. 
 * All manufacturing, reproduction, use, and sales 
 * rights pertaining to this software are governed 
 * by the license agreement. The recipient of this 
 * code implicitly accepts the terms of the license. 
 *
 ***************************************************
 *
 * FILE NAME  :  M3_L3_T7_compass-heading.c
 * DESCRIPTION:  This program reads the Accelerometer, and 
 *               Magnetometer values (with calibration) from 
 *               the LSM9DS1 IMU sensor then calculate the 
 *               heading degree on horizontal plane with 
 *               declination correction and with consideration 
 *               of the roll and pitch angle for tilt-compensation
 * 
 *               The program's output will be shown in the console
 *               as well as on the LCD display of the sensor board.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define LCD_ADDR 0x3E
#define I2C_SLAVE_ADDR 0x6B
#define I2C_SLAVE_ADDR_MAG 0x1E

// LSM9DS1 Accelerometer & Gyroscope Registers.
#define CTRL_REG1_G 0x10
#define CTRL_REG2_G 0x11
#define CTRL_REG3_G 0x12
#define CTRL_REG4 0x1E
#define CTRL_REG9 0x23
#define CTRL_REG5_XL 0x1F
#define CTRL_REG6_XL 0x20
#define CTRL_REG7_XL 0x21
#define OUT_X_L_XL 0x28
#define OUT_X_H_XL 0x29
#define OUT_Y_L_XL 0x2A
#define OUT_Y_H_XL 0x2B
#define OUT_Z_L_XL 0x2C
#define OUT_Z_H_XL 0x2D
#define OUT_X_L_G 0x18
#define FIFO_CTRL 0x2E
#define FIFO_SRC 0x2F
#define ORIENT_CFG_G 0x13

// LSM9DS1 Magneto Registers
#define OFFSET_X_REG_L_M 0x05
#define OFFSET_X_REG_H_M 0x06
#define OFFSET_Y_REG_L_M 0x07
#define OFFSET_Y_REG_H_M 0x08
#define OFFSET_Z_REG_L_M 0x09
#define OFFSET_Z_REG_H_M 0x0A
#define WHO_AM_I_M 0x0F
#define CTRL_REG1_M 0x20
#define CTRL_REG2_M 0x21
#define CTRL_REG3_M 0x22
#define CTRL_REG4_M 0x23
#define CTRL_REG5_M 0x24
#define STATUS_REG_M 0x27
#define OUT_X_L_M 0x28
#define OUT_X_H_M 0x29
#define OUT_Y_L_M 0x2A
#define OUT_Y_H_M 0x2B
#define OUT_Z_L_M 0x2C
#define OUT_Z_H_M 0x2D
#define INT_CFG_M 0x30
#define INT_SRC_M 0x30
#define INT_THS_L_M 0x32
#define INT_THS_H_M 0x33

#define SENSITIVITY_MAGNETO_4 0.00014
#define SENSITIVITY_MAGNETO_8 0.00029
#define SENSITIVITY_MAGNETO_12 0.00043
#define SENSITIVITY_MAGNETO_16 0.00058

#define SENSITIVITY_ACCELEROMETER_2 0.000061
#define SENSITIVITY_ACCELEROMETER_4 0.000122
#define SENSITIVITY_ACCELEROMETER_8 0.000244
#define SENSITIVITY_ACCELEROMETER_16 0.000732

// Other definitions
#define RAD_TO_DEG 180 / M_PI
#define BILLION 1E9
#define BETA 0.5
#define INVSAMPLE 1 / 256

mraa_i2c_context I2Chandle, I2Chandle_mag, i2cp;
int16_t ax, ay, az, mx, my, mz;
float aBiasRaw[3], mBiasRaw[3];
float aRes, mRes;
float aBias[3], mBias[3];
float AccVal[3], MagVal[3];
bool calibrate = TRUE;
bool tiltCompensated = FALSE;

float mxMin = 0, mxMax = 0, myMin = 0, myMax = 0, mzMin = 0, mzMax = 0;
float mxScale, myScale, mzScale;
float mxOffset, myOffset, mzOffset;
float mxCalibrated, myCalibrated, mzCalibrated;
float avgScale;

struct timespec start, stop, instant;
double time_elapse;

float pitch, roll;
float heading, headingDegrees, declination;

void home_LCD(void)
{
    uint8_t buf[2] = {0x00, 0x02};
    mraa_i2c_write(i2cp, buf, 2); //Set to Home
}

void LCD_Print(uint8_t *str)
{
    uint8_t buf[80] = {0}; // Set Buffer to all Null
    uint8_t buf1[2] = {0x00, 0x80};
    int32_t i = 0, strl, j = 0;
    buf[i] = 0x40; //register for display
    i++;
    strl = strlen((char *)str);
    for (j = 0; j < strl; j++)
    {
        buf[i] = str[j];
        i++;
    }

    mraa_i2c_write(i2cp, buf1, 2);
    mraa_i2c_write(i2cp, buf, i);
}

void LCD_init(void)
{
    uint8_t init1[2] = {0x00, 0x38};
    uint8_t init2[8] = {0x00, 0x39, 0x14, 0x74, 0x54, 0x6f, 0x0c, 0x01};
    // 2 lines 8 bit 3.3V Version
    mraa_i2c_write(i2cp, init1, 2);
    mraa_i2c_write(i2cp, init2, 8); //Function Set
}

void clear_LCD(void)
{
    uint8_t buf[2] = {0x00, 0x01};
    mraa_i2c_write(i2cp, buf, 2); //Clear Display
}

void ReadAccel()
{
    uint8_t tempBuffer[6];
    mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_XL, tempBuffer, 6);
    ax = (tempBuffer[1] << 8) | tempBuffer[0];
    ay = (tempBuffer[3] << 8) | tempBuffer[2];
    az = (tempBuffer[5] << 8) | tempBuffer[4];

    if (calibrate)
    {
        ax = ax - aBiasRaw[0];
        ay = ax - aBiasRaw[1];
        az = ax - aBiasRaw[2];
    }

    AccVal[0] = aRes * ax;
    AccVal[1] = aRes * ay;
    AccVal[2] = aRes * az;
}

void ReadMag()
{
    char buf[20];
    uint8_t tempBuffer[6];
    mraa_i2c_read_bytes_data(I2Chandle_mag, OUT_X_L_M, tempBuffer, 6);
    mx = (tempBuffer[1] << 8 | tempBuffer[0]);
    my = (tempBuffer[3] << 8 | tempBuffer[2]);
    mz = (tempBuffer[5] << 8 | tempBuffer[4]);

    MagVal[0] = mRes * mx;
    MagVal[1] = mRes * my;
    MagVal[2] = mRes * mz;

    if (calibrate)
    {
        MagVal[0] = (MagVal[0] - mxOffset) / mxScale;
        MagVal[1] = (MagVal[1] - myOffset) / myScale;
        MagVal[2] = (MagVal[2] - mzOffset) / mzScale;
    }

    if (tiltCompensated)
    {
        float accXnorm, accYnorm, magXcomp, magYcomp;
        accXnorm = AccVal[0] / sqrt(AccVal[0] * AccVal[0] + AccVal[1] * AccVal[1] + AccVal[2] * AccVal[2]);
        accYnorm = AccVal[1] / sqrt(AccVal[0] * AccVal[0] + AccVal[1] * AccVal[1] + AccVal[2] * AccVal[2]);

        // tilt-compensation to include roll and pitch
        pitch = asin(accXnorm);
        roll = -asin(accYnorm / cos(pitch));
        magXcomp = MagVal[0] * cos(pitch) + MagVal[2] * sin(pitch);
        magYcomp = MagVal[0] * sin(roll) * sin(pitch) + MagVal[1] * cos(roll) - MagVal[2] * sin(roll) * cos(pitch);

        heading = atan2f(magYcomp, magXcomp);
    }
    else
    {
        heading = atan2f(MagVal[1], MagVal[0]);
    }

    // heading = atan2f(myCalibrated, mxCalibrated);
    declination = 0.002;
    heading = heading + declination;
    if (heading < 0)
    {
        heading = heading + (2 * M_PI);
    }
    else if (heading > (2 * M_PI))
    {
        heading = heading - (2 * M_PI);
    }
    headingDegrees = heading * RAD_TO_DEG;

    char rep[] = "";
    if (headingDegrees <= 22.5 || headingDegrees > 337.25)
    {
        strcpy(rep, "N");
    }
    else if (headingDegrees <= 337.25 && headingDegrees > 292.5)
    {
        strcpy(rep, "NW");
    }
    else if (headingDegrees <= 292.5 && headingDegrees > 247.5)
    {
        strcpy(rep, "W");
    }
    else if (headingDegrees <= 247.5 && headingDegrees > 202.5)
    {
        strcpy(rep, "SW");
    }
    else if (headingDegrees <= 202.5 && headingDegrees > 157.5)
    {
        strcpy(rep, "S");
    }
    else if (headingDegrees <= 157.5 && headingDegrees > 112.5)
    {
        strcpy(rep, "SE");
    }
    else if (headingDegrees <= 112.5 && headingDegrees > 67.5)
    {
        strcpy(rep, "E");
    }
    else if (headingDegrees <= 67.5 && headingDegrees > 0)
    {
        strcpy(rep, "NE");
    }
    sprintf(buf, "Heading: %3.0f %2s", headingDegrees, rep);
    LCD_Print((uint8_t *)buf);
}

float calcMag(float mag)
{
    // Return the mag raw reading times our pre-calculated Gs / (ADC tick):
    return mRes * mag;
}

float calcAccel(float accel)
{
    //Return the accel raw reading times our pre-calculated g's / (ADC tick):
    return aRes * accel;
}

void accelCalibration()
{
    aBiasRaw[0] = 0;
    aBiasRaw[1] = 0;
    aBiasRaw[2] = 0;

    uint8_t data[6] = {0, 0, 0, 0, 0, 0};
    int ii;
    float aBiasRawTemp[3] = {0, 0, 0};

    uint8_t samples = 35;

    for (ii = 0; ii < samples; ii++)
    {
        ReadAccel();
        aBiasRawTemp[0] += ax;
        aBiasRawTemp[1] += ay;
        aBiasRawTemp[2] += az - (int16_t)(1. / aRes);
    }

    for (ii = 0; ii < 3; ii++)
    {
        aBiasRaw[ii] = aBiasRawTemp[ii] / samples;
        aBias[ii] = calcAccel(aBiasRaw[ii]);
    }
}

void magF8Calibration()
{
    clock_gettime(CLOCK_REALTIME, &start);

    uint8_t tempBuffer[6];
    char buf[20];
    mraa_i2c_read_bytes_data(I2Chandle_mag, OUT_X_L_M, tempBuffer, 6);
    mx = (tempBuffer[1] << 8) | tempBuffer[0];
    my = (tempBuffer[3] << 8) | tempBuffer[2];
    mz = (tempBuffer[5] << 8) | tempBuffer[4];

    MagVal[0] = mRes * mx;
    MagVal[1] = mRes * my;
    MagVal[2] = mRes * mz;

    sprintf(buf, "Calibrating......");
    LCD_Print((uint8_t *)buf);

    sleep(1);
    while (true)
    {
        if (MagVal[0] > mxMax)
        {
            mxMax = MagVal[0];
        }
        if (MagVal[1] > myMax)
        {
            myMax = MagVal[1];
        }
        if (MagVal[2] > mzMax)
        {
            mzMax = MagVal[2];
        }
        if (MagVal[0] < mxMin)
        {
            mxMin = MagVal[0];
        }
        if (MagVal[1] < myMin)
        {
            myMin = MagVal[1];
        }
        if (MagVal[2] < mzMin)
        {
            mzMin = MagVal[2];
        }

        mxOffset = (mxMin + mxMax) / 2;
        myOffset = (myMin + myMax) / 2;
        mzOffset = (mzMin + mzMax) / 2;

        mxScale = (mxMax - mxMin) / 2;
        myScale = (myMax - myMin) / 2;
        mzScale = (mzMax - mzMin) / 2;

        avgScale = (mxScale + myScale + mzScale) / 3;

        clock_gettime(CLOCK_REALTIME, &stop);
        time_elapse = stop.tv_sec + (float)stop.tv_nsec / BILLION - (start.tv_sec + (float)start.tv_nsec / BILLION);
        //printf("Max Mx:%5.2f, Min Mx:%5.2f",mxMax, mxMin);
        if (time_elapse > 10)
        {
            break;
        }
    }
}

void Magnetometer_Init()
{
    int settings_MAG_XYPerformance = 3;
    int settings_MAG_ZPerformance = 3;
    int settings_MAG_sampleRate = 7;
    int settings_MAG_scale = 16;
    int settings_MAG_operatingMode = 0;

    uint8_t tempRegValue = 0;

    // CTRL_REG1_M (Default value: 0x10)
    // [TEMP_COMP][OM1][OM0][DO2][DO1][DO0][0][ST]
    // TEMP_COMP - Temperature compensation
    // OM[1:0] - X & Y axes op mode selection
    //	00:low-power, 01:medium performance
    //	10: high performance, 11:ultra-high performance
    // DO[2:0] - Output data rate selection
    // ST - Self-test enable
    tempRegValue |= (1 << 7);
    tempRegValue |= ((settings_MAG_XYPerformance & 0x3) << 5);
    tempRegValue |= ((settings_MAG_sampleRate & 0x7) << 2);
    mraa_i2c_write_byte_data(I2Chandle_mag, tempRegValue, CTRL_REG1_M);

    // CTRL_REG2_M (Default value 0x00)
    // [0][FS1][FS0][0][REBOOT][SOFT_RST][0][0]
    // FS[1:0] - Full-scale configuration
    // REBOOT - Reboot memory content (0:normal, 1:reboot)
    // SOFT_RST - Reset config and user registers (0:default, 1:reset)
    tempRegValue = 0;
    if (settings_MAG_scale == 8)
    {
        tempRegValue |= (0x1 << 5);
        mRes = SENSITIVITY_MAGNETO_8;
    }
    else if (settings_MAG_scale == 12)
    {
        tempRegValue |= (0x2 << 5);
        mRes = SENSITIVITY_MAGNETO_12;
    }
    else if (settings_MAG_scale == 16)
    {
        tempRegValue |= (0x3 << 5);
        mRes = SENSITIVITY_MAGNETO_16;
    }
    else
    {
        mRes = SENSITIVITY_MAGNETO_4;
    }

    mraa_i2c_write_byte_data(I2Chandle_mag, tempRegValue, CTRL_REG2_M);

    // CTRL_REG3_M (Default value: 0x03)
    // [I2C_DISABLE][0][LP][0][0][SIM][MD1][MD0]
    // I2C_DISABLE - Disable I2C interace (0:enable, 1:disable)
    // LP - Low-power mode cofiguration (1:enable)
    // SIM - SPI mode selection (0:write-only, 1:read/write enable)
    // MD[1:0] - Operating mode
    //	00:continuous conversion, 01:single-conversion,
    //  10,11: Power-down
    tempRegValue = 0;
    tempRegValue |= (settings_MAG_operatingMode & 0x3);
    mraa_i2c_write_byte_data(I2Chandle_mag, tempRegValue, CTRL_REG3_M);

    // CTRL_REG4_M (Default value: 0x00)
    // [0][0][0][0][OMZ1][OMZ0][BLE][0]
    // OMZ[1:0] - Z-axis operative mode selection
    //	00:low-power mode, 01:medium performance
    //	10:high performance, 10:ultra-high performance
    // BLE - Big/little endian data
    tempRegValue = 0;
    tempRegValue = (settings_MAG_ZPerformance & 0x3) << 2;
    mraa_i2c_write_byte_data(I2Chandle_mag, tempRegValue, CTRL_REG4_M);

    // CTRL_REG5_M (Default value: 0x00)
    // [0][BDU][0][0][0][0][0][0]
    // BDU - Block data update for magnetic data
    //	0:continuous, 1:not updated until MSB/LSB are read
    tempRegValue = 0;
    mraa_i2c_write_byte_data(I2Chandle_mag, tempRegValue, CTRL_REG5_M);
}

void Accelerometer_Init()
{

    // Accelerometer settings
    uint8_t tempRegValue = 0;
    uint8_t sampleRate = 6;
    uint8_t scale = 2; // accel scale can be 2, 4, 8, or 16
    uint8_t highResBandwidth, highResEnable;
    uint8_t bandwidth;

    //	CTRL_REG5_XL (0x1F) (Default value: 0x38)
    //	[DEC_1][DEC_0][Zen_XL][Yen_XL][Zen_XL][0][0][0]
    //	DEC[0:1] - Decimation of accel data on OUT REG and FIFO.
    //		00: None, 01: 2 samples, 10: 4 samples 11: 8 samples
    //	Zen_XL - Z-axis output enabled
    //	Yen_XL - Y-axis output enabled
    //	Xen_XL - X-axis output enabled
    tempRegValue |= (1 << 5);
    tempRegValue |= (1 << 4);
    tempRegValue |= (1 << 3);

    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, CTRL_REG5_XL);

    // CTRL_REG6_XL (0x20) (Default value: 0x00)
    // [ODR_XL2][ODR_XL1][ODR_XL0][FS1_XL][FS0_XL][BW_SCAL_ODR][BW_XL1][BW_XL0]
    // ODR_XL[2:0] - Output data rate & power mode selection
    // FS_XL[1:0] - Full-scale selection
    // BW_SCAL_ODR - Bandwidth selection
    // BW_XL[1:0] - Anti-aliasing filter bandwidth selection
    tempRegValue = 0;

    // accel sample rate can be 1-6
    // 1 = 10 Hz    4 = 238 Hz
    // 2 = 50 Hz    5 = 476 Hz
    // 3 = 119 Hz   6 = 952 Hz
    sampleRate = 6;

    tempRegValue |= (sampleRate & 0x07) << 5;

    if (scale == 4)
    {
        tempRegValue |= (0x2 << 3);
        aRes = SENSITIVITY_ACCELEROMETER_4;
    }
    else if (scale == 8)
    {
        tempRegValue |= (0x3 << 3);
        aRes = SENSITIVITY_ACCELEROMETER_8;
    }
    else if (scale == 16)
    {
        tempRegValue |= (0x1 << 3);
        aRes = SENSITIVITY_ACCELEROMETER_16;
    }
    else
    {
        // Otherwise it'll be set to 2g (0x0 << 3)
        tempRegValue |= (0x0 << 3);
        aRes = SENSITIVITY_ACCELEROMETER_2;
    }

    // Accel cutoff freqeuncy can be any value between -1 - 3.
    // -1 = bandwidth determined by sample rate
    // 0 = 408 Hz   2 = 105 Hz
    // 1 = 211 Hz   3 = 50 Hz
    bandwidth = -1;
    if (bandwidth >= 0)
    {
        tempRegValue |= (1 << 2); // Set BW_SCAL_ODR
        tempRegValue |= (bandwidth & 0x03);
    }

    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, CTRL_REG6_XL);

    // CTRL_REG7_XL (0x21) (Default value: 0x00)
    // [HR][DCF1][DCF0][0][0][FDS][0][HPIS1]
    // HR - High resolution mode (0: disable, 1: enable)
    // DCF[1:0] - Digital filter cutoff frequency
    // FDS - Filtered data selection
    // HPIS1 - HPF enabled for interrupt function
    tempRegValue = 0;
    highResEnable = 0;
    if (highResEnable)
    {
        //tempRegValue |= (1<<7); // Set HR bit
        //tempRegValue |= (highResBandwidth & 0x3) << 5;
    }

    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, CTRL_REG7_XL);
}

void ReadSensorValues()
{
    char buf[20];
    int ret;
    while (1)
    {

        ReadAccel();
        ReadMag();
        // printf("%3.1f\n", headingDegrees);
        printf("mx: %4.2f, my: %4.2f, mz: %4.2f\n", MagVal[0], MagVal[1], MagVal[2]);
        // usleep(1000);
    }
}

int main(void)
{
    int status;
    I2Chandle = mraa_i2c_init_raw(I2CS_BUS); // write correct I2C dev number here
    if (!I2Chandle)
    { // The bus is not there
        printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
        printf(" -> Exiting program\n");
        mraa_i2c_stop(I2Chandle);
        mraa_deinit();
        return EXIT_FAILURE;
    }

    I2Chandle_mag = mraa_i2c_init_raw(I2CS_BUS); // write correct I2C dev number here
    if (!I2Chandle_mag)
    { // The bus is not there
        printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
        printf(" -> Exiting program\n");
        mraa_i2c_stop(I2Chandle_mag);
        mraa_deinit();
        return EXIT_FAILURE;
    }

    i2cp = mraa_i2c_init_raw(I2CP_BUS);
    if (!i2cp)
    { // The bus is not there
        printf("I2C Bus 2 is not available. Please check /dev/i2c-2 \n");
        printf(" -> Exiting program\n");
        mraa_i2c_stop(i2cp);
        mraa_deinit();
        return EXIT_FAILURE;
    }

    mraa_i2c_frequency(I2Chandle, MRAA_I2C_STD);
    mraa_i2c_address(I2Chandle, I2C_SLAVE_ADDR);
    status = mraa_i2c_read_byte(I2Chandle);
    if (status < 0)
    {
        printf("Failed to Initialize Accelerometer -> Exiting program\n");
        printf("Check i2cdetect -r -y 1 for 0x6B\n");
        mraa_i2c_stop(I2Chandle);
        mraa_deinit();
        return EXIT_FAILURE;
    }

    mraa_i2c_frequency(I2Chandle_mag, MRAA_I2C_STD);
    mraa_i2c_address(I2Chandle_mag, I2C_SLAVE_ADDR_MAG);
    status = mraa_i2c_read_byte(I2Chandle_mag);
    if (status < 0)
    {
        printf("Failed to Initialize Magnetometer -> Exiting program\n");
        printf("Check i2cdetect -r -y 1 for 0x1E\n");
        mraa_i2c_stop(I2Chandle_mag);
        mraa_deinit();
        return EXIT_FAILURE;
    }

    mraa_i2c_frequency(i2cp, MRAA_I2C_STD);
    mraa_i2c_address(i2cp, LCD_ADDR);
    status = mraa_i2c_read_byte(i2cp);
    if (status < 0)
    {
        printf("Failed to initialize display\n");
        printf("Check i2cdetect -r -y 2 for 0x3E\n");
        mraa_i2c_stop(i2cp);
        mraa_deinit();
        return EXIT_FAILURE;
    }
    clear_LCD();
    Accelerometer_Init();
    Magnetometer_Init();
    if (calibrate)
    {
        accelCalibration();
        printf("Calibration will start in 5 sec...\nMove the device in figure 8 pattern for 10 sec.\n");
        sleep(5);
        magF8Calibration();
    }
    ReadSensorValues();

    return 0;
}
