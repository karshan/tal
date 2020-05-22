#include "stm32f4xx_hal.h"

void init_pin(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pull) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pull;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}
