#define BLYNK_TEMPLATE_ID "TMPL6jKI9jXvY"
#define BLYNK_TEMPLATE_NAME "AirNoiseProject"
#define BLYNK_AUTH_TOKEN "02DFM65auQiH_TDA8z6AuLOHKbIxI-RB"

#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
char ssid[] = "Amer";
char pass[] = "amer654321";
SoftwareSerial EspSerial(2, 3); // RX, TX
#define ESP8266_BAUD 9600
ESP8266 wifi(&EspSerial);

#define SOUND_SENSOR A1
#define GAS_SENSOR A0
#define SOUND_LED 12
#define GAS_LED 10
#define SOUND_THRESHOLD 60
#define GAS_THRESHOLD 180

void sendSensorData() {
  int soundValue = analogRead(SOUND_SENSOR);
  int gasValue = analogRead(GAS_SENSOR);

  if (soundValue < 10 || gasValue < 10) {
    Serial.println("Sensor data low, check connections!");
    return;
  }

  int soundDb = map(soundValue, 0, 1023, 30, 120);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SND:");
  lcd.print(soundDb);
  lcd.print("dB");

  lcd.setCursor(0, 1);
  lcd.print("Gas:");
  lcd.print(gasValue);
  lcd.print("ppm");

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, soundDb);
    Blynk.virtualWrite(V1, gasValue);
  }

  Serial.print("Raw Sound: ");
  Serial.print(soundValue);
  Serial.print(" | Sound Level: ");
  Serial.print(soundDb);
  Serial.print(" dB | Gas Level: ");
  Serial.println(gasValue);

  if (soundDb > SOUND_THRESHOLD) {
    digitalWrite(SOUND_LED, HIGH);
    if (Blynk.connected()) Blynk.virtualWrite(V2, 1);
    lcd.setCursor(9, 0);
    lcd.print(" LOUD");
  } else {
    digitalWrite(SOUND_LED, LOW);
    if (Blynk.connected()) Blynk.virtualWrite(V2, 0);
    lcd.setCursor(9, 0);
    lcd.print("    ");
  }

  if (gasValue > GAS_THRESHOLD) {
    lcd.setCursor(9, 1);
    lcd.print("DANGER!");
    if (Blynk.connected()) Blynk.virtualWrite(V3, 1);
    for (int i = 0; i < 5; i++) {
      digitalWrite(GAS_LED, HIGH);
      delay(300);
      digitalWrite(GAS_LED, LOW);
      delay(300);
    }
  } else {
    lcd.setCursor(9, 1);
    lcd.print("  SAFE");
    digitalWrite(GAS_LED, LOW);
    if (Blynk.connected()) Blynk.virtualWrite(V3, 0);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Serial.println("ESP8266 initialized...");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("LCD initialized...");

  pinMode(SOUND_SENSOR, INPUT);
  pinMode(GAS_SENSOR, INPUT);
  pinMode(SOUND_LED, OUTPUT);
  pinMode(GAS_LED, OUTPUT);
  Serial.println("Pins configured...");

  Serial.println("Configuring Blynk...");
  Blynk.config(wifi, BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  Serial.println("Connecting to WiFi and Blynk...");
  if (Blynk.connectWiFi(ssid, pass)) {
    Serial.println("WiFi connected!");
    if (Blynk.connect(30000)) {
      Serial.println("Connected to Blynk!");
      lcd.setCursor(0, 1);
      lcd.print("Blynk Connected");
    } else {
      Serial.println("Blynk server connect failed...");
      lcd.setCursor(0, 1);
      lcd.print("Blynk Failed");
    }
  } else {
    Serial.println("WiFi connect failed...");
    lcd.setCursor(0, 1);
    lcd.print("WiFi Failed");
  }

  Serial.println("Setup complete!");
}

void loop() {
  if (!Blynk.connected()) {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) { // Try every 5 seconds
      Serial.println("Attempting to reconnect to Blynk...");
      if (Blynk.connectWiFi(ssid, pass) && Blynk.connect(10000)) {
        Serial.println("Reconnected to Blynk!");
      } else {
        Serial.println("Reconnect failed...");
      }
      lastAttempt = millis();
    }
  }
  Blynk.run();
  sendSensorData();
  delay(1000);
}