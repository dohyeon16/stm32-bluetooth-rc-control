#include <SoftwareSerial.h>

/*
 * STM32 Bluetooth RC Control
 *
 * 주요 기능
 * 1. HC-06 Bluetooth 명령 수신
 * 2. L9110 모터 드라이버를 이용한 RC카 제어
 * 3. HC-SR04 초음파 센서 거리 측정
 * 4. 장애물 20cm 이내 감지 시 자동 정지
 * 5. Windows PC의 Python 브릿지로 명령 전달
 *
 * 주의
 * 아래 핀 번호는 예시입니다.
 * 실제 프로젝트 배선과 다르면 핀 번호를 수정해야 합니다.
 */

/* =========================================================
 * HC-06 Bluetooth 설정
 * =========================================================
 *
 * Arduino D2 <- HC-06 TX
 * Arduino D3 -> HC-06 RX
 *
 * HC-06 RX에는 필요에 따라 전압 분배 회로를 사용합니다.
 */
const uint8_t BLUETOOTH_RX_PIN = 2;
const uint8_t BLUETOOTH_TX_PIN = 3;

SoftwareSerial bluetooth(
    BLUETOOTH_RX_PIN,
    BLUETOOTH_TX_PIN
);

/* =========================================================
 * L9110 모터 드라이버 핀
 * =========================================================
 *
 * 왼쪽 모터:
 * LEFT_MOTOR_A
 * LEFT_MOTOR_B
 *
 * 오른쪽 모터:
 * RIGHT_MOTOR_A
 * RIGHT_MOTOR_B
 */
const uint8_t LEFT_MOTOR_A = 5;
const uint8_t LEFT_MOTOR_B = 6;

const uint8_t RIGHT_MOTOR_A = 9;
const uint8_t RIGHT_MOTOR_B = 10;

/* =========================================================
 * HC-SR04 초음파 센서 핀
 * =========================================================
 */
const uint8_t TRIG_PIN = 7;
const uint8_t ECHO_PIN = 8;

/* =========================================================
 * 동작 설정
 * =========================================================
 */
const float OBSTACLE_DISTANCE_CM = 20.0f;
const unsigned long DISTANCE_INTERVAL_MS = 300;

unsigned long lastDistanceCheckTime = 0;
bool obstacleDetected = false;

/* =========================================================
 * 명령 확인
 * =========================================================
 */
bool isValidCommand(char command)
{
    return command == 'w'
        || command == 's'
        || command == 'a'
        || command == 'd'
        || command == 'x';
}

/* =========================================================
 * Python 브릿지로 명령 전달
 * =========================================================
 *
 * Arduino USB Serial을 통해 Windows PC로 전달합니다.
 *
 * 전송 예:
 * CMD:w
 * CMD:s
 * CMD:a
 * CMD:d
 * CMD:x
 * CMD:B
 *
 * Python 브릿지는 CMD:로 시작하는 데이터만 제어 명령으로 처리합니다.
 */
void sendCommandToBridge(char command)
{
    Serial.print("CMD:");
    Serial.println(command);
}

/* 거리 로그 전송 */
void sendDistanceLog(float distanceCm)
{
    Serial.print("DIST:");
    Serial.println(distanceCm, 1);
}

/* =========================================================
 * 전체 모터 정지
 * =========================================================
 */
void stopMotors()
{
    digitalWrite(LEFT_MOTOR_A, LOW);
    digitalWrite(LEFT_MOTOR_B, LOW);

    digitalWrite(RIGHT_MOTOR_A, LOW);
    digitalWrite(RIGHT_MOTOR_B, LOW);
}

/* =========================================================
 * 전진
 * =========================================================
 */
void moveForward()
{
    digitalWrite(LEFT_MOTOR_A, HIGH);
    digitalWrite(LEFT_MOTOR_B, LOW);

    digitalWrite(RIGHT_MOTOR_A, HIGH);
    digitalWrite(RIGHT_MOTOR_B, LOW);
}

/* =========================================================
 * 후진
 * =========================================================
 */
void moveBackward()
{
    digitalWrite(LEFT_MOTOR_A, LOW);
    digitalWrite(LEFT_MOTOR_B, HIGH);

    digitalWrite(RIGHT_MOTOR_A, LOW);
    digitalWrite(RIGHT_MOTOR_B, HIGH);
}

/* =========================================================
 * 좌회전
 * =========================================================
 */
void turnLeft()
{
    digitalWrite(LEFT_MOTOR_A, LOW);
    digitalWrite(LEFT_MOTOR_B, HIGH);

    digitalWrite(RIGHT_MOTOR_A, HIGH);
    digitalWrite(RIGHT_MOTOR_B, LOW);
}

