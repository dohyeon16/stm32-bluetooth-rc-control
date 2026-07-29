#include "rc_status.h"

static UART_HandleTypeDef *rc_uart = NULL;
static uint8_t rx_byte = 0;

#define LED_PORT        GPIOD
#define LED_FORWARD     GPIO_PIN_0
#define LED_BACKWARD    GPIO_PIN_1
#define LED_LEFT        GPIO_PIN_2
#define LED_RIGHT       GPIO_PIN_3

static void RC_Status_AllOff(void)
{
    HAL_GPIO_WritePin(
        LED_PORT,
        LED_FORWARD | LED_BACKWARD | LED_LEFT | LED_RIGHT,
        GPIO_PIN_RESET
    );
}

static void RC_Status_HandleCommand(uint8_t command)
{
    RC_Status_AllOff();

    switch (command)
    {
        case 'w':
            HAL_GPIO_WritePin(LED_PORT, LED_FORWARD, GPIO_PIN_SET);
            break;

        case 's':
            HAL_GPIO_WritePin(LED_PORT, LED_BACKWARD, GPIO_PIN_SET);
            break;

        case 'a':
            HAL_GPIO_WritePin(LED_PORT, LED_LEFT, GPIO_PIN_SET);
            break;

        case 'd':
            HAL_GPIO_WritePin(LED_PORT, LED_RIGHT, GPIO_PIN_SET);
            break;

        case 'x':
            /* 정지 상태: 모든 LED 소등 */
            break;

        case 'B':
            /* 장애물 경고: 모든 LED 점등 */
            HAL_GPIO_WritePin(
                LED_PORT,
                LED_FORWARD | LED_BACKWARD | LED_LEFT | LED_RIGHT,
                GPIO_PIN_SET
            );
            break;

        default:
            /* 알 수 없는 명령은 무시 */
            break;
    }
}

void RC_Status_Init(UART_HandleTypeDef *huart)
{
    rc_uart = huart;
    RC_Status_AllOff();

    HAL_UART_Receive_IT(rc_uart, &rx_byte, 1);
}

void RC_Status_OnUartRxComplete(UART_HandleTypeDef *huart)
{
    if (rc_uart == NULL || huart->Instance != rc_uart->Instance)
    {
        return;
    }

    RC_Status_HandleCommand(rx_byte);

    /* 다음 1바이트 수신 재활성화 */
    HAL_UART_Receive_IT(rc_uart, &rx_byte, 1);
}
