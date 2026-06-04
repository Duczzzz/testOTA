#include <Wire.h>
#include "DHT.h"
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "........";
const char* pass = "........";

#define LED_PIN   9
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LED 2

// #define i2c_Address 0x3c
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET -1
// Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define DHTTYPE DHT11
#define DHTPIN 11
DHT dht(DHTPIN, DHTTYPE);

float temp,hum,CBND,CBDA,lastemp,lasthum;
int demwf = 0;

const char* firmwareUrl = "https://raw.githubusercontent.com/Duczzzz/testOTA/main/firmware.ino.bin";
int checkupdate = 0;

void getupdate()
{
    Serial.print("Starting OTA from: ");
    Serial.println(firmwareUrl);
    HTTPClient http;
    http.begin(firmwareUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        WiFiClient& client = http.getStream();
        int firmwareSize = http.getSize();
        Serial.print("Firmware size: ");
        Serial.println(firmwareSize);

        if (Update.begin(firmwareSize))
        {
            Update.onProgress([](size_t current, size_t total) {
                int percent = (current * 100) / total;
                Serial.printf("OTA Progress: %d%%\n", percent);
                led.setPixelColor(0, led.Color(0, 0, 255));
                led.show();
            });

            size_t written = Update.writeStream(client);
            if (Update.size() == written)
            {
                Serial.println("OTA written successfully, finalizing...");
                if (Update.end())
                {
                    Serial.println("OTA update successful, rebooting...");
                    Firebase.setInt(fbdo, "/updateOTA", 0);
                    ESP.restart();
                }
                else
                {
                    Serial.print("OTA end failed: ");
                    Serial.println(Update.errorString());
                }
            }
            else
            {
                Serial.println("OTA write mismatch, not enough space.");
            }
        }
        else
        {
            Serial.println("Failed to start OTA Update.");
        }
    }
    else
    {
        Serial.print("Failed to download OTA, HTTP code: ");
        Serial.println(httpCode);
    }
    http.end();
    led.setPixelColor(0, led.Color(0, 255, 0));
    led.show();
}

void setup() {
  // Wire.begin(12,13);
  // display.begin(i2c_Address, true);
  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(SH110X_WHITE);
  // display.setCursor(0, 0);
  // display.printf("He thong dang \nkhoi dong...");
  // display.display();
  // delay(1000);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  Serial.begin(115200);
  Serial.println("He thong dang khoi dong...");
  dht.begin();
  lasthum = dht.readHumidity();
  lastemp = dht.readTemperature();
  led.begin();
  led.setBrightness(50);
  led.setPixelColor(0, led.Color(255, 0, 255));
  led.show();
  // display.clearDisplay();
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("dang khoi dong WiFi...");
    // display.setCursor(0,0);
    // display.print("Conecting WiFi");
    if(demwf < 80) {
      // display.setCursor(demwf,10);
      // display.print(".");
      // Serial0.println(".");
    }
    else if(demwf > 80) {
      // display.clearDisplay();
      demwf = 0;
    }
    demwf+=5;
    // display.display();
    digitalWrite(LED,1);
    delay(300);
  }
  digitalWrite(LED,0);
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);
  Firebase.beginStream(fbdo, "/users/luong/dht11");
  Firebase.setFloat(fbdo,"/users/luong/dht11/Temp",lastemp);
  Firebase.setFloat(fbdo,"/users/luong/dht11/Humi",lasthum);  
  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(SH110X_WHITE);
  // display.setCursor(0, 30);
  // display.println("XIN CHAO CAC BAN");
  // display.display();
  // delay(300);
  // display.clearDisplay();
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
}

void loop() {
  if(Firebase.getInt(fbdo,"/updateOTA")) checkupdate = fbdo.intData();
  if(checkupdate == 1) {
    getupdate();
    checkupdate = 0;
  }

  // if (!Firebase.readStream(fbdo)) {
  //   Serial.println(fbdo.errorReason());
  //   return;
  // }
  // if (fbdo.streamAvailable()) {
  //   String path = fbdo.dataPath();

  //   if (path == "/users/duc/dht11/CBNDDht11") {
  //     CBND = fbdo.floatData();
  //     Serial.printf("CBND update: %.2f\n", CBND);
  //   }
  //   if (path == "/users/duc/dht11/CBDADht11") {
  //     CBDA = fbdo.floatData();
  //     Serial.printf("CBND update: %.2f\n", CBDA);
  //   }
  // }
  if(Firebase.getFloat(fbdo,"/users/luong/dht11/CBNDDht11")) CBND = fbdo.floatData();
  if(Firebase.getFloat(fbdo,"/users/luong/dht11/CBDADht11")) CBDA = fbdo.floatData();
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  Serial.printf("DHT11: Nhiệt độ: %f, Độ ẩm: %f\n",temp,hum);
  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setCursor(20, 0);
  // display.print("CAM BIEN DHT11");
  // display.setCursor(0, 10);
  // display.printf("Nhiet do: %.2f",temp);
  // display.setTextSize(1);
  // display.setCursor(0, 20);
  // display.printf("Do am: %.2f",hum);
  // display.setCursor(0, 30);
  // display.print("CANH BAO: ");
  // display.println(CBND);
  if(temp > CBND || hum > CBDA) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    Firebase.setInt(fbdo,"/users/luong/dht11/leddht11",1);
    // display.setTextSize(1);
    // display.setCursor(0, 40);
    // display.print("Da bat den canh bao");
  }
  else {
    led.setPixelColor(0, led.Color(0, 255, 0));
    led.show();
    Firebase.setInt(fbdo,"/users/luong/dht11/leddht11",0);
    // display.setTextSize(1);
    // display.setCursor(0, 40);
    // display.print("Da tat den canh bao");
  }
  if(temp != lastemp) {
    lastemp = temp;
    Firebase.setFloat(fbdo,"/users/luong/dht11/Temp",temp);
  }
  if(hum != lasthum) {
    lasthum = hum;
    Firebase.setFloat(fbdo,"/users/luong/dht11/Humi",hum);  
  }
  delay(1000);
}
