#include "stm32f4xx_hal.h"
#include "util.h"
#include "ui.h"
#include "ws2812b/ws2812b.h"

TIM_HandleTypeDef TIM2_handle;

void TIM2_IRQHandler() {
    HAL_TIM_IRQHandler(&TIM2_handle);
}

void tim2_periodElapsed(TIM_HandleTypeDef *htim) {
    GPIO_PinState cur = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0);

    if (pause == 0) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_0);
    }

    if (cur == GPIO_PIN_RESET) {
        ui_tick();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        ws2812b_PeriodElapsedCallback(htim);
    } else {
        tim2_periodElapsed(htim);
    }
}

void output_init() {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    // FIXME use pins from ui.c ??? one source of truth
    uint16_t output_pins = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9;
    init_pin(GPIOD, output_pins, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    __HAL_RCC_TIM2_CLK_ENABLE();
    __TIM2_CLK_ENABLE();
    TIM2_handle.Instance = TIM2;
    TIM2_handle.Init.Period            = 50;
    TIM2_handle.Init.RepetitionCounter = 0;
    TIM2_handle.Init.Prescaler         = 10000;
    TIM2_handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    TIM2_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TIM2_handle);
    HAL_TIM_Base_Start(&TIM2_handle);
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET);

    TIM2->DIER |= 1;
}
