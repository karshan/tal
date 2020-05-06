/*

  WS2812B CPU and memory efficient library

  Date: 28.9.2016

  Author: Martin Hubacek
  	  	  http://www.martinhubacek.cz
  	  	  @hubmartin

  Licence: MIT License

*/

#include <string.h>

#include "stm32f4xx_conf.h"
#include "ws2812b.h"

extern WS2812_Struct ws2812b;
static void ws2812b_set_pixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue);

// Define source arrays for my DMAs
uint32_t WS2812_IO_High[] =  { WS2812B_PINS };
uint32_t WS2812_IO_Low[] = {WS2812B_PINS << 16};

// WS2812 framebuffer - buffer for 2 LEDs - two times 24 bits
uint16_t ws2812bDmaBitBuffer[24 * 2];

// Gamma correction table
const uint8_t gammaTable[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

static void ws2812b_gpio_init(void)
{
	// WS2812B outputs
	WS2812B_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin   = WS2812B_PINS;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(WS2812B_PORT, &GPIO_InitStruct);

	// Enable output pins for debuging to see DMA Full and Half transfer interrupts
	#if defined(LED_BLUE_PORT) && defined(LED_ORANGE_PORT)
                RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

                GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
                GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
                GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
                GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;

		GPIO_InitStruct.GPIO_Pin = LED_BLUE_PIN;
		GPIO_Init(LED_BLUE_PORT, &GPIO_InitStruct);
		GPIO_InitStruct.GPIO_Pin = LED_ORANGE_PIN;
		GPIO_Init(LED_ORANGE_PORT, &GPIO_InitStruct);
	#endif
}

/* TIM_HandleTypeDef    TIM1_handle;
TIM_OC_InitTypeDef tim2OC1;
TIM_OC_InitTypeDef tim2OC2;
*/

uint32_t tim_period;
uint32_t timer_reset_pulse_period;

static void TIM1_init(void)
{
	// TIM2 Periph clock enable
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	// This computation of pulse length should work ok,
	// at some slower core speeds it needs some tuning.
	tim_period =  SystemCoreClock / 800000; // 0,125us period (10 times lower the 1,25us period to have fixed math below)
	timer_reset_pulse_period = (SystemCoreClock / (320 * 60)); // 60us just to be sure

	uint32_t cc1 = (10 * tim_period) / 36;
	uint32_t cc2 = (10 * tim_period) / 15;

        TIM_TimeBaseInitTypeDef timer_init;
        timer_init.TIM_Period = tim_period;
        timer_init.TIM_Prescaler = 0;
        timer_init.TIM_ClockDivision = TIM_CKD_DIV1;
        timer_init.TIM_CounterMode = TIM_CounterMode_Up;
        timer_init.TIM_RepetitionCounter = 0;
        TIM_InternalClockConfig(TIM1); // ??? what dis
        TIM_TimeBaseInit(TIM1, &timer_init);

	TIM_OCInitTypeDef output_compare_init;
        output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
        output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
        output_compare_init.TIM_OutputNState = TIM_OutputNState_Disable;
        output_compare_init.TIM_Pulse = cc1;
        output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
        output_compare_init.TIM_OCNPolarity = TIM_OCNPolarity_High;
        // XXX OCFastMode = Disable
        TIM_OC1Init(TIM1, &output_compare_init);

        output_compare_init.TIM_OCMode = TIM_OCMode_PWM1;
        output_compare_init.TIM_OutputState = TIM_OutputState_Enable;
        output_compare_init.TIM_OutputNState = TIM_OutputNState_Disable;
        output_compare_init.TIM_Pulse = cc2;
        output_compare_init.TIM_OCPolarity = TIM_OCPolarity_High;
        output_compare_init.TIM_OCNPolarity = TIM_OCNPolarity_High;
        output_compare_init.TIM_OCIdleState = TIM_OCIdleState_Reset;
        output_compare_init.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
        // XXX OCFastMode = Disable
        TIM_OC2Init(TIM1, &output_compare_init);

// XXX	HAL_TIM_Base_Start(&TIM1_handle);
        // why enable to disable immediately ? TIM_Cmd(TIM1, ENABLE);
// XXX	HAL_TIM_PWM_Start(&TIM1_handle, TIM_CHANNEL_1);
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
        TIM_CtrlPWMOutputs(TIM1, ENABLE);

        TIM_Cmd(TIM1, DISABLE);

}

#define BUFFER_SIZE		(sizeof(ws2812bDmaBitBuffer)/sizeof(uint16_t))

uint32_t dummy;

static void DMA2_init(void)
{
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

        DMA_InitTypeDef dma_init;
        dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
        dma_init.DMA_Mode = DMA_Mode_Circular;
        dma_init.DMA_Priority = DMA_Priority_High;
        dma_init.DMA_Channel = DMA_Channel_6;
        dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
        dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_DeInit(DMA2_Stream5);
        DMA_Init(DMA2_Stream5, &dma_init);
	// XXX HAL_DMA_Start(&dmaUpdate, (uint32_t)WS2812_IO_High, (uint32_t)(&WS2812B_PORT->BSRR), BUFFER_SIZE);
        DMA2_Stream5->CR &= (uint32_t)(~DMA_SxCR_DBM);
        DMA2_Stream5->NDTR = BUFFER_SIZE;
        DMA2_Stream5->PAR = (uint32_t)(&WS2812B_PORT->BSRRH);
        DMA2_Stream5->M0AR = (uint32_t)(WS2812_IO_High);
        DMA_Cmd(DMA2_Stream5, ENABLE);

        dma_init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
        dma_init.DMA_Mode = DMA_Mode_Circular;
        dma_init.DMA_Priority = DMA_Priority_High;
        dma_init.DMA_Channel = DMA_Channel_6;
        dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
        dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_DeInit(DMA2_Stream1);
        DMA_Init(DMA2_Stream1, &dma_init);
	// XXX HAL_DMA_Start(&dmaCC1, (uint32_t)ws2812bDmaBitBuffer, (uint32_t)(&WS2812B_PORT->BSRR) + 2, BUFFER_SIZE); //BRR
        DMA2_Stream1->CR &= (uint32_t)(~DMA_SxCR_DBM);
        DMA2_Stream1->NDTR = BUFFER_SIZE;
        DMA2_Stream1->PAR = (uint32_t)(&WS2812B_PORT->BSRRL);
        DMA2_Stream1->M0AR = (uint32_t)(WS2812_IO_High);
        DMA_Cmd(DMA2_Stream1, ENABLE);

        DMA_DeInit(DMA2_Stream2);
        DMA_Init(DMA2_Stream2, &dma_init);
	NVIC_SetPriority(DMA2_Stream2_IRQn, 0);
	NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	// XXX HAL_DMA_Start_IT(&dmaCC2, (uint32_t)WS2812_IO_Low, (uint32_t)&WS2812B_PORT->BSRR, BUFFER_SIZE);
        DMA2_Stream2->CR &= (uint32_t)(~DMA_SxCR_DBM);
        DMA2_Stream2->NDTR = BUFFER_SIZE;
        DMA2_Stream2->PAR = (uint32_t)(&WS2812B_PORT->BSRRL);
        DMA2_Stream2->M0AR = (uint32_t)(WS2812_IO_Low);
        DMA2->LIFCR = 0x3F << 16;
        DMA2_Stream2->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_HT;
        DMA2_Stream2->FCR |= DMA_IT_FE;
        DMA_Cmd(DMA2_Stream2, ENABLE);

}

static void loadNextFramebufferData(WS2812_BufferItem *bItem, uint32_t row)
{

	uint32_t r = bItem->frameBufferPointer[bItem->frameBufferCounter++];
	uint32_t g = bItem->frameBufferPointer[bItem->frameBufferCounter++];
	uint32_t b = bItem->frameBufferPointer[bItem->frameBufferCounter++];

	if(bItem->frameBufferCounter == bItem->frameBufferSize)
		bItem->frameBufferCounter = 0;

	ws2812b_set_pixel(bItem->channel, row, r, g, b);
}


// Transmit the framebuffer
static void WS2812_sendbuf()
{
	// transmission complete flag
	ws2812b.transferComplete = 0;

	uint32_t i;

	for( i = 0; i < WS2812_BUFFER_COUNT; i++ )
	{
		ws2812b.item[i].frameBufferCounter = 0;

		loadNextFramebufferData(&ws2812b.item[i], 0); // ROW 0
		loadNextFramebufferData(&ws2812b.item[i], 1); // ROW 0
	}

	// clear all DMA flags
        DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_TEIF1);
        DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_TEIF1);
        DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2 | DMA_FLAG_TEIF2);

	// configure the number of bytes to be transferred by the DMA controller
	DMA2_Stream5->NDTR = BUFFER_SIZE;
	DMA2_Stream1->NDTR = BUFFER_SIZE;
	DMA2_Stream2->NDTR = BUFFER_SIZE;

	// clear all TIM2 flags
        TIM_ClearFlag(TIM1, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

	// enable DMA channels
        DMA_Cmd(DMA2_Stream5, ENABLE);
        DMA_Cmd(DMA2_Stream1, ENABLE);
        DMA_Cmd(DMA2_Stream2, ENABLE);

	// IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
	TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
	TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
	TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);

	TIM1->CNT = tim_period-1;

	// start TIM2
	TIM_Cmd(TIM1, ENABLE);
}

