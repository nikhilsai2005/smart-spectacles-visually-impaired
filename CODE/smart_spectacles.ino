// ============================================================
//  Smart Obstacle & Fall Detection System
//  ESP32 + VL53L0X + ADXL345 + GPS + Telegram Alert
// ============================================================

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ============================================================
//  EDIT ONLY WIFI DETAILS
// ============================================================
#define WIFI_SSID          "YourHotspotName"
#define WIFI_PASSWORD      "YourHotspotPassword"

// ============================================================
//  TELEGRAM — ALREADY FILLED IN
// ============================================================
#define BOT_TOKEN          "8603470983:AAHWmAby7q5dF4ZWcWPI8aj7g2UyeoM1Na8"
#define CHAT_ID            "7658370149"

// ============================================================
//  PIN DEFINITIONS
// ============================================================
#define BUZZER_PIN         25
#define GPS_RX_PIN         16
#define GPS_TX_PIN         17
#define I2C_SDA            21
#define I2C_SCL            22

// ============================================================
//  CONFIGURATION
// ============================================================
#define FALL_THRESHOLD     25.0f
#define ALERT_COOLDOWN_MS  30000
#define GPS_MAX_AGE_MS     10000
#define GPS_BAUD           9600
#define DIST_CRITICAL      100
#define DIST_CLOSE         200
#define DIST_WARN          400
#define BEEP_FAST_MS       150
#define BEEP_SLOW_MS       500

// ============================================================
//  OBJECTS
// ============================================================
Adafruit_VL53L0X         tof;
Adafruit_ADXL345_Unified accel(12345);
TinyGPSPlus              gps;
HardwareSerial           gpsSerial(2);

// ============================================================
//  STATE VARIABLES
// ============================================================
bool          tofOK          = false;
bool          adxlOK         = false;
bool          wifiOK         = false;
float         axPrev         = 0.0f;
float         ayPrev         = 0.0f;
float         azPrev         = 0.0f;
unsigned long lastAlertTime  = 0;
unsigned long lastBeepToggle = 0;
bool          buzzerState    = false;

enum BuzzerMode { BUZ_OFF, BUZ_CONTINUOUS, BUZ_FAST, BUZ_SLOW };
BuzzerMode buzzerMode = BUZ_OFF;

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println("============================================");
  Serial.println("  Smart Obstacle & Fall Detection System  ");
  Serial.println("  Telegram Alert Version                  ");
  Serial.println("============================================");

  pinMode(BUZZER_PIN, OUTPUT);
  setBuzzer(false);

  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);

  // VL53L0X
  Serial.print("[ToF]   VL53L0X init... ");
  if (!tof.begin()) {
    Serial.println("FAILED - check VCC=3.3V SDA=21 SCL=22");
    tofOK = false;
  } else {
    tof.startRangeContinuous(50);
    tofOK = true;
    Serial.println("OK");
  }

  // ADXL345
  Serial.print("[ACCEL] ADXL345 init... ");
  if (!accel.begin()) {
    Serial.println("FAILED - check CS=3.3V SDO=GND");
    adxlOK = false;
  } else {
    accel.setRange(ADXL345_RANGE_16_G);
    accel.setDataRate(ADXL345_DATARATE_100_HZ);
    adxlOK = true;
    Serial.println("OK");
  }

  // GPS
  Serial.print("[GPS]   NEO-6M init... ");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("OK - acquiring fix...");

  // WiFi
  Serial.print("[WiFi]  Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print("... ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiOK = true;
    Serial.println(" Connected!");
    Serial.print("[WiFi]  IP: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiOK = false;
    Serial.println(" FAILED - check hotspot name and password!");
  }

  Serial.println("\n--- Init Summary ---");
  Serial.print("  VL53L0X  : "); Serial.println(tofOK  ? "OK" : "FAILED");
  Serial.print("  ADXL345  : "); Serial.println(adxlOK ? "OK" : "FAILED");
  Serial.println("  GPS      : OK - waiting for fix");
  Serial.print("  WiFi     : "); Serial.println(wifiOK ? "Connected" : "FAILED");
  Serial.println("--------------------");

  // Boot double beep
  setBuzzer(true);  delay(100);
  setBuzzer(false); delay(100);
  setBuzzer(true);  delay(100);
  setBuzzer(false);

  Serial.println("\n=== System Running ===\n");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  readGPS();
  if (tofOK)  handleToF();
  if (adxlOK) handleMotion();
  handleBuzzer();

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(3000);
  }
}

// ============================================================
//  GPS
// ============================================================
void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

