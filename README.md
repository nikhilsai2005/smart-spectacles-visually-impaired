# smart-spectacles-visually-impaired
IoT wearable fall detection and  obstacle alert system for visually impaired people


> A wearable IoT-based assistive device designed to enhance the safety, mobility, and independence of visually impaired and elderly individuals through real-time obstacle detection, automated fall detection, GPS location tracking, and instant Telegram emergency alerts.

---

## 📌 Table of Contents
- [Overview](#-overview)
- [Features](#-features)
- [Hardware Components](#-hardware-components)
- [Pin Connections](#-pin-connections)
- [System Architecture](#-system-architecture)
- [How It Works](#-how-it-works)
- [Libraries Required](#-libraries-required)
- [Setup Instructions](#-setup-instructions)
- [Telegram Bot Setup](#-telegram-bot-setup)
- [Results](#-results)
- [Project Structure](#-project-structure)
- [College Details](#-college-details)


---

## 📖 Overview

Smart Spectacles for Visually Impaired People is an innovative wearable IoT device mounted on a spectacle frame that provides dual functionality — obstacle detection through proximity-based buzzer feedback and fall detection with automated emergency notification. The system integrates a VL53L0X Time-of-Flight sensor, an ADXL345 accelerometer, a NEO-6M GPS module, and an ESP32 microcontroller with built-in WiFi.

Upon detecting a fall, the system automatically fetches GPS coordinates and sends an instant emergency alert via the Telegram Bot API containing a clickable Google Maps location link to the registered caregiver's phone — all within 2.4 seconds of fall detection.

Unlike conventional GSM-based systems that require a SIM card and monthly recharge costs, this system operates entirely over WiFi at **zero recurring monthly cost**.

---

## ✨ Features

- ✅ **3-Zone Obstacle Detection** — Slow beep at 400mm, fast beep at 200mm, continuous alarm at 100mm
- ✅ **Automated Fall Detection** — ADXL345 delta acceleration thresholding at 25.0 m/s²
- ✅ **Instant Telegram Alert** — Average delivery time of 2.4 seconds after fall detection
- ✅ **Live GPS Location** — Google Maps link embedded in every emergency alert
- ✅ **Zero Monthly Cost** — WiFi + Telegram Bot API replaces expensive GSM module
- ✅ **Non-Blocking Firmware** — millis()-based cooperative multitasking architecture
- ✅ **6 Hours Battery Life** — Runs on standard 5V USB power bank
- ✅ **Auto WiFi Reconnect** — Resilient to temporary network disconnection
- ✅ **30-Second Cooldown** — Prevents duplicate alerts for a single fall event
- ✅ **Multi-Recipient Support** — Send alerts to Telegram group for multiple caregivers

---

## 🔧 Hardware Components

| Component | Model | Purpose |
|---|---|---|
| Microcontroller | ESP32-32 | Central processing + WiFi |
| ToF Sensor | VL53L0X | Real-time obstacle detection |
| Accelerometer | ADXL345 | Fall detection |
| GPS Module | NEO-6M / NEO-7M | Live location tracking |
| Buzzer | Active Buzzer 5V | Audio proximity alerts |
| Power Supply | USB Power Bank 5V | Portable wearable power |

---

## 📐 Pin Connections

| Module | Pin | ESP32 GPIO | Notes |
|---|---|---|---|
| VL53L0X | SDA | GPIO 21 | Shared I2C bus |
| VL53L0X | SCL | GPIO 22 | Shared I2C bus |
| VL53L0X | VCC | 3.3V | — |
| VL53L0X | GND | GND | — |
| ADXL345 | SDA | GPIO 21 | Shared I2C bus |
| ADXL345 | SCL | GPIO 22 | Shared I2C bus |
| ADXL345 | CS | 3.3V | ⚠️ Critical for I2C mode |
| ADXL345 | SDO | GND | ⚠️ Sets I2C address 0x53 |
| ADXL345 | VCC | 3.3V | — |
| ADXL345 | GND | GND | — |
| GPS NEO-6M | TX | GPIO 16 (RX2) | UART2 |
| GPS NEO-6M | RX | GPIO 17 (TX2) | UART2 |
| GPS NEO-6M | VCC | 3.3V | — |
| GPS NEO-6M | GND | GND | — |
| Active Buzzer | Negative (-) | GPIO 25 | Active-LOW |
| Active Buzzer | Positive (+) | VIN (5V) | — |

---

## 🏗️ System Architecture

```
Power Bank (5V USB)
        │
        ▼
ESP32-WROOM-32 (Central Processing Unit)
   │           │              │           │
   │ I2C       │ I2C          │ UART2     │ GPIO 25
   ▼           ▼              ▼           ▼
VL53L0X    ADXL345        NEO-6M GPS   Active Buzzer
ToF Sensor Accelerometer  Location     3-Zone Audio
   │           │              │
Obstacle    Fall           Maps Link
Detection   Detection      in Alert
                │
                ▼
           ESP32 WiFi
                │
                ▼
        Telegram Bot API
                │
                ▼
        Caregiver Phone
     (Push Notification +
      Google Maps Link)
```

---

## ⚡ How It Works

### Obstacle Detection
```
VL53L0X measures distance every 50ms (non-blocking)
   > 400mm  →  Silent (safe zone)
  ≤ 400mm  →  Slow beep 500ms (caution zone)
  ≤ 200mm  →  Fast beep 150ms (warning zone)
  ≤ 100mm  →  Continuous tone (critical zone)
```

### Fall Detection
```
ADXL345 reads acceleration at 100Hz
δ = |ax−ax'| + |ay−ay'| + |az−az'|
δ > 25.0 m/s²  →  Fall detected!
   ↓
GPS coordinates fetched
   ↓
Telegram alert sent via HTTP GET
   ↓
Caregiver receives push notification
Average latency: 2.4 seconds
```

### Alert Message Format
```
🚨 FALL ALERT!
A fall has been detected!

📍 Location:
https://maps.google.com/?q=17.385044,78.486671
```

---

## 📚 Libraries Required

Install these in Arduino IDE via **Tools → Manage Libraries**:

| Library | Author | Purpose |
|---|---|---|
| Adafruit VL53L0X | Adafruit | ToF sensor driver |
| Adafruit ADXL345 | Adafruit | Accelerometer driver |
| Adafruit Unified Sensor | Adafruit | Base sensor layer |
| TinyGPSPlus | Mikal Hart | GPS NMEA parser |
| WiFi | Built-in ESP32 | WiFi connectivity |
| HTTPClient | Built-in ESP32 | HTTP GET requests |
| Wire | Built-in ESP32 | I2C communication |

---

## 🛠️ Setup Instructions

### Step 1 — Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software)

### Step 2 — Add ESP32 Board Support
```
File → Preferences → Additional Board Manager URLs
Add: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
Tools → Board → Board Manager → Search ESP32 → Install
```

### Step 3 — Install Libraries
```
Tools → Manage Libraries
Search and install all libraries listed above
```

### Step 4 — Edit Credentials in Code
Open `CODE/smart_spectacles.ino` and edit:
```cpp
#define WIFI_SSID        "YourWiFiName"
#define WIFI_PASSWORD    "YourWiFiPassword"
#define BOT_TOKEN        "YourTelegramBotToken"
#define CHAT_ID          "YourTelegramChatID"
```

### Step 5 — Upload Code
```
Connect ESP32 via USB
Tools → Board → ESP32 Dev Module
Tools → Port → Select correct COM port
Click Upload
```

### Step 6 — Open Serial Monitor
```
Tools → Serial Monitor
Set baud rate to 115200
Press RESET button on ESP32
```

---

## 📱 Telegram Bot Setup

### Get Bot Token
```
1. Open Telegram
2. Search @BotFather
3. Send: /newbot
4. Follow instructions
5. Copy the bot token received
```

### Get Chat ID
```
1. Search your bot in Telegram
2. Press START and send: hello
3. Open in browser:
   https://api.telegram.org/bot<YOUR_TOKEN>/getUpdates
4. Find "id" number — that is your Chat ID
```

---

## 📊 Results

| Parameter | Target | Achieved | Status |
|---|---|---|---|
| Obstacle detection accuracy | 100% | 100% (80/80 trials) | ✅ Pass |
| Fall detection sensitivity | ≥ 90% | 93.3% (28/30 trials) | ✅ Pass |
| False positive rate | < 10% | 3.3% | ✅ Pass |
| Alert delivery time | < 5 sec | 2.4 sec avg | ✅ Pass |
| GPS accuracy | < 10m | 3–5 metres | ✅ Pass |
| Battery life | ≥ 4 hrs | 6 hours | ✅ Pass |
| Monthly operating cost | Zero | Zero | ✅ Pass |

---



---

## 🎓 College Details

| | |
|---|---|
| **Project Title** | Smart Spectacles for Visually Impaired People |
| **College** | Methodist College of Engineering and Technology |
| **Department** | Electronics and Communication Engineering |
| **Location** | King Koti, Abids, Hyderabad — 500001 |
| **University** | Osmania University |
| **Academic Year** | 2025–26 |
| **Student** | Nikhil Sai Boddu |

---

\
---


## 🔗 Connect

**Nikhil Sai Boddu**
email : boddu.nikhilsai@gmail.com
Methodist College of Engineering and Technology
Department of ECE, Hyderabad, India

---

*"Technology that saves lives — Smart Spectacles for Visually Impaired People"*
