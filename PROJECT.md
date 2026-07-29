# 프로젝트 상세 설명

## 1. 프로젝트 개요

스마트폰에서 전송한 Bluetooth 명령으로 RC카를 제어하고, Arduino와 STM32를 UART 및 TCP/IP 통신으로 연동한 임베디드 통신·제어 프로젝트

Arduino는 Bluetooth 명령에 따라 RC카의 DC 모터를 제어하고, 초음파 센서를 이용해 전방 장애물을 감지합니다.

주행 명령과 장애물 상태는 Windows PC에서 실행되는 Python 브릿지 프로그램을 통해 전달되며, STM32는 수신한 명령에 따라 LED로 현재 주행 상태를 표시합니다.

---

## 2. 주요 기능

- 스마트폰 Bluetooth 기반 RC카 원격 제어
- 전진, 후진, 좌회전, 우회전 및 정지 명령 처리
- Arduino 기반 DC 모터 제어
- HC-SR04 초음파 센서를 이용한 거리 측정
- 장애물 20cm 이내 감지 시 RC카 자동 정지
- UART 기반 장치 간 명령 전달
- Python 브릿지를 이용한 통신 데이터 중계
- IP 주소와 포트를 이용한 TCP/IP 통신
- STM32 LED를 이용한 주행 상태 및 경고 표시

---

## 3. 시스템 구성

```text
[스마트폰 애플리케이션]
        │
        │ Bluetooth
        ▼
[HC-06 Bluetooth 모듈]
        │
        │ UART 9600bps
        ▼
[Arduino Uno]
        ├─ 주행 명령 처리
        ├─ L9110 모터 드라이버 제어
        ├─ DC 모터 2개 구동
        ├─ HC-SR04 초음파 거리 측정
        └─ 장애물 감지 시 자동 정지
        │
        │ UART Serial
        ▼
[Windows PC]
        ├─ Python 브릿지 프로그램 실행
        ├─ Arduino 명령 수신
        ├─ 통신 데이터 로그 출력
        └─ IP 주소 및 포트 기반 TCP/IP 데이터 중계
        │
        │ TCP/IP 및 UART
        ▼
[STM32F429ZIT6]
        ├─ USART 기반 명령 수신
        ├─ 주행 상태 LED 점등
        └─ 장애물 경고 상태 표시
```

> Windows Command Prompt는 Python 브릿지 프로그램을 실행한 환경이며, 통신 방식 자체는 UART Serial과 TCP/IP입니다.

---

## 4. 통신 구조

### 스마트폰 → HC-06

- 통신 방식: Bluetooth Serial
- 사용 애플리케이션: Bluetooth Serial Controller
- 전송 명령: `w`, `s`, `a`, `d`, `x`

### HC-06 → Arduino Uno

- 통신 방식: UART
- 통신 속도: 9600bps
- 역할: 스마트폰의 RC카 제어 명령 전달

### Arduino Uno → Windows PC

- 통신 방식: UART Serial
- 연결 방식: USB 가상 COM 포트
- 역할:
  - RC카 주행 명령 전송
  - 장애물 감지 상태 전송
  - 디버깅 데이터 출력

### Windows PC의 Python 브릿지

- UART 데이터 수신
- 수신 명령 로그 출력
- IP 주소와 포트 번호를 이용한 TCP/IP 통신
- 수신 데이터를 STM32 연동 프로그램으로 중계

### Windows PC → STM32

- 통신 방식: UART Serial
- 연결 방식: USB 가상 COM 포트
- STM32 통신 장치: USART
- 역할: 주행 및 장애물 상태 명령 전달

---

## 5. 제어 명령

| 명령 | RC카 동작 | STM32 상태 표시 |
|---|---|---|
| `w` | 전진 | 전진 LED 점등 |
| `s` | 후진 | 후진 LED 점등 |
| `a` | 좌회전 | 좌회전 LED 점등 |
| `d` | 우회전 | 우회전 LED 점등 |
| `x` | 정지 | 주행 LED 소등 |
| `B` | 장애물 감지 및 자동 정지 | 장애물 경고 표시 |

---

## 6. 장애물 감지

HC-SR04 초음파 센서가 RC카 전방의 거리를 주기적으로 측정합니다.

장애물이 20cm 이내로 감지되면 다음 순서로 동작합니다.

1. Arduino가 DC 모터 정지
2. 장애물 경고 명령 `B` 생성
3. Python 브릿지로 `B` 명령 전달
4. TCP/IP 및 UART 통신을 통해 STM32로 명령 전달
5. STM32에서 장애물 경고 상태 표시

---

## 7. 하드웨어 구성

| 구성 요소 | 역할 |
|---|---|
| Arduino Uno | Bluetooth 명령 처리, 모터 및 센서 제어 |
| STM32F429ZIT6 | 명령 수신 및 LED 상태 표시 |
| HC-06 | 스마트폰 Bluetooth 통신 |
| HC-SR04 | 전방 장애물 거리 측정 |
| L9110 모터 드라이버 | DC 모터 구동 |
| DC 모터 2개 | RC카 좌우 바퀴 구동 |
| Windows PC | Python 브릿지 및 STM32 프로그램 실행 |
| 배터리 | RC카와 모터 전원 공급 |

---

## 8. 개발 환경

- STM32CubeIDE
- STM32CubeMX
- Arduino IDE
- Python
- Windows Command Prompt
- Bluetooth Serial Controller
- Serial Monitor

---

## 9. 전체 동작 순서

1. 스마트폰과 HC-06 Bluetooth 모듈 연결
2. 스마트폰에서 RC카 주행 명령 전송
3. Arduino가 Bluetooth 명령 수신
4. Arduino가 L9110 모터 드라이버 제어
5. DC 모터를 이용해 RC카 주행
6. HC-SR04로 전방 장애물 거리 측정
7. Arduino가 명령과 상태를 Windows PC로 전송
8. Python 브릿지가 UART 데이터를 수신
9. Python 브릿지가 IP 주소와 포트를 이용해 데이터 중계
10. STM32가 UART 명령 수신
11. STM32 LED로 현재 주행 상태 표시
12. 장애물 감지 시 RC카 자동 정지 및 경고 상태 표시

---

## 10. 프로젝트 특징

- Bluetooth, UART 및 TCP/IP를 결합한 통신 구조
- Arduino와 STM32를 연동한 멀티 MCU 시스템
- 스마트폰 명령과 실제 DC 모터 제어 연동
- Python 브릿지를 이용한 데이터 중계
- 초음파 센서 기반 장애물 자동 정지
- STM32 LED 기반 실시간 상태 모니터링
