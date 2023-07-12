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
 * FILE NAME  :  M3_L3_T6_IMU-Euler.c
 * DESCRIPTION:  This program reads the Accelerometer, Gyroscope
 *               and Magnetometer values (with calibration) from 
 *               the LSM9DS1 IMU sensor then using Madgwick's 
 *               quaternion based filter to generate the rotation 
 *               or the Euler angles (roll, pitch, yaw).
 * 
 *               The program's output is complement to the 
 *               M3_L3_T6_YPRDisplay.exe, which able to visualize the
 *               3D rotation.
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

#define LSM9DS1_COMMUNICATION_TIMEOUT 1000

#define SENSITIVITY_ACCELEROMETER_2 0.000061
#define SENSITIVITY_ACCELEROMETER_4 0.000122
#define SENSITIVITY_ACCELEROMETER_8 0.000244
#define SENSITIVITY_ACCELEROMETER_16 0.000732

#define SENSITIVITY_GYROSCOPE_245 0.00748
#define SENSITIVITY_GYROSCOPE_500 0.01526
#define SENSITIVITY_GYROSCOPE_2000 0.06

#define SENSITIVITY_MAGNETO_4 0.00014
#define SENSITIVITY_MAGNETO_8 0.00029
#define SENSITIVITY_MAGNETO_12 0.00043
#define SENSITIVITY_MAGNETO_16 0.00058

// Other definitions
#define RAD_TO_DEG 180 / M_PI
#define BILLION 1E9
#define BETA 0.6
#define INVSAMPLE 1 / 256

mraa_i2c_context I2Chandle, I2Chandle_mag;
mraa_uart_context UARThandle;

int16_t ax, ay, az, gx, gy, gz, mx, my, mz;
float gBiasRaw[3], aBiasRaw[3], mBiasRaw[3];
float aRes, gRes, mRes;
float gBias[3], aBias[3], mBias[3];
float AccVal[3], GyroVal[3], MagVal[3];
bool calibrate = FALSE;

float pitch, roll, yaw;
float heading, headingDegrees, headingFiltered, declination;
float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
float beta, sampleFreqDef, invSampleFreq;

FILE *fp;

void ReadAccel()
{
    uint8_t tempBuffer[6];
    mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_XL, tempBuffer, 6);
    ax = (tempBuffer[1] << 8) | tempBuffer[0];
    ay = (tempBuffer[3] << 8) | tempBuffer[2];
    az = (tempBuffer[5] << 8) | tempBuffer[4];

    if (calibrate)
    {
        ax -= aBiasRaw[0];
        ay -= aBiasRaw[1];
        az -= aBiasRaw[2];
    }

    AccVal[0] = aRes * ax;
    AccVal[1] = aRes * ay;
    AccVal[2] = aRes * az;

    // printf("aBias[0]: %5.4f aBias[1]: %5.4f aBias[2]: %5.4f\n", ax, ay, az);
}

void ReadGyro()
{
    uint8_t tempBuffer[6];
    mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_G, tempBuffer, 6);

    gx = (tempBuffer[1] << 8) | tempBuffer[0]; // Store x-axis values into gx
    gy = (tempBuffer[3] << 8) | tempBuffer[2]; // Store y-axis values into gy
    gz = (tempBuffer[5] << 8) | tempBuffer[4]; // Store z-axis values into gz

    if (calibrate)
    {
        gx -= gBiasRaw[0];
        gy -= gBiasRaw[1];
        gz -= gBiasRaw[2];
    }

    GyroVal[0] = gRes * gx;
    GyroVal[1] = gRes * gy;
    GyroVal[2] = gRes * gz;
}

void ReadMag()
{
    uint8_t tempBuffer[6];
    mraa_i2c_read_bytes_data(I2Chandle_mag, OUT_X_L_M, tempBuffer, 6);
    mx = (tempBuffer[1] << 8 | tempBuffer[0]);
    my = (tempBuffer[3] << 8 | tempBuffer[2]);
    mz = (tempBuffer[5] << 8 | tempBuffer[4]);

    MagVal[0] = mRes * mx;
    MagVal[1] = mRes * my;
    MagVal[2] = mRes * mz;
}

float calcAccel(float accel)
{
    //Return the accel raw reading times our pre-calculated g's / (ADC tick):
    return aRes * accel;
}

float calcGyro(float gyro)
{
    // Return the gyro raw reading times our pre-calculated DPS / (ADC tick):
    return gRes * gyro;
}

float calcMag(float mag)
{
    // Return the mag raw reading times our pre-calculated Gs / (ADC tick):
    return mRes * mag;
}

