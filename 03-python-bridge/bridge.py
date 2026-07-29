from __future__ import annotations

import socket
import time

import serial
from serial import SerialException


# ============================================================
# Arduino Serial 설정
# ============================================================

# Windows 장치 관리자에서 확인한 Arduino COM 포트로 수정합니다.
ARDUINO_PORT = "COM3"

# Arduino 코드의 Serial.begin(115200)과 일치해야 합니다.
ARDUINO_BAUDRATE = 115200


# ============================================================
# TCP 서버 설정
# ============================================================

# 0.0.0.0은 현재 PC의 모든 네트워크 인터페이스에서
# 연결을 허용한다는 의미입니다.
TCP_BIND_IP = "0.0.0.0"

# stm32_gateway.py와 동일한 포트를 사용해야 합니다.
TCP_PORT = 5000


VALID_COMMANDS = {"w", "s", "a", "d", "x", "B"}


def parse_command(serial_line: str) -> str | None:
    """
    Arduino가 전송한 CMD:x 형식의 데이터에서
    제어 명령만 추출합니다.

    예:
        CMD:w -> w
        CMD:B -> B
        DIST:20.5 -> 무시
        INFO:... -> 무시
    """

    if not serial_line.startswith("CMD:"):
        return None

    command = serial_line[4:].strip()

    if command not in VALID_COMMANDS:
        return None

    return command


def open_arduino_serial() -> serial.Serial:
    """
    Arduino가 연결된 COM 포트를 엽니다.
    """

    while True:
        try:
            arduino = serial.Serial(
                port=ARDUINO_PORT,
                baudrate=ARDUINO_BAUDRATE,
                timeout=0.2,
            )

            print(
                f"[Arduino 연결 성공] "
                f"{ARDUINO_PORT}, "
                f"{ARDUINO_BAUDRATE}bps"
            )

            return arduino

        except SerialException as error:
            print(f"[Arduino 연결 실패] {error}")
            print("[재시도] 2초 후 다시 연결합니다.")

            time.sleep(2)


def create_tcp_server() -> socket.socket:
    """
    STM32 Gateway가 연결할 TCP 서버를 생성합니다.
    """

    server = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM,
    )

    server.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1,
    )

    server.bind(
        (
            TCP_BIND_IP,
            TCP_PORT,
        )
    )

    server.listen(1)

    print(
        f"[TCP 서버 시작] "
        f"{TCP_BIND_IP}:{TCP_PORT}"
    )

    return server


def run_bridge() -> None:
    """
    Arduino UART 데이터를 수신한 뒤
    TCP/IP로 STM32 Gateway에 전달합니다.
    """

    arduino = open_arduino_serial()
    server = create_tcp_server()

    try:
        while True:
            print("[TCP 대기] STM32 Gateway 연결 대기 중")

            client, address = server.accept()

            print(
                f"[TCP 연결 성공] "
                f"{address[0]}:{address[1]}"
            )

            try:
                while True:
                    raw_data = arduino.readline()

                    if not raw_data:
                        continue

                    serial_line = raw_data.decode(
                        "utf-8",
                        errors="ignore",
                    ).strip()

                    if not serial_line:
                        continue

                    print(f"[Arduino 수신] {serial_line}")

                    command = parse_command(serial_line)

                    if command is None:
                        continue

                    packet = f"{command}\n".encode("utf-8")

                    client.sendall(packet)

                    print(f"[TCP 전송] {command}")

            except (
                ConnectionError,
                ConnectionResetError,
                BrokenPipeError,
                OSError,
            ) as error:
                print(f"[TCP 연결 종료] {error}")

            finally:
                client.close()

    except KeyboardInterrupt:
        print("\n[종료] 사용자 요청으로 브릿지를 종료합니다.")

    finally:
        arduino.close()
        server.close()


if __name__ == "__main__":
    run_bridge()
