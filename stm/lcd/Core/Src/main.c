/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Chương trình ví dụ điều khiển LCD I2C 16x2 trên STM32F401CCU6.
  * Hiển thị chuỗi "Hello, World!" và một bộ đếm.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c_lcd.h" // Thêm thư viện LCD của bạn
#include <stdio.h>
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

// Khai báo biến handle cho LCD
I2C_LCD_HandleTypeDef lcd;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init(); // Khởi tạo I2C

  /* --- BẮT ĐẦU CODE CHO LCD --- */

  // 1. Cấu hình handle cho LCD
  lcd.hi2c = &hi2c1;             // Trỏ tới handle I2C1 đã được khởi tạo
  lcd.address = (0x27 << 1);     // Địa chỉ của module LCD I2C.
                                 // Phổ biến là 0x27 hoặc 0x3F.
                                 // Phải dịch trái 1 bit cho thư viện HAL.

  // 2. Khởi tạo LCD
  lcd_init(&lcd);

  // 3. Xóa màn hình
  lcd_clear(&lcd);

  // 4. Di chuyển con trỏ đến vị trí (cột 0, hàng 0)
  lcd_gotoxy(&lcd, 0, 0);

  // 5. Gửi chuỗi "hello" tới LCD
  lcd_puts(&lcd, "hello");

  // Ví dụ thêm: Hiển thị một chuỗi khác ở hàng thứ hai
  lcd_gotoxy(&lcd, 0, 1);
  lcd_puts(&lcd, "STM32F401CCU6");

  /* --- KẾT THÚC CODE CHO LCD --- */


  /* Infinite loop */
  while (1)
  {
      // Vòng lặp chính có thể dùng để cập nhật màn hình hoặc làm việc khác.
      // Ví dụ: làm một bộ đếm đơn giản
      static uint8_t counter = 0;
      char buffer[16];

      // Di chuyển con trỏ tới cuối hàng đầu tiên
      lcd_gotoxy(&lcd, 10, 0);

      // Định dạng chuỗi và hiển thị bộ đếm
      sprintf(buffer, "C:%03d", counter++);
      lcd_puts(&lcd, buffer);

      HAL_Delay(500); // Chờ 0.5 giây
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000; // 100kHz
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