// ============================================================
//  ToF
// ============================================================
void handleToF() {
  if (!tof.isRangeComplete()) return;

  uint16_t distance = tof.readRangeResult();
  uint8_t  status   = tof.readRangeStatus();

  if (status != 0) { buzzerMode = BUZ_OFF; return; }

  Serial.print("[ToF]   Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  if (distance <= DIST_CRITICAL) {
    if (buzzerMode != BUZ_CONTINUOUS)
      Serial.println("[ToF]   CRITICAL!");
    buzzerMode = BUZ_CONTINUOUS;
  } else if (distance <= DIST_CLOSE) {
    if (buzzerMode != BUZ_FAST)
      Serial.println("[ToF]   WARNING!");
    buzzerMode = BUZ_FAST;
  } else if (distance <= DIST_WARN) {
    if (buzzerMode != BUZ_SLOW)
      Serial.println("[ToF]   CAUTION!");
    buzzerMode = BUZ_SLOW;
  } else {
    buzzerMode = BUZ_OFF;
  }
}

// ============================================================
//  Buzzer
// ============================================================
void handleBuzzer() {
  unsigned long now = millis();
  switch (buzzerMode) {
    case BUZ_CONTINUOUS: setBuzzer(true);  break;
    case BUZ_OFF:        setBuzzer(false); break;
    case BUZ_FAST:
      if (now - lastBeepToggle >= BEEP_FAST_MS) {
        buzzerState = !buzzerState;
        setBuzzer(buzzerState);
        lastBeepToggle = now;
      }
      break;
    case BUZ_SLOW:
      if (now - lastBeepToggle >= BEEP_SLOW_MS) {
        buzzerState = !buzzerState;
        setBuzzer(buzzerState);
        lastBeepToggle = now;
      }
      break;
  }
}

void setBuzzer(bool on) {
  digitalWrite(BUZZER_PIN, on ? LOW : HIGH);
}

// ============================================================
//  Motion - Fall detection
// ============================================================
void handleMotion() {
  sensors_event_t event;
  accel.getEvent(&event);

  float ax = event.acceleration.x;
  float ay = event.acceleration.y;
  float az = event.acceleration.z;

  float delta = abs(ax-axPrev) + abs(ay-ayPrev) + abs(az-azPrev);
  axPrev = ax; ayPrev = ay; azPrev = az;

  if (delta > FALL_THRESHOLD) {
    Serial.print("[ACCEL] Fall detected! Delta = ");
    Serial.print(delta, 2);
    Serial.println(" m/s2");

    unsigned long now = millis();
    if (now - lastAlertTime >= ALERT_COOLDOWN_MS) {
      if (wifiOK) {
        sendTelegram();
        lastAlertTime = now;
      } else {
        Serial.println("[WiFi]  No WiFi - alert skipped!");
      }
    } else {
      unsigned long rem = (ALERT_COOLDOWN_MS-(now-lastAlertTime))/1000;
      Serial.print("[Alert] Cooldown - ");
      Serial.print(rem);
      Serial.println("s left");
    }
  }
}

// ============================================================
//  GPS Location
// ============================================================
String buildLocationString() {
  if (gps.location.isValid() && gps.location.age() < GPS_MAX_AGE_MS) {
    Serial.println("[GPS]   Valid fix!");
    return "https://maps.google.com/?q="
           + String(gps.location.lat(), 6)
           + "," + String(gps.location.lng(), 6);
  } else if (gps.location.isValid()) {
    Serial.println("[GPS]   Last known location");
    return "Last known: https://maps.google.com/?q="
           + String(gps.location.lat(), 6)
           + "," + String(gps.location.lng(), 6);
  } else {
    Serial.println("[GPS]   No fix");
    return "GPS not available - go outdoors";
  }
}

// ============================================================
//  Telegram Alert
// ============================================================
void sendTelegram() {
  String location = buildLocationString();
  String message  = "FALL ALERT!\n";
  message        += "A fall has been detected!\n\n";
  message        += "Location:\n";
  message        += location;

  Serial.println("[WiFi]  Sending Telegram alert...");

  // URL encode
  message.replace(" ", "%20");
  message.replace("\n", "%0A");
  message.replace("!", "%21");
  message.replace(":", "%3A");
  message.replace("/", "%2F");
  message.replace("?", "%3F");
  message.replace("=", "%3D");
  message.replace("&", "%26");
  message.replace(",", "%2C");
  message.replace("+", "%2B");

  String url  = "https://api.telegram.org/bot";
  url        += BOT_TOKEN;
  url        += "/sendMessage?chat_id=";
  url        += CHAT_ID;
  url        += "&text=";
  url        += message;

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  int httpCode = http.GET();

  if (httpCode == 200) {
    Serial.println("[WiFi]  Telegram alert sent successfully!");
  } else {
    Serial.print("[WiFi]  Failed - code: ");
    Serial.println(httpCode);
  }

  http.end();
}
