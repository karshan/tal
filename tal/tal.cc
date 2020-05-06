#include <stm32f4xx_conf.h>
#include "stmlib/stmlib.h"
#include "tal/ws2812b/ws2812b.h"

uint8_t frameBuffer[3*64];

int main() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_12;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_14;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &gpio_init);


//    GPIO_WriteBit(GPIOD, GPIO_Pin_12, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_14, Bit_SET);

    int i;
    for (i = 0; i < 64; i++) {
        frameBuffer[3 * i] = 100;
        frameBuffer[3 * i + 1] = 100;
        frameBuffer[3 * i + 2] = 100;
    }
    ws2812b.item[0].frameBufferPointer = frameBuffer;
    ws2812b.item[0].frameBufferSize = sizeof(frameBuffer);
    ws2812b.item[0].channel = 0;
    ws2812b_init();
    i = 0;
    while (1)
    {
        if(ws2812b.transferComplete)
        {
            // Update your framebuffer here or swap buffers
            // Signal that buffer is changed and transfer new data
            ws2812b.startTransfer = 1;
            ws2812b_handle();
        }
    }
    return 0;
}