void DMA_TransferHalfHandler()
{

	// Is this the last LED?
	if(ws2812b.repeatCounter == WS2812B_NUMBER_OF_LEDS)
	 {

		// If this is the last pixel, set the next pixel value to zeros, because
		// the DMA would not stop exactly at the last bit.
		ws2812b_set_pixel(0, 0, 0, 0, 0);

	} else {
		uint32_t i;

		for( i = 0; i < WS2812_BUFFER_COUNT; i++ )
		{
			loadNextFramebufferData(&ws2812b.item[i], 0);
		}

		ws2812b.repeatCounter++;
	}



}

void DMA_TransferCompleteHandler()
{

	#if defined(LED_ORANGE_PORT)
		LED_ORANGE_PORT->BSRRL = LED_ORANGE_PIN;
	#endif

	if(ws2812b.repeatCounter == WS2812B_NUMBER_OF_LEDS)
	{
		// Transfer of all LEDs is done, disable DMA but enable tiemr update IRQ to stop the 50us pulse
		ws2812b.repeatCounter = 0;

		// Stop timer
		TIM1->CR1 &= ~TIM_CR1_CEN;

		// Disable DMA
                DMA_Cmd(DMA2_Stream5, DISABLE);
                DMA_Cmd(DMA2_Stream1, DISABLE);
                DMA_Cmd(DMA2_Stream2, DISABLE);

		// Disable the DMA requests
                TIM_DMACmd(TIM1, TIM_DMA_Update, DISABLE);
                TIM_DMACmd(TIM1, TIM_DMA_CC1, DISABLE);
                TIM_DMACmd(TIM1, TIM_DMA_CC2, DISABLE);

		// Set 50us period for Treset pulse
		//TIM2->PSC = 1000; // For this long period we need prescaler 1000
		TIM1->ARR = timer_reset_pulse_period;
		// Reset the timer
		TIM1->CNT = 0;

		// Generate an update event to reload the prescaler value immediately
		TIM1->EGR = TIM_EGR_UG;
                TIM_ClearFlag(TIM1, TIM_FLAG_Update);

		// Enable TIM2 Update interrupt for 50us Treset signal
                TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
		// Enable timer
		TIM1->CR1 |= TIM_CR1_CEN;

		// Manually set outputs to low to generate 50us reset impulse
		WS2812B_PORT->BSRRH = WS2812B_PINS;
	} else {

		// Load bitbuffer with next RGB LED values
		uint32_t i;
		for( i = 0; i < WS2812_BUFFER_COUNT; i++ )
		{
			loadNextFramebufferData(&ws2812b.item[i], 1);
		}

		ws2812b.repeatCounter++;
	}



	#if defined(LED_ORANGE_PORT)
		LED_ORANGE_PORT->BSRRH = LED_ORANGE_PIN;
	#endif

}

