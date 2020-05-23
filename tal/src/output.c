#include "stm32f4xx_hal.h"
#include "util.h"
#include "ws2812b/ws2812b.h"

static uint8_t state = 0;

TIM_HandleTypeDef TIM2_handle;

void TIM2_IRQHandler() {
    HAL_TIM_IRQHandler(&TIM2_handle);
}

void tim2_periodElapsed(TIM_HandleTypeDef *htim) {
    state ^= 1;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
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
    init_pin(GPIOD, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

//    uint32_t tim_period =  SystemCoreClock / 80000; // 1.25us

    __HAL_RCC_TIM2_CLK_ENABLE();
    __TIM2_CLK_ENABLE();
    TIM2_handle.Instance = TIM2;
    TIM2_handle.Init.Period            = 50;
    TIM2_handle.Init.RepetitionCounter = 0;
    TIM2_handle.Init.Prescaler         = 40000;
    TIM2_handle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    TIM2_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TIM2_handle);
    HAL_TIM_Base_Start(&TIM2_handle);
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->DIER |= 1;
}
