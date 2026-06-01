// Đây là source trống cho người dùng tự build trên board do Nuke Dashboard phát triển
// Để có thể sử dụng source code này bạn cần cài danh sách các thư viện sau: 
// + thư viện Adafruit NeoPixel by Adafruit
// + thư viện Firebase ESP32 Client by Mobizt
// + thư viện Adafruit GFX libraray by Adafruit
// + thư viện Adafruit SSD1306 by Adafruit
// Tác giả MinhDuc
// 07/03/2026
// Led RGB được cấu hình chân DIN ở GPIO9
// BME280 SDA chân 8
// BME280 SCL chân 18
// Oled tft SDA chân 8
// Oled tft SCL chân 18
// DHT chân 11
// Điều khiển driver động cơ chân GPIO16 và GPIO15

#include <Wire.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "DUC";
const char* pass = "14042004";

#define LED_PIN   9
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LED 2

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* firmwareUrl = "https://raw.githubusercontent.com/Duczzzz/testOTA/main/firmware.ino.bin";

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int checkupdate = 0;

// ---------- Thông tin màu LED ----------
struct ColorInfo {
  uint32_t color;
  const char* name;
};

ColorInfo colors[] = {
  {0xFF0000, "Red"},
  {0x00FF00, "Green"},
  {0x0000FF, "Blue"},
  {0xFFFF00, "Yellow"},
  {0xFF00FF, "Magenta"},
  {0x00FFFF, "Cyan"},
  {0xFFFFFF, "White"},
  {0x000000, "Off"}
};
int colorCount = sizeof(colors) / sizeof(colors[0]);
int currentIndex = 0;
unsigned long lastChange = 0;
unsigned long changeInterval = 2000; // 2 giây

void getupdate()
{
    display.setTextColor(SSD1306_WHITE);
    Firebase.setInt(fbdo, "/updateOTA",0);  
    Serial.print("Firmware URL: ");
    Serial.println(firmwareUrl);
    HTTPClient http;
    http.begin(firmwareUrl);
    Firebase.setInt(fbdo,"/updateOTA",0);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
      WiFiClient& client = http.getStream();
      int firmwareSize = http.getSize();
      display.clearDisplay();
      display.setTextSize(1);
      Serial.print("Firmware Size: ");
      Serial.println(firmwareSize);
      display.setCursor(0, 0);
      display.printf("Size: %d",firmwareSize);
      display.display();
      if (Update.begin(firmwareSize))
      {
          Update.onProgress([](size_t current, size_t total) {
              int percent = (current * 100) / total;

              Serial.printf("OTA %d%%\n", percent);

              display.clearDisplay();
              display.setCursor(0,0);
              display.print("Updating");

              display.setCursor(0,20);
              display.print(percent);
              display.print("%");
              display.drawRect(0, 30, 120, 10, SSD1306_WHITE);
              display.fillRect(
                    2,
                    32,
                    (percent * 116) / 100,
                    6,
                    SSD1306_WHITE);
              display.display();
          });
          size_t written = Update.writeStream(client);
          display.clearDisplay();
          if (Update.size() == written)
          {
              display.setCursor(0, 10);
              display.print("Update successfully completed");
              Serial.println("Update successfully completed. Rebooting...");
              if (Update.end())
              {
                  Serial.println("Rebooting...");
                  display.setCursor(0, 30);
                  display.printf("Rebooting...");
                  ESP.restart();
              } 
              else 
              {
                  Serial.print("Update failed: ");
                  display.setCursor(0, 30);
                  display.print("Update failed");
                  Serial.println(Update.errorString());
              }
          }
          else
          {
              display.setCursor(0, 30);
              display.print("Not enough space for OTA.");
              Serial.println("Not enough space for OTA.");
          }
      } 
        else
        {
            display.setCursor(0, 10);
            display.print("Failed to begin OTA update.");
            Serial.println("Failed to begin OTA update.");
        }
    }
    else
    {
        display.setCursor(0, 10);
        display.print("Failed to download firmware. HTTP code: ");
        display.println(httpCode);
        Serial.print("Failed to download firmware. HTTP code: ");
        Serial.println(httpCode);
    }
    display.display();
    delay(400);
    http.end();
}

void setup() {
  /*
    Người dùng build code tại đây
  */
  // Khởi tạo I2C, LED, OLED như trước
  // Đã có trong phần code gốc, không thay đổi

  // Thiết lập hiển thị màu hiện tại lần đầu
  led.setPixelColor(0, colors[currentIndex].color);
  led.show();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Mau hien tai: ");
  display.println(colors[currentIndex].name);
  display.display();
  lastChange = millis();
}

void loop() {
  if(Firebase.getInt(fbdo, "/updateOTA")) checkupdate = fbdo.intData();
  if(checkupdate == 1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("UPDATE OTA");
    display.display();
    getupdate();
  }

  // ----- Xây dựng cơ chế xử lý của bạn tại đây -----
  // Thay đổi màu LED mỗi khoảng changeInterval và hiển thị lên OLED
  unsigned long now = millis();
  if (now - lastChange >= changeInterval) {
    lastChange = now;
    currentIndex = (currentIndex + 1) % colorCount;
    // Cập nhật LED
    led.setPixelColor(0, colors[currentIndex].color);
    led.show();
    // Cập nhật OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Mau hien tai: ");
    display.println(colors[currentIndex].name);
    display.display();
  }

  // Thêm một số hiệu ứng đơn giản: nhấp nháy khi màu là "Off"
  if (strcmp(colors[currentIndex].name, "Off") == 0) {
    static bool blinkState = false;
    static unsigned long blinkLast = 0;
    if (now - blinkLast >= 500) {
      blinkLast = now;
      blinkState = !blinkState;
      uint32_t c = blinkState ? 0xFFFFFF : 0x000000;
      led.setPixelColor(0, c);
      led.show();
    }
  }
  // Kết thúc phần xử lý
}