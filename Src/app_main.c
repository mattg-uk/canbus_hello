// main.c
//

// Right now, main.h isn't doing very much
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_uart.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void Error_Handler(void);
void clock_config(void);
void present_clocks(void);

// Our peripheral initialization functions
void CAN1_init(void);
void UART2_Init(void);
void LED_init(void);

// CAN Functions
void CAN1_TX(void);

// Our Handles
UART_HandleTypeDef huart2;
CAN_HandleTypeDef  hcan1;

int main(void)
{
    HAL_Init();
    clock_config();
    UART2_Init();
    LED_init();
    CAN1_init();

    present_clocks();

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }

    char     msg[64];
    uint32_t alive = 0;

    while (true) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        alive += 1;
        sprintf(msg, "Alive: %ld\r\n\n", (long)alive);
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        CAN1_TX();
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(1000);
    }
}

void UART2_Init(void)
{
    huart2.Instance = USART2;

    // The init type member is of UART_InitTypeDef
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

void CAN1_init(void)
{
    CAN_InitTypeDef config = {
        .Prescaler            = 9,
        .Mode                 = CAN_MODE_NORMAL,
        .SyncJumpWidth        = CAN_SJW_1TQ,
        .TimeSeg1             = CAN_BS1_2TQ,
        .TimeSeg2             = CAN_BS2_1TQ,
        .AutoBusOff           = DISABLE,
        .AutoRetransmission   = ENABLE,
        .AutoWakeUp           = DISABLE,
        .ReceiveFifoLocked    = DISABLE,
        .TransmitFifoPriority = DISABLE,
    };

    hcan1.Instance = CAN1;
    hcan1.Init     = config;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    };
}

void clock_config(void)
{
    // Configure oscillator - HSI is DIV2 on this platform - and configure the
    // PLL. F1 PLL only has one multiplier, whereas F4 has M divisor, N multiplier
    // and P divisor. MUL9 gives 72MHz. PLLMUL only goes up to 16.
    RCC_OscInitTypeDef oscillator_config = {.OscillatorType = RCC_OSCILLATORTYPE_HSE,
                                            .HSEState       = RCC_HSE_BYPASS,
                                            .PLL            = {.PLLState  = RCC_PLL_ON,
                                                               .PLLMUL    = RCC_PLL_MUL9,
                                                               .PLLSource = RCC_PLLSOURCE_HSE}};
    if (HAL_RCC_OscConfig(&oscillator_config) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitTypeDef clock_params = {
        .ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                     RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider  = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV2,
        .APB2CLKDivider = RCC_HCLK_DIV1,
    };

    // Above 60MHz two wait states are required
    if (HAL_RCC_ClockConfig(&clock_params, FLASH_ACR_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }

    // We could disable the HSI, but we won't

    // Ticks per ms
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);

    // Actually use the HCLK for systick
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void CAN1_TX(void)
{
    // SO NOW CREATE A MESSAGE
    CAN_TxHeaderTypeDef header = {.DLC                = 5,
                                  .ExtId              = 200,
                                  .IDE                = CAN_ID_EXT,
                                  .RTR                = CAN_RTR_DATA,
                                  .TransmitGlobalTime = DISABLE};

    uint8_t data[] = "hello";

    // Got the header, data, and the chandle, just need a mailbox
    // Turns out that its populated by out variable
    uint32_t mailbox;
    if (HAL_CAN_AddTxMessage(&hcan1, &header, data, &mailbox) != HAL_OK) {
        Error_Handler();
    }

    // Let's wait for the message to be sent
    while (HAL_CAN_IsTxMessagePending(&hcan1, mailbox)) {
        ;
    }
}

void LED_init(void)
{

    GPIO_InitTypeDef g_uart;

    g_uart.Pin   = GPIO_PIN_5;
    g_uart.Mode  = GPIO_MODE_OUTPUT_PP;
    g_uart.Pull  = GPIO_NOPULL;
    g_uart.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g_uart);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void Error_Handler(void)
{
    while (1)
        ;
}

void present_clocks(void)
{
    char msg[64];
    sprintf(msg, "SYSCLK: %ld\r\n", (long)HAL_RCC_GetSysClockFreq());
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "HCLK: %ld\r\n", (long)HAL_RCC_GetHCLKFreq());
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "APB1: %ld\r\n", (long)HAL_RCC_GetPCLK1Freq());
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "APB2: %ld\r\n\n", (long)HAL_RCC_GetPCLK2Freq());
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}
