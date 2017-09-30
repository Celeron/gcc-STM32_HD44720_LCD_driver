#ifndef HD44780_STM32F10X_H_
#define HD44780_STM32F10X_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "hd44780.h"

typedef struct
{
  GPIO_TypeDef *gpio;
  uint16_t pinmask;
} HD44780_STM32F10x_Pin;

typedef struct
{
  HD44780_STM32F10x_Pin pins[HD44780_PINS_AMOUNT];
} HD44780_STM32F10x_Pinout;

typedef struct
{
  HD44780_GPIO_Interface interface;
  HD44780_STM32F10x_Pinout pinout;
  HD44780_AssertFn assert_failure_handler;
} HD44780_STM32F10x_GPIO_Driver;

extern const HD44780_GPIO_Interface HD44780_STM32F10X_PINDRIVER_INTERFACE;

#ifdef __cplusplus
}
#endif

#endif /* HD44780_STM32F10X_H_ */
