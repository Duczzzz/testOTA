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
#define LED 2

Adafruit_NeoPixel led(LED_COUNT, LED_RGB, NEO_GRB + NEO_KHZ800);

TwoWire I2C_BME = TwoWire(0);
TwoWire I2C_OLED = TwoWire(1);

#define i2c_Address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, OLED_RESET);
Adafruit_BME280 bme;

const char* firmwareUrl = "https://raw.githubusercontent.com/Duczzzz/testOTA/main/firmware_luong.ino.bin";

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int checkupdate = 0;
int demwf = 0;

bool ledState = false;
unsigned long lastToggle = 0;

bool oledOK = false;
bool bmeOK = false;

void getupdate() {
  Firebase.setInt(fbdo, "/users/luong/updateOTA", 0);

  Serial.print("Firmware URL: ");
  Serial.println(firmwareUrl);

  HTTPClient http;
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient& client = http.getStream();
    int firmwareSize = http.getSize();

    Serial.print("Firmware Size: ");
    Serial.println(firmwareSize);

    if (oledOK) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.printf("Size: %d", firmwareSize);
      display.display();
    }

    if (Update.begin(firmwareSize)) {
      Update.onProgress([](size_t current, size_t total) {
        int percent = 0;
        if (total > 0) percent = (current * 100) / total;

        Serial.printf("OTA %d%%\n", percent);

        if (oledOK) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);

          display.setCursor(0, 0);
          display.print("Updating");

          display.setCursor(0, 20);
          display.print(percent);
          display.print("%");

          display.drawRect(0, 30, 120, 10, SSD1306_WHITE);
          display.fillRect(2, 32, (percent * 116) / 100, 6, SSD1306_WHITE);

          display.display();
        }
      });

      size_t written = Update.writeStream(client);

      if (Update.size() == written) {
        Serial.println("Update successfully completed. Rebooting...");

        if (oledOK) {
          display.clearDisplay();
          display.setCursor(0, 10);
          display.print("Update completed");
          display.setCursor(0, 30);
          display.print("Rebooting...");
          display.display();
        }

        if (Update.end()) {
          delay(1000);
          ESP.restart();
        } else {
          Serial.print("Update failed: ");
          Serial.println(Update.errorString());
        }
      } else {
        Serial.println("Not enough space or write failed.");
      }
    } else {
      Serial.println("Failed to begin OTA update.");
    }
  } else {
    Serial.print("Failed to download firmware. HTTP code: ");
    Serial.println(httpCode);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("He thong dang khoi dong...");

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  led.begin();
  led.setBrightness(50);
  led.setPixelColor(0, led.Color(255, 0, 255));
  led.show();

  I2C_BME.begin(8, 18);
  I2C_OLED.begin(13, 12);

  oledOK = display.begin(SSD1306_SWITCHCAPVCC, i2c_Address);

  if (!oledOK) {
    Serial.println("OLED fail!");
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 30);
    display.print("NUKEDASHBOARD");
    display.display();
  }

  bmeOK = bme.begin(0x76, &I2C_BME);

  if (!bmeOK) {
    Serial.println("Khong tim thay BME280!");

    if (oledOK) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Khong tim thay BME280");
      display.display();
    }

    led.setPixelColor(0, led.Color(255, 120, 0));
    led.show();

    delay(1000);
  }

  delay(1000);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();

    Serial.println("Dang ket noi WiFi...");

    if (oledOK) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Dang ket noi WiFi");
      display.setCursor(0, 20);
      display.print("SSID: ");
      display.print(ssid);
      display.setCursor(demwf, 40);
      display.print(".");
      display.display();
    }

    demwf += 5;
    if (demwf > 120) demwf = 0;

    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(300);
  }

  Serial.println("WiFi da ket noi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);

  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();

  if (oledOK) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("System Ready");
    display.display();
  }
}

void loop() {
  if (Firebase.getInt(fbdo, "/users/luong/updateOTA")) {
    checkupdate = fbdo.intData();
  }

  if (checkupdate == 1) {
    if (oledOK) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.print("UPDATE OTA");
      display.display();
    }

    getupdate();
    checkupdate = 0;
  }

  if (millis() - lastToggle >= 2000) {
    ledState = !ledState;
    digitalWrite(LED, ledState ? HIGH : LOW);

    if (oledOK) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.print("LED ");
      display.print(ledState ? "ON" : "OFF");
      display.display();
    }

    lastToggle = millis();
  }
}