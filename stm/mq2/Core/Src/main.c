#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char txBuf[200];	// buffer for serial communication
uint8_t adc_valid = 0;

/* ADC Samples array */
uint32_t NUM_SAMPLES = 0;
uint32_t i = 0;
uint32_t ADC_SAMPLES[1000];

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	adc_valid = 1;
	if(i < NUM_SAMPLES){
		ADC_SAMPLES[i] = HAL_ADC_GetValue(&hadc1);
		i++;
		HAL_ADC_Start_IT(&hadc1);
	}
	else{
		i = 0;	// reinitialize iterator
		HAL_ADC_Stop_IT(&hadc1);
	}
}

/* MQ2 Sensor Specific Functions */
void MQ2_create(MQ2 *sensor) {
    sensor->LPGCurve[0] = 2.3;
    sensor->LPGCurve[1] = 0.21;
    sensor->LPGCurve[2] = -0.47;
    sensor->COCurve[0] = 2.3;
    sensor->COCurve[1] = 0.72;
    sensor->COCurve[2] = -0.34;
    sensor->SmokeCurve[0] = 2.3;
    sensor->SmokeCurve[1] = 0.53;
    sensor->SmokeCurve[2] = -0.44;
    sensor->Ro = -1.0;
    sensor->lastReadTime = 0;
}

void MQ2_begin(MQ2 *sensor) {
    // Initialization code here
	sensor->Ro = MQ2_MQCalibration(sensor);

	sprintf(txBuf, "Ro: %f kohm\r\n\r\n", sensor->Ro);
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
	HAL_Delay(50);
}

void MQ2_close(MQ2 *sensor) {
	sensor->Ro = -1.0;
	sensor->values[0] = 0.0;
	sensor->values[1] = 0.0;
	sensor->values[2] = 0.0;
}

bool MQ2_checkCalibration(MQ2 *sensor) {
	if (sensor->Ro < 0.0) {
		sprintf(txBuf, "%s ", "Device not calibrated, call MQ2_begin before reading any value.");
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
		return false;
	}
	return true;
}

float* MQ2_read(MQ2 *sensor, bool print) {
    // Reading code here
	if (!MQ2_checkCalibration(sensor)) return NULL;

	sensor->values[0] = MQ2_MQGetPercentage(sensor->LPGCurve, sensor);
	sensor->values[1] = MQ2_MQGetPercentage(sensor->COCurve, sensor);
	sensor->values[2] = MQ2_MQGetPercentage(sensor->SmokeCurve, sensor);

	sensor->lastReadTime = HAL_GetTick();

	if (print){
		sprintf(txBuf, "%dms\r\n", sensor->lastReadTime);
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
		HAL_Delay(50);

		sprintf(txBuf, "LPG: %f ppm\r\n", sensor->values[0]);
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
		HAL_Delay(50);

		sprintf(txBuf, "CO: %f ppm \r\n", sensor->values[1]);
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
		HAL_Delay(50);

		sprintf(txBuf, "SMOKE: %f ppm\r\n", sensor->values[2]);
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
		HAL_Delay(50);
	}

	return sensor->values;
}

float MQ2_readLPG(MQ2 *sensor) {

	if (!MQ2_checkCalibration(sensor)) return 0.0;

	if (HAL_GetTick() < (sensor->lastReadTime + READ_DELAY) && sensor->values[0] > 0)
		return sensor->values[0];
	else
		return (sensor->values[0] = MQ2_MQGetPercentage(sensor->LPGCurve, sensor));
}

float MQ2_readCO(MQ2 *sensor) {
	if (!MQ2_checkCalibration(sensor)) return 0.0;

	if (HAL_GetTick() < (sensor->lastReadTime + READ_DELAY) && sensor->values[1] > 0)
		return sensor->values[1];
	else
		return (sensor->values[1] = MQ2_MQGetPercentage(sensor->COCurve, sensor));
}

float MQ2_readSmoke(MQ2 *sensor) {
	if (!MQ2_checkCalibration(sensor)) return 0.0;

	if (HAL_GetTick() < (sensor->lastReadTime + READ_DELAY) && sensor->values[2] > 0)
		return sensor->values[2];
	else
		return (sensor->values[2] = MQ2_MQGetPercentage(sensor->SmokeCurve, sensor));
}

float MQ2_MQResistanceCalculation(int raw_adc) {
    // MQResistanceCalculation code here
	float flt_adc = (float) raw_adc;
	return RL_VALUE * (1023.0 - flt_adc) / flt_adc;
}

float MQ2_MQCalibration(MQ2 *sensor) {
    // MQCalibration code here
	float val = 0.0;

	// take multiple samples
	NUM_SAMPLES = CALIBARAION_SAMPLE_TIMES;
	HAL_ADC_Start_IT(&hadc1);
	HAL_Delay(CALIBRATION_SAMPLE_INTERVAL);

	for (int i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {
		val += MQ2_MQResistanceCalculation(ADC_SAMPLES[i]);
	}

	//calculate the average value
	val = val / ((float) CALIBARAION_SAMPLE_TIMES);

	//divided by RO_CLEAN_AIR_FACTOR yields the Ro according to the chart in the datasheet
	val = val / RO_CLEAN_AIR_FACTOR;

	return val;
}

float MQ2_MQRead(MQ2 *sensor) {
	float rs = 0.0;

	// take multiple samples
	NUM_SAMPLES = READ_SAMPLE_TIMES;
	HAL_ADC_Start_IT(&hadc1);
	HAL_Delay(READ_SAMPLE_INTERVAL);

	for (int i = 0; i < READ_SAMPLE_TIMES; i++) {
		rs += MQ2_MQResistanceCalculation(ADC_SAMPLES[i]);
	}

	return rs / ((float) READ_SAMPLE_TIMES);  // return the average
}

float MQ2_MQGetPercentage(float *pcurve, MQ2 *sensor) {
	float rs_ro_ratio = MQ2_MQRead(sensor) / sensor->Ro;
	return pow(10.0, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Setup
  MQ2 sensor;
  MQ2_create(&sensor);
  MQ2_begin(&sensor);
  float lpg, co, smoke;
  while (1)
  {
	  MQ2_read(&sensor, true); //set it false if you don't want to print the values to the Serial

	  // lpg = values[0];
	  lpg = MQ2_readLPG(&sensor);
	  // co = values[1];
	  co = MQ2_readCO(&sensor);
	  // smoke = values[2];
	  smoke = MQ2_readSmoke(&sensor);

	  sprintf(txBuf, "lpg: %f\r\n", lpg);
	  HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
	  HAL_Delay(50);

	  sprintf(txBuf, "co: %f\r\n", co);
	  HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
	  HAL_Delay(50);

	  sprintf(txBuf, "smoke: %f\r\n\r\n", smoke);
	  HAL_UART_Transmit_IT(&huart2, (uint8_t *)txBuf, strlen(txBuf));
	  HAL_Delay(50);

	  HAL_Delay(500);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
