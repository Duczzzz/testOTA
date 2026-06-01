// Thêm thư viện cảm biến BME280
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // Đối tượng cảm biến

void setup() {
  /*
    Người dùng build code tại đây
  */
  Wire.begin(8,18);
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
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("He thong dang \nkhhoi dong...");
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
  Serial.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(512, 512);
  Firebase.begin(&config, &auth);
  // Khởi tạo cảm biến BME280
  if (!bme.begin(0x76, &Wire)) {
    Serial.println("Khong the khoi tao BME280!");
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
  } else {
    Serial.println("BME280 khoi tao thanh cong");
    led.setPixelColor(0, led.Color(0, 255, 0));
    led.show();
  }
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
  // Đọc dữ liệu từ BME280
  float temperature = bme.readTemperature(); // độ C
  float pressure = bme.readPressure() / 100.0F; // hPa
  float altitude = bme.readAltitude(1013.25); // m (giá trị áp suất chuẩn)

  // Cập nhật LED RGB dựa trên nhiệt độ
  if (temperature > 35.0) {
    led.setPixelColor(0, led.Color(255, 0, 0)); // Đỏ khi >35°C
  } else {
    led.setPixelColor(0, led.Color(0, 0, 255)); // Xanh dương khi <=35°C
  }
  led.show();

  // Hiển thị lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.printf("Nhiet do: %.1f C", temperature);
  display.setCursor(0,12);
  display.printf("Ap suat: %.1f hPa", pressure);
  display.setCursor(0,24);
  display.printf("Chieu cao: %.1f m", altitude);
  display.display();

  delay(2000); // Độ trễ giữa các lần đọc
}