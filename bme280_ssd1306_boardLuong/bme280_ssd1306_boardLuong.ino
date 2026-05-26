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

//--- Biến toàn cục cho việc thay đổi màu LED và hiển thị ---
const uint32_t colors[] = {
  0xFF0000, // RED
  0x00FF00, // GREEN
  0x0000FF, // BLUE
  0xFFFF00, // YELLOW
  0xFF00FF, // MAGENTA
  0x00FFFF, // CYAN
  0xFFFFFF  // WHITE
};
const char* colorNames[] = {
  "Red",
  "Green",
  "Blue",
  "Yellow",
  "Magenta",
  "Cyan",
  "White"
};
const uint8_t colorCount = sizeof(colors) / sizeof(colors[0]);
uint8_t currentColorIdx = 0;
unsigned long lastChangeTime = 0;
const unsigned long changeInterval = 2000; // 2 giây

void getupdate()
{
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
      display.printf("Firmware Size: %d",firmwareSize);
      if (Update.begin(firmwareSize))
      {
          size_t written = Update.writeStream(client);
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
  // Khởi tạo I2C, LED và OLED (giữ lại các dòng hiện có)
  Wire.begin(13,12);
  led.begin();
  led.setBrightness(50);
  // Đặt màu ban đầu và hiển thị tên màu
  led.setPixelColor(0, colors[currentColorIdx]);
  led.show();
  if (!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address)) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    Serial.println("OLED fail!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("He thong dang khoi dong...");
  display.display();
  delay(1000);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  Serial.begin(115200);
  Serial.println("He thong dang khoi dong...");
  display.display();
  display.clearDisplay();
  WiFi.begin(ssid,pass);
  // (phần kết nối WiFi giữ nguyên)
  while (WiFi.status() != WL_CONNECTED) {
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();
    Serial.println("dang khoi dong WiFi...");
    display.setCursor(0,0);
    display.print("Conecting WiFi");
    // biến demwf chưa khai báo trong source gốc, bỏ qua phần hiển thị chấm
    digitalWrite(LED,1);
    delay(300);
  }
  // Cấu hình thời gian và Firebase giữ nguyên
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
  digitalWrite(LED,0);
  Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.println("XIN CHAO CAC BAN");
  display.display();
  delay(300);
  display.clearDisplay();
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();

  // Hiển thị màu hiện tại trên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Mau hien tai: ");
  display.println(colorNames[currentColorIdx]);
  display.display();
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
  /*
    Xây dựng cơ chế xử lý của bạn tại đây
  */
  // Thay đổi màu LED mỗi 2 giây và cập nhật OLED
  unsigned long now = millis();
  if (now - lastChangeTime >= changeInterval) {
    lastChangeTime = now;
    currentColorIdx = (currentColorIdx + 1) % colorCount;
    // Cập nhật LED
    led.setPixelColor(0, colors[currentColorIdx]);
    led.show();
    // Cập nhật OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Mau hien tai: ");
    display.println(colorNames[currentColorIdx]);
    display.display();
  }
}