void agCalibration()
{
    aBiasRaw[0] = 0;
    aBiasRaw[1] = 0;
    aBiasRaw[2] = 0;
    gBiasRaw[0] = 0;
    gBiasRaw[1] = 0;
    gBiasRaw[2] = 0;

    uint8_t data[6] = {0, 0, 0, 0, 0, 0};
    int ii;
    float aBiasRawTemp[3] = {0, 0, 0};
    float gBiasRawTemp[3] = {0, 0, 0};

    uint8_t samples = 35;

    for (ii = 0; ii < samples; ii++)
    {
        ReadAccel();
        aBiasRawTemp[0] += ax;
        aBiasRawTemp[1] += ay;
        aBiasRawTemp[2] += az - (int16_t)(1. / aRes);

        ReadGyro();
        gBiasRawTemp[0] += gx;
        gBiasRawTemp[1] += gy;
        gBiasRawTemp[2] += gz;
        // printf("aBias[0]: %5.2f aBias[1]: %5.2f\n", ax, ay);
    }

    for (ii = 0; ii < 3; ii++)
    {
        gBiasRaw[ii] = gBiasRawTemp[ii] / samples;
        gBias[ii] = calcGyro(gBiasRaw[ii]);
        aBiasRaw[ii] = aBiasRawTemp[ii] / samples;
        aBias[ii] = calcAccel(aBiasRaw[ii]);
    }

    // printf("aBias[0]: %5.2f aBias[1]: %5.2f\n", aBiasRaw[0], aBiasRaw[1]);
}

void magOffset(uint8_t axis, int16_t offset)
{
    if (axis > 2)
        return;
    uint8_t msb, lsb;
    msb = (offset & 0xFF00) >> 8;
    lsb = offset & 0x00FF;
    mraa_i2c_write_byte_data(I2Chandle_mag, lsb, OFFSET_X_REG_L_M + (2 * axis));
    mraa_i2c_write_byte_data(I2Chandle_mag, msb, OFFSET_X_REG_H_M + (2 * axis));
}

void magCalibration()
{
    int i, j;
    int16_t magMin[3] = {0, 0, 0};
    int16_t magMax[3] = {0, 0, 0};

    int size = 128;

    for (i = 0; i < size; i++)
    {
        int16_t magTemp[3] = {0, 0, 0};
        magTemp[0] = mx;
        magTemp[1] = my;
        magTemp[2] = mz;
        for (j = 0; j < 3; j++)
        {
            if (magTemp[j] > magMax[j])
                magMax[j] = magTemp[j];
            if (magTemp[j] < magMin[j])
                magMin[j] = magTemp[j];
        }
    }

    for (j = 0; j < 3; j++)
    {
        mBiasRaw[j] = (magMax[j] + magMin[j]) / 2;
        mBias[j] = calcMag(mBiasRaw[j]);
        magOffset(j, mBiasRaw[j]);
    }
}

void Magnetometer_Init()
{
    int settings_MAG_XYPerformance = 3;
    int settings_MAG_ZPerformance = 3;
    int settings_MAG_sampleRate = 7;
    int settings_MAG_scale = 4;
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

void Gyroscope_Init()
{
    // Gyroscope settings
    uint8_t tempRegValue = 0;
    uint8_t sampleRate;
    uint8_t bandwidth;
    uint8_t scale;

    // Initialize CTRL_REG1_G (10h) register :[ODR_G2][ODR_G1][ODR_G0][FS_G1][FS_G0][0][BW_G1][BW_G0]

    // FS_G- gyro sample rate: value between 1-6
    // 1 = 14.9    4 = 238
    // 2 = 59.5    5 = 476
    // 3 = 119     6 = 952
    sampleRate = 6;

    // ODR_G- gyro scale can be 245, 500, or 2000
    scale = 245; // Setting default to 245

    // gyro cutoff frequency: value between 0-3
    // Actual value of cutoff frequency depends on sample rate.
    bandwidth = 0;

    tempRegValue = (sampleRate & 0x07) << 5;
    if (scale == 500)
    {
        tempRegValue |= (0x1 << 3);
        gRes = SENSITIVITY_GYROSCOPE_500;
    }
    else if (scale == 2000)
    {
        tempRegValue |= (0x3 << 3);
        gRes = SENSITIVITY_GYROSCOPE_2000;
    }
    else
    {
        gRes = SENSITIVITY_GYROSCOPE_245;
    }

    tempRegValue |= (bandwidth & 0x3);

    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, CTRL_REG1_G);

    // CTRL_REG2_G (Default value: 0x00)
    // [0][0][0][0][INT_SEL1][INT_SEL0][OUT_SEL1][OUT_SEL0]
    // INT_SEL[1:0] - INT selection configuration
    // OUT_SEL[1:0] - Out selection configuration
    mraa_i2c_write_byte_data(I2Chandle, 0x00, CTRL_REG2_G);

    // CTRL_REG3_G (Default value: 0x00): [LP_mode][HP_EN][0][0][HPCF3_G][HPCF2_G][HPCF1_G][HPCF0_G]
    mraa_i2c_write_byte_data(I2Chandle, 0x00, CTRL_REG3_G);

    // CTRL_REG4 (Default value: 0x38)
    // [0][0][Zen_G][Yen_G][Xen_G][0][LIR_XL1][4D_XL1]
    // Zen_G - Z-axis output enable (0:disable, 1:enable)
    // Yen_G - Y-axis output enable (0:disable, 1:enable)
    // Xen_G - X-axis output enable (0:disable, 1:enable)
    // LIR_XL1 - Latched interrupt (0:not latched, 1:latched)
    // 4D_XL1 - 4D option on interrupt (0:6D used, 1:4D used)
    tempRegValue = 0;
    tempRegValue |= (1 << 5);
    tempRegValue |= (1 << 4);
    tempRegValue |= (1 << 3);
    tempRegValue |= (1 << 1); //Latched Interrupt ?? need to check the behavior
    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, CTRL_REG4);

    // ORIENT_CFG_G (Default value: 0x00)
    // [0][0][SignX_G][SignY_G][SignZ_G][Orient_2][Orient_1][Orient_0]
    // SignX_G - Pitch axis (X) angular rate sign (0: positive, 1: negative)
    // Orient [2:0] - Directional user orientation selection
    tempRegValue = 0;
    tempRegValue |= (1 << 5);
    tempRegValue |= (1 << 4);
    tempRegValue |= (1 << 3);
    mraa_i2c_write_byte_data(I2Chandle, tempRegValue, ORIENT_CFG_G);
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

float invSqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

void implementMadgwickFilter(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float hx, hy;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))
    {
    }

    gx *= 0.0174533f;
    gy *= 0.0174533f;
    gz *= 0.0174533f;

    qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
    qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
    qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        recipNorm = invSqrt(mx * mx + my * my + mz * mz);
        mx *= recipNorm;
        my *= recipNorm;
        mz *= recipNorm;

        _2q0mx = 2.0f * q0 * mx;
        _2q0my = 2.0f * q0 * my;
        _2q0mz = 2.0f * q0 * mz;
        _2q1mx = 2.0f * q1 * mx;
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _2q0q2 = 2.0f * q0 * q2;
        _2q2q3 = 2.0f * q2 * q3;
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;

        // Reference direction of Earth's magnetic field
        hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
        hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
        _2bx = sqrtf(hx * hx + hy * hy);
        _2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
        _4bx = 2.0f * _2bx;
        _4bz = 2.0f * _2bz;

        // Gradient decent algorithm corrective step
        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 -= BETA * s0;
        qDot2 -= BETA * s1;
        qDot3 -= BETA * s2;
        qDot4 -= BETA * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot1 * INVSAMPLE;
    q1 += qDot2 * INVSAMPLE;
    q2 += qDot3 * INVSAMPLE;
    q3 += qDot4 * INVSAMPLE;

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;
    // anglesComputed = 0;
}

void getHeadingAsYaw()
{
    heading = atan2f(MagVal[0], MagVal[1]);
    declination = 0.002;
    heading = heading + declination;
    if (heading < 0)
    {
        heading = heading + (2 * M_PI);
    }
    else if (heading > 2 * M_PI)
    {
        heading = heading - (2 * M_PI);
    }
    headingDegrees = heading * RAD_TO_DEG;
    yaw = headingDegrees;
}

void getAngles()
{
    roll = asinf(2 * (q0 * q2 - q3 * q1));
    pitch = atan2f((2 * (q0 * q1 + q2 * q3)), (1 - 2 * (q1 * q1 + q2 * q2)));
    yaw = atan2f((2 * (q0 * q3 + q1 * q2)), (1 - 2 * (q2 * q2 + q3 * q3)));

    roll = roll * RAD_TO_DEG;
    pitch = pitch * RAD_TO_DEG;
    yaw = yaw * RAD_TO_DEG;
}

void filtering()
{
    implementMadgwickFilter(GyroVal[0], GyroVal[1], GyroVal[2], AccVal[0], AccVal[1], AccVal[2], MagVal[0], MagVal[1], MagVal[2]);
    getAngles();
}

void ReadSensors()
{

    char buf[20];
    int ret;
    int i;

    while (1)
    {
        ReadAccel();
        ReadGyro();
        ReadMag();
        filtering();
        printf("%5.0f,%5.0f,%5.0f\n", yaw, pitch, roll);
        // fprintf(fp, "%5.2f, %5.2f, %5.2f\n", AccVal[0], AccVal[1], AccVal[2]);
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

    Accelerometer_Init();
    Gyroscope_Init();
    Magnetometer_Init();
    if (calibrate)
    {
        agCalibration();
    }
    ReadSensors();

    return 0;
}