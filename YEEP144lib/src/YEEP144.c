#include "YEEP144.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_tim.h"

// Private Function Prototypes
void SystemClock_Config(void);
void GPIO_Init(void);

#define SYSTICK_LOAD (SystemCoreClock/1000000U)
#define SYSTICK_DELAY_CALIB (85)// Determined experimentally

/** 
* This function initialises the minimum required for basic operation
* Each feature on top of this will be initialised by its own init function
*/
int YEEP144_Init(void){
    HAL_Init();

    SystemClock_Config();

    return 0;
}

/**
 * Initialises the specified GPIO with basic configuration. This assumes it will be one of the standard modes
 * with no pull-ups or pull-downs, and a low frequency output
 */
void YEEP144_GPIO_Init(GPIO_TypeDef* port, uint32_t pins, uint32_t mode){
	// Initialise GPIO pins

    // Create structure to hold GPIO init data
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pins; // Pins used
    GPIO_InitStruct.Mode = mode; // Set pin mode
    GPIO_InitStruct.Pull = GPIO_NOPULL; // No pull-ups or pull-downs
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Low speed is sufficient for this task

    // Initialise the port
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * Initialises the specified GPIO with access to all of the configuration options, allowing you to configure
 * pull-ups / pull-downs, GPIO speed, and access alternate functions such as timer IO, SPI, I2C etc
 */
void YEEP144_GPIO_InitAdv(GPIO_TypeDef* port, uint32_t pins, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t alternate){
	// Initialise GPIO pins

    // Create structure to hold GPIO init data
    GPIO_InitTypeDef GPIO_InitStruct = {0};
 
    GPIO_InitStruct.Pin = pins; // Pins used
    GPIO_InitStruct.Mode = mode; // Set pin mode
    GPIO_InitStruct.Pull = pull; // Set pullup / pulldown
    GPIO_InitStruct.Speed = speed; // Set speed - low is good for up to 8MHz
	GPIO_InitStruct.Alternate = alternate; // Set alternate mode

    // Initialise the port
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
* Initialises and configures the system clocks for standard operation
*
* By default we are configured for the maximum clock speed of 250MHz on the core
* and on the APBx peripheral and timer clocks
*
* As we do not have a HSE clock due to the configuration of the Nucleo board
* all clocks are based from the internal oscillators, primarily the HSI clock
* where possible as it should give use the most stable clock from the options available
*
* This code is mostly copied from an MX generated project
*/
void SystemClock_Config(void){
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	*/
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_CSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.CSIState = RCC_CSI_ON;
	RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 32;
	RCC_OscInitStruct.PLL.PLLN = 250;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	//Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
								|RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
	//Error_Handler();
	}

	/** Configure the programming delay
	*/
	__HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);

	// Enable all GPIO clocks
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
}

/**
 * This function provides a delay in uS instead of ms
 * It should only be used for small delays <1ms
 */
void delayUS(uint32_t us){
    uint32_t startCount = SysTick->VAL;
    uint32_t targetCount = startCount - (us * SYSTICK_LOAD) + SYSTICK_DELAY_CALIB;

    // Handle wraparounds
    // If the targetCount is greater than the LOAD value, this means the previous subtraction
    // has underflowed
    if(targetCount > SysTick->LOAD){
      // Calculate a new targetCount taking the wraparound into account 
      targetCount = SysTick->LOAD - (us * SYSTICK_LOAD) + startCount + (SYSTICK_DELAY_CALIB);

      // Wait until the SysTick reloads
      while(SysTick->VAL < startCount); 
      startCount = SysTick->LOAD;
    }

    // Wait until we count down until the targetCount
    while((SysTick->VAL > targetCount) && (SysTick->VAL < startCount));
}
