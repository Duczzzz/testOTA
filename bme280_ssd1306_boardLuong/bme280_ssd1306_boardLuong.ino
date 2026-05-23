// Đây là source test các nút bấm trên board do Nuke Dashboard phát triển
// Để có thể sử dụng source code này bạn cần cài danh sách các thư viện sau: 
// + thư viện Adafruit NeoPixel by Adafruit
// + thư viện Adafruit BME280 libraray by Adafruit
// + thư viện Firebase ESP32 Client by Mobizt
// + thư viện Adafruit GFX libraray by Adafruit
// + thư viện Adafruit SH110X by Adafruit
// Tác giả MinhDuc
// 07/03/2026
// Led RGB được cấu hình chân DIN ở GPIO9
// Chân SDA BME280 kết nối với GPIO8
// Chân SCL BME280 kết nối với GPIO18

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
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

#define DATABASE_URL "https://doantn-885dc-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "rPb2lv5DjHze997hD9pxnzTzWJsir4wwdP1poStt"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Adafruit_BME280 bme;

float temp,hum,CBND,CBDA,lastemp,lasthum;
int demwf = 0;

void setup() {
  Wire.begin(8,18); // Nếu bạn sử dụng ESP32 Dev Kit Module thì hãy comment dòng code này lại
  led.begin();
  led.setBrightness(50);
  led.setPixelColor(0, led.Color(255, 0, 255));
  led.show();  
  if (!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address)) {
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
  Serial.begin(115200);
  Serial.println("He thong dang khoi dong...");
  display.display();
  if (!bme.begin(0x76)) {
    display.clearDisplay();
    Serial.println("Không tìm thấy BME280!");
    display.setCursor(0, 0);
    display.printf("Khong tim thay BME280!");
    display.display();
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    while (1);
  }
  lasthum = bme.readHumidity();
  lastemp = bme.readTemperature();
  display.clearDisplay();
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED) {
    led.setPixelColor(0, led.Color(255, 0, 255));
    led.show();
    Serial.println("dang khoi dong WiFi...");
    display.setCursor(0,0);
    display.print("Conecting WiFi");
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
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);
  Firebase.setFloat(fbdo,"/users/duc/bme280/Temp",lastemp);
  Firebase.setFloat(fbdo,"/users/duc/bme280/Humi",lasthum);  
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

void loop() {
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
  display.clearDisplay();
  if(Firebase.getFloat(fbdo,"/users/duc/bme280/CBNDBME280")) CBND = fbdo.floatData();
  if(Firebase.getFloat(fbdo,"/users/duc/bme280/CBDABME280")) CBDA = fbdo.floatData();
  hum = bme.readHumidity();
  temp = bme.readTemperature();
  // Serial.printf("BME280: Nhiệt độ: %f, Độ ẩm: %f\n",temp,hum);
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.print("CAM BIEN BME280");
  display.setCursor(0, 15);
  display.printf("Nhiet do: %.2f",temp);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.printf("Do am: %.2f",hum);
  display.setCursor(0, 35);
  display.printf("CANH BAO ND: %.2f",CBND);
  display.setCursor(0, 45);
  display.printf("CANH BAO DA: %.2f",CBDA);
  if(temp > CBND || hum > CBDA) {
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    Firebase.setInt(fbdo,"/users/duc/bme280/ledbme280",1);
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("Da bat den canh bao");
  }
  else {
    led.setPixelColor(0, led.Color(0, 255, 0));
    led.show();
    Firebase.setInt(fbdo,"/users/duc/bme280/ledbme280",0);
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("Da tat den canh bao");
  }
  if(temp != lastemp) {
    lastemp = temp;
    Firebase.setFloat(fbdo,"/users/duc/bme280/Temp",temp);
  }
  if(hum != lasthum) {
    lasthum = hum;
    Firebase.setFloat(fbdo,"/users/duc/bme280/Humi",hum);
  }
  display.display();
  delay(1000);
}