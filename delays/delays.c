//-----------------------------------------------------------------------------
// Original library taken from Easyrider83 (c) http://kazus.ru/forums/showpost.php?p=524970&postcount=14
//-----------------------------------------------------------------------------

#include "stm32f1xx_hal.h"	// ������� ��� ��������� � HAL � ��������� �������� �������� SystemCoreClock (���������������)
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
// ���� ������� ��������: ���������� ��������� K_Const ��� ���������� ��������������� (�� ����� �������/������������� ����������)

// �������� ����, ������� �������� ������������� ��� ���������� ������������ (��������: �� ������ ���������������� ����� ��������������!)
#define TEST_PORT  GPIOD,GPIO_PIN_7

// �������� ����� ���� ������� � "main", ����� ������������� ������, �� ������� RTOS
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

