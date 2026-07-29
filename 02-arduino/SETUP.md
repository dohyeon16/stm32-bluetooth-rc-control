# Arduino 설정 가이드

## 사용 보드 및 부품

- Arduino Uno
- HC-06 Bluetooth 모듈
- HC-SR04 초음파 센서
- L9110 모터 드라이버
- DC 모터 2개
- RC카 프레임
- 모터 구동용 배터리

## Arduino 핀 설정

| 기능 | Arduino 핀 |
|---|---:|
| HC-06 RX | D2 |
| HC-06 TX | D3 |
| 왼쪽 모터 입력 A | D5 |
| 왼쪽 모터 입력 B | D6 |
| HC-SR04 TRIG | D7 |
| HC-SR04 ECHO | D8 |
| 오른쪽 모터 입력 A | D9 |
| 오른쪽 모터 입력 B | D10 |

> 위 핀 번호는 복원 코드 기준입니다. 실제 프로젝트 배선과 다르면 `rc_car_control.ino`의 핀 번호를 수정해야 합니다.

## 통신 속도

| 연결 | 통신 속도 |
|---|---:|
| HC-06 ↔ Arduino | 9600bps |
| Arduino ↔ Windows PC | 115200bps |

## Bluetooth 제어 명령

| 명령 | RC카 동작 |
|---|---|
| `w` | 전진 |
| `s` | 후진 |
| `a` | 좌회전 |
| `d` | 우회전 |
| `x` | 정지 |

## Python 브릿지 전송 형식

Arduino는 Windows PC에 다음 형태로 명령을 전송합니다.

```text
CMD:w
CMD:s
CMD:a
CMD:d
CMD:x
CMD:B
```

초음파 거리값은 다음 형식으로 출력합니다.

```text
DIST:35.2
```

상태 로그는 다음 형식으로 출력합니다.

```text
INFO:RC_CAR_CONTROLLER_STARTED
INFO:OBSTACLE_DETECTED
INFO:OBSTACLE_CLEARED
WARN:DISTANCE_TIMEOUT
```

## 장애물 감지

- HC-SR04 측정 주기: 약 300ms
- 장애물 기준 거리: 20cm
- 장애물이 20cm 이내로 감지되면 RC카 자동 정지
- 장애물 감지 시 Python 브릿지에 `CMD:B` 전송
- STM32는 `B` 명령을 수신해 경고 상태 표시

## Arduino IDE 업로드 방법

1. Arduino Uno를 USB로 Windows PC에 연결
2. Arduino IDE 실행
3. `rc_car_control.ino` 열기
4. `Tools → Board → Arduino Uno` 선택
5. `Tools → Port`에서 Arduino COM 포트 선택
6. 코드의 핀 번호와 실제 배선 확인
7. 왼쪽 위 검증 버튼 클릭
8. 컴파일 오류가 없는지 확인
9. 업로드 버튼 클릭
10. Serial Monitor를 115200bps로 실행
11. `INFO:RC_CAR_CONTROLLER_STARTED` 출력 확인

## Bluetooth 테스트

1. HC-06 전원 연결
2. 스마트폰 Bluetooth 설정에서 HC-06 연결
3. Bluetooth Serial Controller 앱 실행
4. 버튼별 명령 설정

```text
전진: w
후진: s
좌회전: a
우회전: d
정지: x
```

5. 버튼을 눌러 RC카 동작 확인
6. Arduino Serial Monitor에서 `CMD:` 로그 확인

## 주의사항

- Arduino와 L9110 모터 드라이버의 GND를 공통으로 연결합니다.
- 모터 전원은 Arduino 5V 핀에서 직접 공급하지 않는 것이 좋습니다.
- 모터용 외부 전원을 사용할 때도 GND는 Arduino와 공통 연결합니다.
- HC-06 RX 핀 입력 전압을 확인하고 필요하면 전압 분배 회로를 사용합니다.
- 모터 방향이 반대로 움직이면 해당 모터의 두 입력 핀 또는 모터 배선을 반대로 연결합니다.
