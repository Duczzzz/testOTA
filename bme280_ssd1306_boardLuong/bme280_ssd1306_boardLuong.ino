#include <Arduino.h>

// Thiết lập chân LED (GPIO 2 thường được dùng cho board ESP32)
const int ledPin = 2; // Thay đổi nếu dùng chân khác

void setup() {
    // Cấu hình chân LED là OUTPUT
    pinMode(ledPin, OUTPUT);
}

void loop() {
    // Bật LED
    digitalWrite(ledPin, HIGH);
    delay(1000); // Đợi 1 giây

    // Tắt LED
    digitalWrite(ledPin, LOW);
    delay(1000); // Đợi 1 giây
}