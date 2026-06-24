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
#include <DHT.h>

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

#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== DATABASE CARD TEST =====
#define PATH_DATA1   "users/luong/Card/Data-6-1"
#define PATH_WARNLED "users/luong/Card/Data-6-Warnled"
#define PATH_CB1     "users/luong/Card/Data-6-CB1"
#define PATH_OTA     "/users/luong/updateOTA"

float nguongCanhBao = 32.0;

void getupdate()
{
    display.setTextColor(SSD1306_WHITE);

    Firebase.setInt(fbdo, PATH_OTA, 0);

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
        display.printf("Size: %d", firmwareSize);
        display.display();

        if (Update.begin(firmwareSize))
        {
            Update.onProgress([](size_t current, size_t total) {
                int percent = (current * 100) / total;

                Serial.printf("OTA %d%%\n", percent);

                display.clearDisplay();
                display.setCursor(0, 0);
                display.print("Updating");

                display.setCursor(0, 20);
                display.print(percent);
                display.print("%");

                display.drawRect(0, 30, 120, 10, SSD1306_WHITE);
                display.fillRect(2, 32, (percent * 116) / 100, 6, SSD1306_WHITE);
                display.display();
            });

            size_t written = Update.writeStream(client);

            display.clearDisplay();

            if (written == Update.size())
            {
                Serial.println("Update ghi du du lieu.");

                if (Update.end())
                {
                    if (Update.isFinished())
                    {
                        Serial.println("Update thanh cong. Dang khoi dong lai...");

                        display.clearDisplay();
                        display.setCursor(0, 10);
                        display.print("Update thanh cong");
                        display.setCursor(0, 30);
                        display.print("Rebooting...");
                        display.display();

                        delay(1000);
                        ESP.restart();
                    }
                    else
                    {
                        Serial.println("Update chua hoan tat!");
                    }
                }
                else
                {
                    Serial.print("Update failed: ");
                    Serial.println(Update.errorString());
                }
            }
            else
            {
                Serial.println("Ghi firmware bi thieu du lieu!");
            }
        }
        else
        {
            Serial.println("Khong the bat dau OTA!");
        }
    }
    else
    {
        Serial.print("Tai firmware that bai. HTTP code: ");
        Serial.println(httpCode);
    }

    http.end();
}

void setup()
{
    I2C_BME.begin(8, 18);
    I2C_OLED.begin(13, 12);

    led.begin();
    led.setBrightness(50);
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();

    if (!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address))
    {
        led.setPixelColor(0, led.Color(255, 0, 0));
        led.show();

        Serial.println("OLED fail!");
        while (1);
    }

    display.clearDisplay();
    display.setCursor(25, 30);
    display.print("NUKEDASHBOARD");

    if (!bme.begin(0x76, &I2C_BME))
    {
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

    pinMode(LED, OUTPUT);
    digitalWrite(LED, 0);

    Serial.begin(115200);
    Serial.println("He thong dang khoi dong...");

    display.display();
    display.clearDisplay();

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        led.setPixelColor(0, led.Color(255, 0, 255));
        led.show();

        Serial.println("dang khoi dong WiFi...");

        display.setCursor(10, 0);
        display.print("Dang ket noi WiFi");

        display.setCursor(0, 20);
        display.printf("SSID: %s", ssid);

        if (demwf < 80)
        {
            display.setCursor(demwf, 30);
            display.print(".");
            Serial.println(".");
        }
        else if (demwf > 80)
        {
            display.clearDisplay();
            demwf = 0;
        }

        demwf += 5;
        display.display();

        digitalWrite(LED, 1);
        delay(300);
    }

    digitalWrite(LED, 0);

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

    dht.begin();
}

void loop()
{
    if (Firebase.getInt(fbdo, PATH_OTA))
    {
        checkupdate = fbdo.intData();
    }

    if (checkupdate == 1)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("UPDATE OTA");
        display.display();

        getupdate();
    }

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Lay gia tri nguong canh bao tu Firebase
    if (Firebase.getFloat(fbdo, PATH_CB1))
    {
        nguongCanhBao = fbdo.floatData();
    }

    // Gui nhiet do len card kenh 1
    Firebase.setFloat(fbdo, PATH_DATA1, temperature);

    if (temperature >= nguongCanhBao)
    {
        led.setPixelColor(0, led.Color(255, 0, 0));

        // Gui gia tri den canh bao = 1
        Firebase.setInt(fbdo, PATH_WARNLED, 1);
    }
    else
    {
        led.setPixelColor(0, led.Color(0, 255, 0));

        // Gui gia tri den canh bao = 0
        Firebase.setInt(fbdo, PATH_WARNLED, 0);
    }

    led.show();

    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("Nhiet do: ");
    display.print(temperature);
    display.print((char)223);
    display.print("C");

    display.setCursor(0, 20);
    display.print("Do am: ");
    display.print(humidity);
    display.print("%");

    display.setCursor(0, 40);
    display.print("Nguong: ");
    display.print(nguongCanhBao);
    display.print((char)223);
    display.print("C");

    display.display();

    delay(2000);
}