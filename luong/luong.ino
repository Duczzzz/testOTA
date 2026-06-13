// Đây là source trống cho người dùng tự build trên board do Nuke Dashboard phát triển
// Để có thể sử dụng source code này bạn cần cài danh sách các thư viện sau: 
// + thư viện Adafruit NeoPixel by Adafruit
// + thư viện Firebase ESP32 Client by Mobizt
// + thư viện Adafruit GFX libraray by Adafruit
// + thư viện Adafruit SSD1306 by Adafruit
// Tác giả MinhDuc
// 07/03/2026
// Led RGB chân 9
// BME280 SDA chân 8
// BME280 SCL chân 18
// Oled tft SDA chân 13
// Oled tft SCL chân 12
// DHT chân 11
// Điều khiển driver động cơ chân 16 và 15
// Các nút nhấn hoạt động tích cực mức thấp 
// Nút nhấn SW8 kết nối chân GPIO10
// Nút nhấn SW9 kết nối chân GPIO12 
// Nút nhấn SW11 kết nối chân GPIO14
#include <Wire.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Adafruit_BME280.h>

const char* ssid = "Su Ni";
const char* pass = "04072009";
#define LED_COUNT 1
#define LED_RGB 9
Adafruit_NeoPixel led(LED_COUNT, LED_RGB, NEO_GRB + NEO_KHZ800);

#define LED 2

TwoWire I2C_BME = TwoWire(0);
TwoWire I2C_OLED = TwoWire(1);

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, OLED_RESET);

Adafruit_BME280 bme;

const char* firmwareUrl = "https://raw.githubusercontent.com/Duczzzz/testOTA/main/firmware_luong.ino.bin";

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int checkupdate = 0;
int demwf = 0;

// Định nghĩa các nút
const int BTN_RED   = 10; // SW8
const int BTN_GREEN = 12; // SW9
const int BTN_BLUE  = 14; // SW11

// Trạng thái hiện tại của LED
enum ColorState { RED, GREEN, BLUE };
ColorState currentColor = RED;

// Hàm hiển thị màu hiện tại lên OLED
void displayColor() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Mau LED: ");
  switch (currentColor) {
    case RED:   display.print("Do"); break;
    case GREEN: display.print("Xanh la"); break;
    case BLUE:  display.print("Xanh duong"); break;
  }
  display.display();
}

// Hàm đổi màu LED và cập nhật OLED
void setColor(ColorState col) {
  currentColor = col;
  switch (col) {
    case RED:   led.setPixelColor(0, led.Color(255, 0, 0)); break;
    case GREEN: led.setPixelColor(0, led.Color(0, 255, 0)); break;
    case BLUE:  led.setPixelColor(0, led.Color(0, 0, 255)); break;
  }
  led.show();
  displayColor();
}

void getupdate()
{
    display.setTextColor(SSD1306_WHITE);
    Firebase.setInt(fbdo, "/users/luong/updateOTA",0);  
    Serial.print("Firmware URL: ");
    Serial.println(firmwareUrl);
    HTTPClient http;
    http.begin(firmwareUrl);
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
  // Khởi tạo I2C cho BME280 và OLED
  I2C_BME.begin(8,18);
  I2C_OLED.begin(13,12);
  // Khởi tạo LED RGB
  led.begin();
  led.setBrightness(50);
  // Đặt màu mặc định RED và hiển thị
  setColor(RED);
  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address)) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    Serial.println("OLED fail!");
    while (1);
  }
  display.clearDisplay();
  display.setCursor(25, 30);
  display.print("NUKEDASHBOARD");
  // Khởi tạo BME280
  if (!bme.begin(0x76,&I2C_BME)) {
    display.clearDisplay();
    Serial.println("Không tìm thấy BME280!");
    display.setCursor(0, 0);
    display.printf("Khong tim thay BME280!");
    display.display();
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    while (1);
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  delay(1000);
  // Cấu hình LED báo trạng thái
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  // Serial
  Serial.begin(115200);
  Serial.println("He thong dang khoi dong...");
  display.display();
  display.clearDisplay();
  // WiFi
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();
    Serial.println("dang khoi dong WiFi...");
    display.setCursor(10,0);
    display.print("Dang ket noi WiFi");
    display.setCursor(0,20);
    display.printf("SSID: %s",ssid);
    if(demwf < 80) {
      display.setCursor(demwf,30);
      display.print(".");
      Serial.println(".");
    }
    else if(demwf > 80) {
      display.clearDisplay();
      demwf = 0;
    }
    demwf+=5;
    display.display();
    digitalWrite(LED,1);
    delay(300);
  }
  digitalWrite(LED,0);
  Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
  display.clearDisplay();
  display.display();

  // Cấu hình các nút nhấn
  pinMode(BTN_RED,   INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);
  pinMode(BTN_BLUE,  INPUT_PULLUP);
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
  // Đọc nút nhấn và đổi màu LED nếu có nhấn
  static bool lastRedState   = HIGH;
  static bool lastGreenState = HIGH;
  static bool lastBlueState  = HIGH;
  bool curRedState   = digitalRead(BTN_RED);
  bool curGreenState = digitalRead(BTN_GREEN);
  bool curBlueState  = digitalRead(BTN_BLUE);

  // Debounce đơn giản
  if (curRedState == LOW && lastRedState == HIGH) {
    setColor(RED);
    delay(200);
  }
  if (curGreenState == LOW && lastGreenState == HIGH) {
    setColor(GREEN);
    delay(200);
  }
  if (curBlueState == LOW && lastBlueState == HIGH) {
    setColor(BLUE);
    delay(200);
  }

  lastRedState   = curRedState;
  lastGreenState = curGreenState;
  lastBlueState  = curBlueState;

  // Thêm một chút delay để giảm tải CPU
  delay(10);
}