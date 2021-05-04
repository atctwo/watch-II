/**
 * @file watch2_battery.cpp
 * @author atctwo
 * @brief functions for handling the battery
 * @version 0.1
 * @date 2021-01-05
 * 
 * so far, this file only holds functions for handling the MAX17043G lipo fuel gauge (to determine how much charge is left
 * in the battery).  the functions are slightly modified versions of SparkFun's example code, which can be found at
 * https://learn.sparkfun.com/tutorials/wireless-joystick-hookup-guide#MAX17043
 * 
 * I'm using their breakout board to prototype the watch, which can be found at https://www.sparkfun.com/products/10617
 * 
 * @copyright Copyright (c) 2021 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    bool is_fuel_gauge_present = false;
    
    unsigned int vcellMAX17043()
    {
        if (!is_fuel_gauge_present) return 0.0;

        unsigned int vcell;

        vcell = i2cRead16(0x02);
        vcell = vcell >> 4;  // last 4 bits of vcell are nothing

        return vcell;
    }

    float percentMAX17043()
    {
        Serial.println("kill me");
        if (!is_fuel_gauge_present) return 0.0;

        unsigned int soc;
        float percent;

        soc = i2cRead16(0x04);  // Read SOC register of MAX17043
        percent = (byte) (soc >> 8);  // High byte of SOC is percentage
        percent += ((float)((byte)soc))/256;  // Low byte is 1/256%

        return percent;
    }

    bool configMAX17043(byte percent)
    {
        // check if the device is present
        Wire.beginTransmission(I2C_ADDRESS_MAX17043);
        uint8_t error = Wire.endTransmission();
        if (error != 0) {
            is_fuel_gauge_present = false;
            return false;
        }


        // set the alert percentage
        if ((percent >= 32)||(percent == 0))  // Anything 32 or greater will set to 32%
            i2cWrite16(0x9700, 0x0C);
        else
        {
            byte percentBits = 32 - percent;
            i2cWrite16((0x9700 | percentBits), 0x0C);
        }

        is_fuel_gauge_present = true;
        return true;
    }

    void qsMAX17043()
    {
        i2cWrite16(0x4000, 0x06);  // Write a 0x4000 to the MODE register
    }

    unsigned int i2cRead16(unsigned char address)
    {
        int data = 0;

        Wire.beginTransmission(I2C_ADDRESS_MAX17043);
        Wire.write(address);
        Wire.endTransmission();

        Wire.requestFrom(I2C_ADDRESS_MAX17043, 2);
        while (Wire.available() < 2)
            ;
        data = ((int) Wire.read()) << 8;
        data |= Wire.read();

        return data;
    }

    void i2cWrite16(unsigned int data, unsigned char address)
    {
        Wire.beginTransmission(I2C_ADDRESS_MAX17043);
        Wire.write(address);
        Wire.write((byte)((data >> 8) & 0x00FF));
        Wire.write((byte)(data & 0x00FF));
        Wire.endTransmission();
    }

}