void DMA2_Stream2_IRQHandler(void)
{

	#if defined(LED_BLUE_PORT)
		LED_BLUE_PORT->BSRRL = LED_BLUE_PIN;
	#endif

	// Check the interrupt and clear flag
	// XXX HAL_DMA_IRQHandler(&dmaCC2);
        uint32_t isr = DMA2->LISR;
        if (isr & DMA_FLAG_HTIF0) {
            if (DMA2_Stream2->CR & DMA_IT_HT) {
                // clear flag
                DMA2->LIFCR = DMA_FLAG_HTIF0 << 16; // ???? Same as ... _FLAG_HTIF2;
                DMA_TransferHalfHandler();
            }
        }

        if (isr & DMA_FLAG_TCIF0) {
            if (DMA2_Stream2->CR & DMA_IT_TC) {
                // clear flag
                DMA2->LIFCR = DMA_FLAG_TCIF0 << 16; // ???? Same as ... _FLAG_TCIF2;
	        DMA_TransferCompleteHandler();
            }
        }

	#if defined(LED_BLUE_PORT)
		LED_BLUE_PORT->BSRRH = LED_BLUE_PIN;
	#endif
}

// TODO TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled
void HAL_TIM_PeriodElapsedCallback()
{
	/*
	// I have to wait 50us to generate Treset signal
	if (ws2812b.timerPeriodCounter < (uint8_t)WS2812_RESET_PERIOD)
	{
		// count the number of timer periods
		ws2812b.timerPeriodCounter++;
	}
	else
	{
		ws2812b.timerPeriodCounter = 0;
		__HAL_TIM_DISABLE(&TIM1_handle);
		TIM1->CR1 = 0; // disable timer

		// disable the TIM2 Update
		__HAL_TIM_DISABLE_IT(&TIM1_handle, TIM_IT_UPDATE);
		// set TransferComplete flag
		ws2812b.transferComplete = 1;
	}*/

    ws2812b.timerPeriodCounter = 0;
    TIM1->CR1 = 0; // disable timer

    // disable the TIM2 Update IRQ
    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);

    // Set back 1,25us period
    TIM1->ARR = tim_period;

    // Generate an update event to reload the Prescaler value immediatly
    TIM1->EGR = TIM_EGR_UG;
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);

    // set transfer_complete flag
    ws2812b.transferComplete = 1;

}

