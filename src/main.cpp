#include "mbed.h"

// RS-485接続用ピンの定義
BufferedSerial rs485(PB_6, PA_10, 200000); // TX, RX for RS-485
BufferedSerial pc(USBTX, USBRX, 9600); // シリアル通信用
DigitalOut rs485_control_1(D6);
// DigitalOut rs485_control_2(D7); // DE/RE ピン

// エンコーダからのデータを格納する変数
char encoder_data[2];

void send_rs485_command() {
    printf("Sending RS-485 command\r\n");
    rs485_control_1 = 1;
    // rs485_control_2 = 0; // 送信モードに切り替え
    
    char command = 0x51; // Position read command
    rs485.write(&command, 1); // Send command
    
    rs485_control_1 = 0; // 受信モードに切り替え
    // rs485_control_2 = 1;
}

void read_encoder_value() {
    send_rs485_command(); // Send command

    // データ受信
    rs485.read(encoder_data, sizeof(encoder_data)); // Read 2 bytes for position data

    // 受信したデータを処理する（例: デバッグ用にシリアル出力）
    uint16_t position = (encoder_data[0] << 8) | encoder_data[1]; // Combine into 16-bit value
    position &= 0x0FFF; // 12ビットのデータをマスク
    position <<= 2; // 左に2ビットシフト
    printf("Position: %u\r\n", position); // Output the position
}

int main() {
    while (true) {
        read_encoder_value(); // Read and display encoder value
        ThisThread::sleep_for(1000ms); // 1秒待機
    }
}