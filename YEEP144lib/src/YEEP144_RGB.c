#include "YEEP144.h"
#include "YEEP144_RGB.h"
#include "YEEP144_I2C.h"
#include "stm32h5xx_hal_i2c.h"

#define RGB_ADDR 0x50
#define RGB_CHIP_EN_REG 0x000
#define RGB_DEV_CONFIG_0_REG 0x001
#define RGB_DEV_CONFIG_1_REG 0x002
#define RGB_DEV_CONFIG_2_REG 0x003
#define RGB_DEV_CONFIG_3_REG 0x004
#define RGB_DEV_CONFIG_4_REG 0x005
#define RGB_DEV_CONFIG_5_REG 0x006
#define RGB_DEV_CONFIG_6_REG 0x007
#define RGB_DEV_CONFIG_7_REG 0x008
#define RGB_DEV_CONFIG_8_REG 0x009
#define RGB_DEV_CONFIG_9_REG 0x00A
#define RGB_DEV_CONFIG_10_REG 0x00B
#define RGB_DEV_CONFIG_11_REG 0x00C
#define RGB_DEV_CONFIG_12_REG 0x00D
#define RGB_CMD_UPDATE_REG 0x010
#define RGB_CMD_START_REG 0x011
#define RGB_CMD_STOP_REG 0x012
#define RGB_CMD_PAUSE_REG 0x013
#define RGB_CMD_CONTINUE_REG 0x014
#define RGB_LED_EN_1_REG 0x020
#define RGB_LED_EN_2_REG 0x021
#define RGB_FAULT_CLEAR_REG 0x022
#define RGB_RESET_REG 0x023
#define RGB_MANUAL_DC_BASE_REG 0x030

#define RGB_MANUAL_DC_4_REG 0x034
#define RGB_MANUAL_DC_5_REG 0x035
#define RGB_MANUAL_DC_6_REG 0x036
#define RGB_MANUAL_DC_7_REG 0x037
#define RGB_MANUAL_DC_8_REG 0x038
#define RGB_MANUAL_DC_9_REG 0x039
#define RGB_MANUAL_DC_10_REG 0x03A
#define RGB_MANUAL_DC_11_REG 0x03B
#define RGB_MANUAL_DC_12_REG 0x03C
#define RGB_MANUAL_DC_13_REG 0x03D
#define RGB_MANUAL_DC_14_REG 0x03E
#define RGB_MANUAL_DC_15_REG 0x03F
#define RGB_MANUAL_PWM_BASE_REG 0x040

#define RGB_MANUAL_PWM_4_REG 0x044
#define RGB_MANUAL_PWM_5_REG 0x045
#define RGB_MANUAL_PWM_6_REG 0x046
#define RGB_MANUAL_PWM_7_REG 0x047
#define RGB_MANUAL_PWM_8_REG 0x048
#define RGB_MANUAL_PWM_9_REG 0x049
#define RGB_MANUAL_PWM_10_REG 0x04A
#define RGB_MANUAL_PWM_11_REG 0x04B
#define RGB_MANUAL_PWM_12_REG 0x04C
#define RGB_MANUAL_PWM_13_REG 0x04D
#define RGB_MANUAL_PWM_14_REG 0x04E
#define RGB_MANUAL_PWM_15_REG 0x04F

#define I2C_Timeout 1000

/**
 * Initialise the RGB led driver. This requires that the I2C has already been initialised
 */
void YEEP144_RGB_Init(void){

    // Reset the LP5812 - 0x66 is magic number according to datasheet
    YEEP144_RGB_WriteReg(RGB_RESET_REG, 0x66);

    // Enable the chip
    YEEP144_RGB_WriteReg(RGB_CHIP_EN_REG, 1);

    // Set the scan mode - 24kHz PWM, scan drive mode with 4 scans, no direct drive outputs
    YEEP144_RGB_WriteReg(RGB_DEV_CONFIG_1_REG, 0x40);

    // Set protection options - shutdown on open LED detected, shutdown on short LED detected, short threshold 0.65 * VCC
    YEEP144_RGB_WriteReg(RGB_DEV_CONFIG_12_REG, 0x0F);

    // Apply updated configuration - 0x55 is magic number as per datasheet
    YEEP144_RGB_WriteReg(RGB_CMD_UPDATE_REG, 0x55);

    // Set red current limits to 50% of max 25.2mA
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB1_RED , 0xFF / 2);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB2_RED , 0xFF / 2);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB3_RED , 0xFF / 2);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB4_RED , 0xFF / 2);

    // Set green and blue current limits to 20% of max

    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB1_GREEN , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB1_BLUE , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB2_GREEN , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB2_BLUE , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB3_GREEN , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB3_BLUE , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB4_GREEN , 0xFF / 5);
    YEEP144_RGB_WriteReg(RGB_MANUAL_DC_BASE_REG + RGB4_BLUE , 0xFF / 5);

    // Enable all the LEDS
    YEEP144_RGB_WriteReg(RGB_LED_EN_1_REG, 0xF0);
    YEEP144_RGB_WriteReg(RGB_LED_EN_2_REG, 0xFF);

    // Set default colours
    YEEP144_RGB_SetRGB(1, RGB_GREEN);
    YEEP144_RGB_SetRGB(2, RGB_GOLD);
    YEEP144_RGB_SetRGB(3, RGB_RED);
    YEEP144_RGB_SetRGB(4, RGB_INDIGO);
    
}

/**
 * Individually sets the brightness of an LED. Use the defines in the header file
 * to specify which LED e.g. RGB1_GREEN
 */
void YEEP144_RGB_SetLED(uint8_t led, uint8_t value){
    YEEP144_RGB_WriteReg(RGB_MANUAL_PWM_BASE_REG + led, value);
}

/**
 * Sets the brightness for all 3 elements of an RGB led at one.
 * Instead of specifiying the RGB values, you can also substitute in one of the defines e.g. RGB_GOLD
 */
void YEEP144_RGB_SetRGB(uint8_t led, uint8_t red, uint8_t green, uint8_t blue){
    uint8_t offset = ((led - 1) * 3);
    YEEP144_RGB_WriteReg(RGB_MANUAL_PWM_BASE_REG + RGB1_RED + offset, red);
    YEEP144_RGB_WriteReg(RGB_MANUAL_PWM_BASE_REG + RGB1_GREEN + offset, green);
    YEEP144_RGB_WriteReg(RGB_MANUAL_PWM_BASE_REG + RGB1_BLUE + offset, blue);
}

/**
 * Helper function that writes a byte to the RGB controller at the specified register address
 */
void YEEP144_RGB_WriteReg(uint16_t reg, uint8_t data){
    // As there are so many registers in this chip, it uses a couple of the I2C address bits as part of the register address
    // This means we need to do some bit level manipulation to build our transmission address and data

    uint8_t I2CAddress = RGB_ADDR + (reg >> 8); // set the bottom two bits of the address to the top two bits of the register

    // Cast the register to a uint8_t so we just get the lowest 8 bits
    uint8_t reg8Bit = (uint8_t) reg;

    // Pack out register address and data into an array for transmission
    uint8_t RGBCommand[2];
    RGBCommand[0] = reg8Bit;
    RGBCommand[1] = data;

    // Send out the data. The constructed I2CAddress still needs to be shifted by 1 due to 7/8-bit addressing differences
    HAL_I2C_Master_Transmit(&hi2c2, I2CAddress << 1, RGBCommand, 2, I2C_Timeout);
}