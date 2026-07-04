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
// Động cơ servo kết nối chân GPIO17
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
#include <ESP32Servo.h>

const char* ssid = "Su Ni";
const char* pass = "04072009";
#define LED_COUNT 1
#define LED_RGB 9
Adafruit_NeoPixel led(LED_COUNT, LED_RGB, NEO_GRB + NEO_KHZ800);

#define LED 2

Servo myservo;
#define servoPin 17

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

float temp, hum, CBND, CBDA, lasttemp, lasthum;

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
      if (written == Update.size()) {
        Serial.println("Update ghi du du lieu.");
        if (Update.end()) {
          if (Update.isFinished()) {
            Serial.println("Update thanh cong. Dang khoi dong lai...");
            display.clearDisplay();
            display.setCursor(0, 10);
            display.print("Update thanh cong");
            display.setCursor(0, 30);
            display.print("Rebooting...");
            display.display();
            delay(1000);
            ESP.restart();
          } else {
            Serial.println("Update chua hoan tat!");
          }
        } else {
          Serial.print("Update failed: ");
          Serial.println(Update.errorString());
        }
      } else {
        Serial.println("Ghi firmware bi thieu du lieu!");
      }
    } else {
      Serial.println("Khong the bat dau OTA!");
    }
    }else {
      Serial.print("Tai firmware that bai. HTTP code: ");
      Serial.println(httpCode);
    }
    http.end();
}

void setup() {
  /*
    Người dùng build code tại đây
  */
  I2C_BME.begin(8,18);
  I2C_OLED.begin(13,12);
  led.begin();
  led.setBrightness(50);
  led.setPixelColor(0, led.Color(255, 0, 255));
  led.show();  
  if (!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address)) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    Serial.println("OLED fail!");
    while (1);
  }
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 1000, 2000);
  display.clearDisplay();
  display.setCursor(25, 30);
  display.print("NUKEDASHBOARD");
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
  lasttemp = bme.readTemperature();
  lasthum = bme.readHumidity();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  delay(1000);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  Serial.begin(115200);
  Serial.println("He thong dang khoi dong...");
  display.display();
  display.clearDisplay();
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
  // Hiển thị thông báo khởi tạo cảm biến
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("BME280 Ready");
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
  if(Firebase.getFloat(fbdo,"/users/luong/bme280/CBNDBME280")) CBND = fbdo.floatData();
  if(Firebase.getFloat(fbdo,"/users/luong/bme280/CBDABME280")) CBDA = fbdo.floatData();

  hum = bme.readHumidity();
  temp = bme.readTemperature();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(20, 0);
  display.print("CAM BIEN BME280");

  Serial.printf("BME280: Nhiet do: %f, Do am: %f\n", temp, hum);

  display.setCursor(0, 20);
  display.printf("ND:%.2f", temp);

  display.setCursor(55, 20);
  display.printf("DA:%.2f", hum);

  display.setCursor(0, 30);
  display.printf("CBND:%.1f", CBND);

  display.setCursor(65, 30);
  display.printf("CBDA:%.1f", CBDA);

  if(temp > CBND || hum > CBDA) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    Firebase.setInt(fbdo,"/users/luong/bme280/ledbme280",1);
    display.setCursor(0, 40);
    display.print("Den canh bao: Bat");
  }
  else {
    led.setPixelColor(0, led.Color(0, 255, 0));
    Firebase.setInt(fbdo,"/users/luong/bme280/ledbme280",0);
    display.setCursor(0, 40);
    display.print("Den canh bao: Tat");
  }

  if(temp != lasttemp) {
    Firebase.setFloat(fbdo,"/users/luong/bme280/Temp",temp);
    lasttemp = temp;
  }

  if(hum != lasthum) {
    Firebase.setFloat(fbdo,"/users/luong/bme280/Humi",hum);
    lasthum = hum;
  }

  led.show();
  display.display();
  delay(2000);
}