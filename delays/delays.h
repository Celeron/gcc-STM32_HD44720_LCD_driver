// Функции простой блокирующей задержки (для точной выдержки)

#ifndef DELAYS_H
#define DELAYS_H

#define CPU_CLOCK		HAL_RCC_GetHCLKFreq()
#define K_Const			5040

void delay_ms(uint32_t nTime);
void delay_us(uint32_t nTime);
void delay_test(void);

#endif  // DELAYS_H
