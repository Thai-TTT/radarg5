#define BLYNK_TEMPLATE_ID "TMPL6efk3zTCm"
#define BLYNK_TEMPLATE_NAME "rada"
#define BLYNK_AUTH_TOKEN "Q_THZMHDVEZiEXUewTHNI8jNlVx_2G2I"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi & Blynk
char auth[] = BLYNK_AUTH_TOKEN;  // Thay bằng Auth Token từ Blynk
char ssid[] = "N5";         // Tên WiFi
char pass[] = "123456789";     // Mật khẩu WiFi

// Khai báo chân linh kiện
#define TRIG_PIN D6
#define ECHO_PIN D7
#define BUZZER_PIN D5
#define SERVO_PIN D4

Servo servo;//scl D1 - Sda D2
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Serial.begin(115200);
    // Kết nối WiFi với Blynk
    Blynk.begin(auth, ssid, pass);

    // Khởi động LCD
    lcd.begin();
    lcd.backlight();

    // Khởi động cảm biến và buzzer
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);// t 
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    // Khởi động I2C (SDA = D2, SCL = D1)
    // Wire.begin(D2, D1);
    // Khởi động servo
    servo.attach(SERVO_PIN);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP8266 Radar");
}

long getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    return pulseIn(ECHO_PIN, HIGH, 30000) * 0.034 / 2;  // Tính toán khoảng cách
}

void scanAndDisplay(bool forward) {
    int startAngle = forward ? 1 : 180;
    int endAngle = forward ? 180 : 1;
    int step = forward ? 12 : -12;
    
    lcd.clear();
    bool objectDetected = false;
    
    for (int angle = startAngle; (forward ? angle <= endAngle : angle >= endAngle); angle += step) {
        servo.write(angle);
        delay(800);
        long distance = getDistance();
        int col = map(angle, 1, 180, 0, 15);

        // Hiển thị trên LCD
        if (distance > 10) {
            lcd.setCursor(col, 0);
            lcd.print(".");
        } else {
            lcd.setCursor(col, 1);
            lcd.print("*");
            objectDetected = true;
        }

        // Gửi dữ liệu lên Blynk
        Blynk.virtualWrite(V0, angle);    // Gửi góc quét
        Blynk.virtualWrite(V1, distance); // Gửi khoảng cách

        Serial.print("Angle: ");
        Serial.print(angle);
        Serial.print(" | Distance: ");
        Serial.println(distance);
    }

    // Buzzer cảnh báo nếu có vật cản gần
    if (objectDetected) {
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void loop() {
    Blynk.run();  // Luôn chạy Blynk để cập nhật dữ liệu
    scanAndDisplay(true);
    delay(500);
    scanAndDisplay(false);
    delay(500);
}
