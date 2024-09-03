#include "mbed.h"

// UART接続用ピンの定義
BufferedSerial uart(PB_6, PA_10, 2000000); // TX, RX for UART
BufferedSerial pc(USBTX, USBRX, 9600); // TX, RX for USB Serial
DigitalOut uart_control(D6); // DE/RE ピン
char command = 0x54; // 送信するバイナリデータ（デフォルトのエンコーダーアドレス）

void send_uart_command() {
    uart_control = 1; // 送信モードに切り替え
    uart.write(&command, 1);
    wait_us(3); // 送信完了後の遅延を追加
    uart_control = 0; // 受信モードに切り替え
}

bool verify_checksum(uint16_t response) {
    uint8_t high_byte = (response >> 8) & 0xFF;
    uint8_t low_byte = response & 0xFF;

    bool k1 = !((high_byte & 0x20) ^ (high_byte & 0x08) ^ (high_byte & 0x02) ^ (low_byte & 0x80) ^ (low_byte & 0x20) ^ (low_byte & 0x08) ^ (low_byte & 0x02));
    bool k0 = !((high_byte & 0x10) ^ (high_byte & 0x04) ^ (high_byte & 0x01) ^ (low_byte & 0x40) ^ (low_byte & 0x10) ^ (low_byte & 0x04) ^ (low_byte & 0x01));

    return (((response & 0x8000) >> 15) == k1) && (((response & 0x4000) >> 14) == k0);
}

void receive_uart_data() {
    // データが読み取れるまで待機
    while (!uart.readable()) {
        // ここで待機
    }

    char received_data[2]; // 2バイトのデータを受信
    uart.read(received_data, sizeof(received_data)); // データを読み取る

    uint16_t response = (received_data[1] << 8) | received_data[0];

    if (verify_checksum(response)) {
        uint16_t position = response & 0x3FFF; // 下位14ビットが位置データ

        // シフトしたデータを表示
        char debug_msg[32];
        int len = sprintf(debug_msg, "Position: %u\r\n", position);
        pc.write(debug_msg, len);
    } else {
        pc.write("Checksum error\r\n", 16);
    }
}

int main() {
    while (true) {
        send_uart_command(); // コマンド送信
        ThisThread::sleep_for(1ms); // 1ミリ秒待機
        receive_uart_data(); // 受信データを処理
    }
}