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

const char* ssid = "DUC";
const char* pass = "14042004";
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

const char* firmwareUrl = "https://raw.githubusercontent.com/Duczzzz/testOTA/main/firmware_duc.ino.bin";

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int checkupdate = 0;
int demwf = 0;
void getupdate()
{
    display.setTextColor(SSD1306_WHITE);
    Firebase.setInt(fbdo, "/users/duc/updateOTA",0);
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
              display.drawRect(0, 30, 100, 10, SSD1306_WHITE);
              display.fillRect(2, 32, (percent * 96) / 100, 6, SSD1306_WHITE);
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
    } else {
      Serial.print("Tai firmware that bai. HTTP code: ");
      Serial.println(httpCode);
    }
    http.end();
}

void setup() {
    /* Người dùng build code tại đây */
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
        } else if(demwf > 80) {
            display.clearDisplay();
            demwf = 0;
        }
        demwf+=5;
        display.display();
        digitalWrite(0,1);
        delay(300);
    }
    digitalWrite(0,0);
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
    // Đọc giá trị ban đầu và hiển thị
    float tempInit = bme.readTemperature(); 
    float humInit = bme.readHumidity(); 
    if (tempInit > 32) { led.setPixelColor(0, led.Color(255,0,0)); } else { led.setPixelColor(0, led.Color(0,255,0)); } 
    led.show(); 
    display.clearDisplay(); 
    display.setCursor(0,0); 
    display.print("Temp: "); 
    display.print(temperature); 
    display.print(" C"); 
    display.setCursor(0,10); 
    display.print("Hum: "); 
    display.print(humidity); 
    display.print(" %"); 
    display.display();
}

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
    /* Xây dựng cơ chế xử lý của bạn tại đây */
    float temperature = bme.readTemperature(); 
    float humidity = bme.readHumidity(); 
    if (temperature > 32) { led.setPixelColor(0, led.Color(255,0,0)); } else { led.setPixelColor(0, led.Color(0,255,0)); } 
    led.show(); 
    display.clearDisplay(); 
    display.setCursor(0,0); 
    display.print("Temp: "); 
    display.print(temperature); 
    display.print(" C"); 
    display.setCursor(0,10); 
    display.print("Hum: "); 
    display.print(humidity); 
    display.print(" %"); 
    display.display();
    delay(1000);
}