from __future__ import annotations

import socket
import time

import serial
from serial import SerialException


# ============================================================
# Python Bridge TCP 설정
# ============================================================

# bridge.py와 같은 PC에서 실행하면 127.0.0.1을 사용합니다.
# 다른 PC에서 실행하면 bridge.py가 실행되는 PC의 IPv4 주소로 변경합니다.
BRIDGE_IP = "127.0.0.1"

# bridge.py의 TCP_PORT와 같아야 합니다.
BRIDGE_PORT = 5000


# ============================================================
# STM32 Serial 설정
# ============================================================

# Windows 장치 관리자에서 확인한 STM32 COM 포트로 수정합니다.
STM32_PORT = "COM4"

# STM32 USART3 설정과 일치해야 합니다.
STM32_BAUDRATE = 9600


VALID_COMMANDS = {"w", "s", "a", "d", "x", "B"}


def open_stm32_serial() -> serial.Serial:
    """
    STM32가 연결된 COM 포트를 엽니다.
    """

    while True:
        try:
            stm32 = serial.Serial(
                port=STM32_PORT,
                baudrate=STM32_BAUDRATE,
                timeout=0.2,
            )

            print(
                f"[STM32 연결 성공] "
                f"{STM32_PORT}, "
                f"{STM32_BAUDRATE}bps"
            )

            return stm32

        except SerialException as error:
            print(f"[STM32 연결 실패] {error}")
            print("[재시도] 2초 후 다시 연결합니다.")

            time.sleep(2)


def connect_to_bridge() -> socket.socket:
    """
    bridge.py TCP 서버에 연결합니다.
    """

    while True:
        client = socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM,
        )

        try:
            client.connect(
                (
                    BRIDGE_IP,
                    BRIDGE_PORT,
                )
            )

            print(
                f"[TCP 연결 성공] "
                f"{BRIDGE_IP}:{BRIDGE_PORT}"
            )

            return client

        except OSError as error:
            client.close()

            print(f"[TCP 연결 실패] {error}")
            print("[재시도] 2초 후 다시 연결합니다.")

            time.sleep(2)


def run_gateway() -> None:
    """
    TCP/IP로 명령을 수신하고
    STM32 UART로 1바이트 명령을 전달합니다.
    """

    stm32 = open_stm32_serial()

    try:
        while True:
            client = connect_to_bridge()
            receive_buffer = ""

            try:
                while True:
                    received_data = client.recv(1024)

                    if not received_data:
                        raise ConnectionError(
                            "Python Bridge 연결이 종료되었습니다."
                        )

                    receive_buffer += received_data.decode(
                        "utf-8",
                        errors="ignore",
                    )

                    while "\n" in receive_buffer:
                        line, receive_buffer = receive_buffer.split(
                            "\n",
                            1,
                        )

                        command = line.strip()

                        if command not in VALID_COMMANDS:
                            print(
                                f"[명령 무시] "
                                f"잘못된 명령: {command}"
                            )
                            continue

                        stm32.write(
                            command.encode("utf-8")
                        )

                        stm32.flush()

                        print(f"[STM32 전송] {command}")

            except (
                ConnectionError,
                ConnectionResetError,
                OSError,
            ) as error:
                print(f"[TCP 오류] {error}")

            finally:
                client.close()

    except KeyboardInterrupt:
        print("\n[종료] 사용자 요청으로 Gateway를 종료합니다.")

    finally:
        stm32.close()


if __name__ == "__main__":
    run_gateway()
