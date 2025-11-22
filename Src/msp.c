// msp.c

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

// Override the default implementations with our own

void HAL_MspInit(void)
{
    // Set up priority grouping for the arm cortex m processor
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // Enable the required system exceptions (of the arm cortex m processor)
    // Enable from the System Handler Control Status Register the bits 16, 17, 18
    // This corresponds to memory fault, bus fault, usage fault.
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_USGFAULTENA_Msk;

    // Configure the priority of the system exceptions
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    UNUSED(huart);

    // enable the clock for USART2
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Pin configuration - UART2 TX / RX config (RX is AF_PP also)
    GPIO_InitTypeDef g_uart;

    g_uart.Pin   = GPIO_PIN_2 | GPIO_PIN_3;
    g_uart.Mode  = GPIO_MODE_AF_PP;
    g_uart.Pull  = GPIO_PULLUP;
    g_uart.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g_uart);

    // enable the IRQ and set the IRQ priority
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    UNUSED(hcan);

    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // The RX pin is configured as a no-pull input
    GPIO_InitTypeDef g_can = {0};

    g_can.Pin  = GPIO_PIN_8;
    g_can.Mode = GPIO_MODE_INPUT;
    g_can.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &g_can);

    // TX is high speed alternate configuration
    g_can.Pin   = GPIO_PIN_9;
    g_can.Mode  = GPIO_MODE_AF_PP;
    g_can.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &g_can);

    __HAL_AFIO_REMAP_CAN1_2();

    // We don't need any IRQs yet
    // HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    // HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);

    // HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 15, 0);
    // HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 15, 0);
}
