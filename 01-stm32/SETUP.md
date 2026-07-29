# STM32 설정 가이드

## 사용 보드 및 개발 환경

- STM32F429ZIT6
- STM32CubeIDE
- STM32CubeMX
- C 언어

## USART 설정

STM32CubeMX 또는 STM32CubeIDE의 `.ioc` 설정 화면에서 USART3를 다음과 같이 설정합니다.

- USART: USART3
- Mode: Asynchronous
- Baud Rate: 9600bps
- Word Length: 8 Bits
- Parity: None
- Stop Bits: 1
- Hardware Flow Control: None
- USART3 Global Interrupt: Enable

## USART 핀 설정

- PD8: USART3_TX
- PD9: USART3_RX

## LED GPIO 설정

다음 핀을 GPIO Output으로 설정합니다.

- PD0: 전진 상태
- PD1: 후진 상태
- PD2: 좌회전 상태
- PD3: 우회전 상태
- 장애물 명령 `B`: 전체 LED 점등

## rc_status 파일 추가

STM32CubeIDE 프로젝트에 다음 파일을 추가합니다.

```text
Core/
├── Inc/
│   └── rc_status.h
└── Src/
    └── rc_status.c
```

- `rc_status.h`: UART 상태 제어 함수 선언
- `rc_status.c`: UART 명령 처리 및 LED 제어 구현

## main.c 적용 방법

### 1. 헤더 파일 추가

STM32CubeIDE 프로젝트의 `Core/Src/main.c`를 열고 다음 위치를 찾습니다.

```c
/* USER CODE BEGIN Includes */
```

그 아래에 다음 코드를 추가합니다.

```c
#include "rc_status.h"
```

최종 형태는 다음과 같습니다.

```c
/* USER CODE BEGIN Includes */
#include "rc_status.h"
/* USER CODE END Includes */
```

### 2. UART 수신 초기화 추가

`main.c`에서 다음 초기화 코드가 실행되는 부분을 찾습니다.

```c
MX_GPIO_Init();
MX_USART3_UART_Init();
```

그 아래에 다음 코드를 추가합니다.

```c
RC_Status_Init(&huart3);
```

최종 형태는 다음과 같습니다.

```c
MX_GPIO_Init();
MX_USART3_UART_Init();

/* USER CODE BEGIN 2 */
RC_Status_Init(&huart3);
/* USER CODE END 2 */
```

`MX_GPIO_Init()`과 `MX_USART3_UART_Init()`은 STM32CubeMX가 자동으로 생성하므로 직접 중복 작성하지 않습니다.

### 3. UART 수신 콜백 추가

`main.c`의 아래쪽에서 다음 영역을 찾습니다.

```c
/* USER CODE BEGIN 4 */
```

그 안에 다음 코드를 추가합니다.

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    RC_Status_OnUartRxComplete(huart);
}
```

최종 형태는 다음과 같습니다.

```c
/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    RC_Status_OnUartRxComplete(huart);
}

/* USER CODE END 4 */
```

## 명령별 LED 동작

| 수신 명령 | RC카 상태 | STM32 LED 동작 |
|---|---|---|
| `w` | 전진 | PD0 점등 |
| `s` | 후진 | PD1 점등 |
| `a` | 좌회전 | PD2 점등 |
| `d` | 우회전 | PD3 점등 |
| `x` | 정지 | 전체 LED 소등 |
| `B` | 장애물 감지 | 전체 LED 점등 |

## 주의사항

- Arduino와 STM32의 통신 속도가 일치해야 합니다.
- STM32 USART3 수신 설정은 인터럽트 방식으로 구성합니다.
- `HAL_UART_Receive_IT()`는 수신 완료 후 다시 호출해야 다음 명령을 받을 수 있습니다.
- 실제 GPIO 핀은 프로젝트 당시 사용한 배선과 다를 경우 수정해야 합니다.
- 이 문서는 설정 방법을 설명하는 문서이며 실제 실행 코드는 `rc_status.h`, `rc_status.c`, `main.c`에 적용합니다.
