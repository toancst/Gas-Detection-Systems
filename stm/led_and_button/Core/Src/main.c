/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  /* Reset của tất cả các ngoại vi, khởi tạo Flash interface và Systick. */
  HAL_Init();

  /* Cấu hình System Clock */
  SystemClock_Config();

  /* Khởi tạo các ngoại vi đã cấu hình */
  MX_GPIO_Init();

  /* Khai báo biến để lưu trạng thái nút bấm */
  // Trạng thái ban đầu của nút là thả ra (HIGH/SET)
  GPIO_PinState last_button1_state = GPIO_PIN_SET;
  GPIO_PinState last_button2_state = GPIO_PIN_SET;

  GPIO_PinState current_button1_state;
  GPIO_PinState current_button2_state;


  /* Vòng lặp vô hạn */
  while (1)
  {
    /* --- XỬ LÝ NÚT 1 (PB0) VÀ LED 1 (PA5) --- */

    // 1. Đọc trạng thái hiện tại của nút 1
    current_button1_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);

    // 2. Kiểm tra sự kiện nhấn nút (chuyển từ 1->0)
    // Nếu hiện tại đang nhấn (RESET) và trước đó đang thả (SET)
    if (current_button1_state == GPIO_PIN_RESET && last_button1_state == GPIO_PIN_SET)
    {
      // Đảo trạng thái của LED 1
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }

    // 3. Cập nhật trạng thái cũ của nút 1 cho lần lặp tiếp theo
    last_button1_state = current_button1_state;


    /* --- XỬ LÝ NÚT 2 (PB1) VÀ LED 2 (PA6) --- */

    // 1. Đọc trạng thái hiện tại của nút 2
    current_button2_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

    // 2. Kiểm tra sự kiện nhấn nút (chuyển từ 1->0)
    if (current_button2_state == GPIO_PIN_RESET && last_button2_state == GPIO_PIN_SET)
    {
      // Đảo trạng thái của LED 2
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    }

    // 3. Cập nhật trạng thái cũ của nút 2 cho lần lặp tiếp theo
    last_button2_state = current_button2_state;


    /* --- CHỐNG DỘI PHÍM (DEBOUNCE) --- */
    // Thêm một khoảng trễ nhỏ để CPU không đọc tín hiệu nhiễu khi nút đang rung
    HAL_Delay(20); // Delay 20 mili-giây
  }
}

/**
  * @brief Cấu hình các chân GPIO
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Bật clock cho các port GPIO */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Cấu hình chân LED mặc định là tắt */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /* Cấu hình các chân LED: PA5 và PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Chế độ Output Push-Pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;         // Không dùng trở kéo
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;// Tốc độ thấp là đủ cho LED
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Cấu hình các chân nút bấm: PB0 và PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // Chế độ Input
  GPIO_InitStruct.Pull = GPIO_PULLUP;         // Dùng điện trở kéo lên nội
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


/* Các hàm SystemClock_Config, Error_Handler... được tạo tự động bởi CubeIDE */
/* Bạn có thể để nguyên chúng */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

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
#endif
