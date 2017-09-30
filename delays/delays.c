//-----------------------------------------------------------------------------
// Original library taken from Easyrider83 (c) http://kazus.ru/forums/showpost.php?p=524970&postcount=14
//-----------------------------------------------------------------------------

#include "stm32f1xx_hal.h"	// Функция для обращения к HAL и получения текущего значения SystemCoreClock (универсализация)
#include "delays.h"

void delay_ms(uint32_t nTime)
{ 
	nTime = (CPU_CLOCK/K_Const)*nTime;
  	while(nTime != 0)
  	{nTime--;}
}

void delay_us(uint32_t nTime)
{ 
	nTime = ((CPU_CLOCK/K_Const)*nTime)/1000;
  	while(nTime != 0)
  	{nTime--;}
}


//-----------------------------------------------------------------------------
// Тест функции задержки: Калибровка константы K_Const под конкретный Микроконтроллер (со своим набором/длительностью инструкций)

// Выберите порт, который смотрите осциллографом или логическим анализатором (Внимание: вы должны сконфигурировать порты самостоятельно!)
#define TEST_PORT  GPIOD,GPIO_PIN_7

// Вставьте вызов этой функции в "main", после инициализации портов, до запуска RTOS
void delay_test(void)
{
  HAL_GPIO_TogglePin(TEST_PORT);
  delay_us(100);
  HAL_GPIO_TogglePin(TEST_PORT);
  delay_us(200);
  HAL_GPIO_TogglePin(TEST_PORT);
  delay_us(1000);
  HAL_GPIO_TogglePin(TEST_PORT);
  delay_us(1500);
  HAL_GPIO_TogglePin(TEST_PORT);
  
  while(1);
}

