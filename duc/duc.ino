// ----- Global definitions -----
const uint8_t BTN_SW8 = 10;               // GPIO10 cho nút SW8, active low
const unsigned long debounceDelay = 50;   // thời gian debounce (ms)
unsigned long lastDebounceTime = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;

const uint32_t colors[4] = {
  0xFF0000, // Đỏ
  0x00FF00, // Xanh lá
  0x0000FF, // Xanh dương
  0xFFFF00  // Vàng
};
const char* colorNames[4] = {
  "Do",
  "Xanh",
  "Xanh Duong",
  "Vang"
};
int colorIndex = 0;

// ----- Hàm setup -----
void setup() {
  /*
    Người dùng build code tại đây
  */
  // Cấu hình I2C, LED, OLED, BME280 đã có ở trên
  // Thiết lập nút nhấn SW8
  pinMode(BTN_SW8, INPUT_PULLUP);

  // Đặt màu LED ban đầu và hiển thị lên OLED
  led.setPixelColor(0, colors[colorIndex]);
  led.show();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mau: ");
  display.print(colorNames[colorIndex]);
  display.display();
}

// ----- Hàm loop -----
void loop() {
  if(Firebase.getInt(fbdo, "/users/duc/updateOTA")) checkupdate = fbdo.intData();
  if(checkupdate == 1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("UPDATE OTA");
    display.display();
    getupdate();
  }
  /*
    Xây dựng cơ chế xử lý của bạn tại đây
  */
  // Đọc trạng thái nút SW8 với debounce
  int reading = digitalRead(BTN_SW8);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) { // nút được nhấn
        // Chuyển sang màu tiếp theo
        colorIndex = (colorIndex + 1) % 4;
        led.setPixelColor(0, colors[colorIndex]);
        led.show();

        // Cập nhật OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Mau: ");
        display.print(colorNames[colorIndex]);
        display.display();
      }
    }
  }
  lastButtonState = reading;
}