/* =========================================================
 * 우회전
 * =========================================================
 */
void turnRight()
{
    digitalWrite(LEFT_MOTOR_A, HIGH);
    digitalWrite(LEFT_MOTOR_B, LOW);

    digitalWrite(RIGHT_MOTOR_A, LOW);
    digitalWrite(RIGHT_MOTOR_B, HIGH);
}

/* =========================================================
 * 초음파 거리 측정
 * =========================================================
 */
float measureDistanceCm()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(
        ECHO_PIN,
        HIGH,
        30000UL
    );

    /*
     * 제한 시간 안에 Echo 신호가 없으면
     * -1을 반환합니다.
     */
    if (duration == 0)
    {
        return -1.0f;
    }

    /*
     * 음속 약 0.0343cm/us
     * 왕복 시간이므로 2로 나눕니다.
     */
    return duration * 0.0343f / 2.0f;
}

/* =========================================================
 * RC카 제어 명령 실행
 * =========================================================
 */
void executeCommand(char command)
{
    switch (command)
    {
        case 'w':
            /*
             * 장애물이 감지된 상태에서는
             * 전진 명령을 실행하지 않습니다.
             */
            if (obstacleDetected)
            {
                stopMotors();
                sendCommandToBridge('B');
                return;
            }

            moveForward();
            break;

        case 's':
            /*
             * 장애물 상태에서도 후진은 허용합니다.
             */
            moveBackward();
            break;

        case 'a':
            turnLeft();
            break;

        case 'd':
            turnRight();
            break;

        case 'x':
            stopMotors();
            break;

        default:
            return;
    }

    /*
     * 실제 실행한 명령을 Python 브릿지로 전달합니다.
     */
    sendCommandToBridge(command);
}

/* =========================================================
 * 장애물 감지
 * =========================================================
 */
void checkObstacle()
{
    unsigned long currentTime = millis();

    /*
     * 300ms 간격으로만 거리 측정
     */
    if (
        currentTime - lastDistanceCheckTime
        < DISTANCE_INTERVAL_MS
    )
    {
        return;
    }

    lastDistanceCheckTime = currentTime;

    float distanceCm = measureDistanceCm();

    /*
     * 측정 실패 시 이번 결과는 무시합니다.
     */
    if (distanceCm < 0.0f)
    {
        Serial.println("WARN:DISTANCE_TIMEOUT");
        return;
    }

    sendDistanceLog(distanceCm);

    /*
     * 20cm 이내 장애물 감지
     */
    if (distanceCm <= OBSTACLE_DISTANCE_CM)
    {
        stopMotors();

        /*
         * 장애물이 처음 감지됐을 때만
         * B 명령을 한 번 전송합니다.
         */
        if (!obstacleDetected)
        {
            obstacleDetected = true;

            sendCommandToBridge('B');
            Serial.println("INFO:OBSTACLE_DETECTED");
        }
    }
    else
    {
        /*
         * 장애물이 감지 범위 밖으로 이동한 경우
         */
        if (obstacleDetected)
        {
            obstacleDetected = false;
            Serial.println("INFO:OBSTACLE_CLEARED");
        }
    }
}

/* =========================================================
 * 초기 설정
 * =========================================================
 */
void setup()
{
    /* 모터 핀 출력 설정 */
    pinMode(LEFT_MOTOR_A, OUTPUT);
    pinMode(LEFT_MOTOR_B, OUTPUT);

    pinMode(RIGHT_MOTOR_A, OUTPUT);
    pinMode(RIGHT_MOTOR_B, OUTPUT);

    /* 초음파 센서 설정 */
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    /* 시작 시 모터 정지 */
    stopMotors();

    /*
     * Arduino ↔ Windows PC
     * USB Serial 통신
     */
    Serial.begin(115200);

    /*
     * 스마트폰 ↔ HC-06 ↔ Arduino
     */
    bluetooth.begin(9600);

    delay(500);

    Serial.println("INFO:RC_CAR_CONTROLLER_STARTED");
}

/* =========================================================
 * 반복 실행
 * =========================================================
 */
void loop()
{
    /*
     * Bluetooth 명령 확인
     */
    if (bluetooth.available() > 0)
    {
        char command = bluetooth.read();

        /*
         * 대문자가 전송되면 소문자로 변환
         */
        if (command >= 'A' && command <= 'Z')
        {
            command = command - 'A' + 'a';
        }

        /*
         * 줄바꿈 문자와 유효하지 않은 데이터는 무시
         */
        if (isValidCommand(command))
        {
            executeCommand(command);
        }
    }

    /*
     * 주기적인 장애물 거리 확인
     */
    checkObstacle();
}
