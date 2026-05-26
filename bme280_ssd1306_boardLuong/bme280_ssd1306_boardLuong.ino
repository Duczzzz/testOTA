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

/*-------------------- OTA Function --------------------*/
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

/*-------------------- Color Management --------------------*/
struct LedColor {
  const char* name;
  uint32_t value;
};

LedColor colors[] = {
  {"Red",     0xFF0000},
  {"Green",   0x00FF00},
  {"Blue",    0x0000FF},
  {"Yellow",  0xFFFF00},
  {"Cyan",    0x00FFFF},
  {"Magenta", 0xFF00FF},
  {"White",   0xFFFFFF}
};
const int colorCount = sizeof(colors) / sizeof(colors[0]);
int currentColorIndex = 0;
unsigned long lastColorChange = 0;
const unsigned long colorInterval = 2000; // 2 giây

void setLedColor(int idx) {
  uint32_t c = colors[idx].value;
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >> 8) & 0xFF;
  uint8_t b = c & 0xFF;
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Mau hien tai:");
  display.setCursor(0, 12);
  display.print(colors[idx].name);
  display.display();
}

/*-------------------- Setup --------------------*/
void setup() {
  /*
    Người dùng build code tại đây
  */
  // Khởi tạo I2C, LED và OLED (giữ nguyên phần cũ)
  Wire.begin(13,12);
  led.begin();
  led.setBrightness(50);
  // Thiết lập màu ban đầu
  setLedColor(currentColorIndex);
  
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
  display.printf("He thong dang \nkhoi dong...");
  display.display();
  delay(1000);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  Serial.begin(1150);
  Serial.println("He thong dang khoi dong...");
  display.display();
  display.clearDisplay();
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();
    Serial.println("dang khoi dong WiFi...");
    display.setCursor(0,0);
    display.print("Conecting WiFi");
    // Giữ nguyên phần hiển thị chấm (đã có biến demwf trong code gốc)
    if(demwf < 80) {
      display.setCursor(demwf,10);
      display.print(".");
      Serial0.println(".");
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
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.println("XIN CHAO CAC BAN");
  display.display();
  delay(300);
  display.clearDisplay();
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
}

/*-------------------- Loop --------------------*/
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
  // Thay đổi màu LED và hiển thị màu hiện tại mỗi 2 giây
  unsigned long now = millis();
  if (now - lastColorChange >= colorInterval) {
    lastColorChange = now;
    currentColorIndex = (currentColorIndex + 1) % colorCount;
    setLedColor(currentColorIndex);
  }
}