void TIM1_UP_TIM10_IRQHandler(void)
{
	#if defined(LED_ORANGE_PORT)
		LED_ORANGE_PORT->BSRRL = LED_ORANGE_PIN;
	#endif

	// XXX HAL_TIM_IRQHandler(&TIM1_handle);
        if (TIM1->SR & TIM_FLAG_CC1) {
            if (TIM1->DIER & TIM_IT_CC1) {
                TIM1->SR = ~(TIM_IT_CC1);
                /* TODO
                HAL_TIM_OC_DelayElapsedCallback(htim);
                HAL_TIM_PWM_PulseFinishedCallback(htim);
                */
            }
        }

        if (TIM1->SR & TIM_FLAG_CC2) {
            if (TIM1->DIER & TIM_IT_CC2) {
                TIM1->SR = ~(TIM_IT_CC2);
                /* TODO
                HAL_TIM_OC_DelayElapsedCallback(htim);
                HAL_TIM_PWM_PulseFinishedCallback(htim);
                */
            }
        }

        if (TIM1->SR & TIM_FLAG_Update) {
            if (TIM1->DIER & TIM_IT_Update) {
                TIM1->SR = ~(TIM_IT_Update);
                /* TODO
                HAL_TIM_OC_DelayElapsedCallback(htim);
                HAL_TIM_PWM_PulseFinishedCallback(htim);
                */
                HAL_TIM_PeriodElapsedCallback();
            }
        }


	#if defined(LED_ORANGE_PORT)
		LED_ORANGE_PORT->BSRRH = LED_ORANGE_PIN;
	#endif
}

static void ws2812b_set_pixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue)
{
	// Apply gamma
	red = gammaTable[red];
	green = gammaTable[green];
	blue = gammaTable[blue];


	uint32_t calcCol = (column*24);
	uint32_t invRed = ~red;
	uint32_t invGreen = ~green;
	uint32_t invBlue = ~blue;

	// Bitband optimizations with pure increments, 5us interrupts
	volatile uint32_t *bitBand = BITBAND_SRAM(&ws2812bDmaBitBuffer[(calcCol)], row);

	*bitBand =  (invGreen >> 7);
	bitBand+=16;

	*bitBand = (invGreen >> 6);
	bitBand+=16;

	*bitBand = (invGreen >> 5);
	bitBand+=16;

	*bitBand = (invGreen >> 4);
	bitBand+=16;

	*bitBand = (invGreen >> 3);
	bitBand+=16;

	*bitBand = (invGreen >> 2);
	bitBand+=16;

	*bitBand = (invGreen >> 1);
	bitBand+=16;

	*bitBand = (invGreen >> 0);
	bitBand+=16;

	// RED
	*bitBand =  (invRed >> 7);
	bitBand+=16;

	*bitBand = (invRed >> 6);
	bitBand+=16;

	*bitBand = (invRed >> 5);
	bitBand+=16;

	*bitBand = (invRed >> 4);
	bitBand+=16;

	*bitBand = (invRed >> 3);
	bitBand+=16;

	*bitBand = (invRed >> 2);
	bitBand+=16;

	*bitBand = (invRed >> 1);
	bitBand+=16;

	*bitBand = (invRed >> 0);
	bitBand+=16;

	// BLUE
	*bitBand =  (invBlue >> 7);
	bitBand+=16;

	*bitBand = (invBlue >> 6);
	bitBand+=16;

	*bitBand = (invBlue >> 5);
	bitBand+=16;

	*bitBand = (invBlue >> 4);
	bitBand+=16;

	*bitBand = (invBlue >> 3);
	bitBand+=16;

	*bitBand = (invBlue >> 2);
	bitBand+=16;

	*bitBand = (invBlue >> 1);
	bitBand+=16;

	*bitBand = (invBlue >> 0);
	bitBand+=16;
}


void ws2812b_init()
{
	ws2812b_gpio_init();

	/*TIM2_init();
	DMA_init();*/


	DMA2_init();
	TIM1_init();


	// Need to start the first transfer
	ws2812b.transferComplete = 1;
}


void ws2812b_handle()
{
	if(ws2812b.startTransfer) {
		ws2812b.startTransfer = 0;
		WS2812_sendbuf();
	}

}
