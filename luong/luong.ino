// Global definitions
struct LedColor {
  uint32_t rgb;
  const char* name;
};
LedColor colors[] = {
  {0xFF0000, "Red"},
  {0x00FF00, "Green"},
  {0x0000FF, "Blue"},
  {0xFFFF00, "Yellow"},
  {0xFF00FF, "Magenta"},
  {0x00FFFF, "Cyan"},
  {0xFFFFFF, "White"},
  {0x000000, "Off"}
};
const uint8_t colorCount = sizeof(colors) / sizeof(colors[0]);
volatile uint8_t currentColor = 0;

#define BTN_PIN 10          // SW8 button
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
bool lastButtonState = HIGH;

void setup() {
  /*
    Người dùng build code tại đây
  */
  // Khởi tạo I2C, LED, OLED, BME đã có trong phần trên
  pinMode(BTN_PIN, INPUT_PULLUP);
  // Đặt màu LED ban đầu
  led.setPixelColor(0, colors[currentColor].rgb);
  led.show();
  // Hiển thị màu hiện tại lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Mau LED: ");
  display.print(colors[currentColor].name);
  display.display();
}

void loop() {
  if(Firebase.getInt(fbdo, "/users/luong/updateOTA")) checkupdate = fbdo.intData();
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
  // Đọc trạng thái nút bấm
  bool reading = digitalRead(BTN_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      // Nút được nhấn, chuyển sang màu tiếp theo
      currentColor = (currentColor + 1) % colorCount;
      led.setPixelColor(0, colors[currentColor].rgb);
      led.show();
      // Cập nhật OLED
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Mau LED: ");
      display.print(colors[currentColor].name);
      display.display();
    }
  }
  lastButtonState = reading;
}