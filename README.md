# 🔐 IoT Smart Door Security System

An IoT-based smart door security system developed as part of a university project using Arduino Uno, ESP8266, RFID, keypad authentication and a real-time web dashboard.

---

## 📖 Overview

This project combines embedded systems, IoT and web technologies to create a secure smart door access control system. Users can unlock the door using either an authorised RFID card or a keypad PIN. An IR sensor detects visitors, while an ESP8266 hosts a live dashboard displaying system status.

---

## 🏗 System Architecture

The project uses two microcontrollers working together:

- **Arduino Uno** handles the keypad, IR sensor and servo motor.
- **NodeMCU ESP8266** manages RFID authentication, Wi-Fi connectivity, the web dashboard and system monitoring.
- The two devices communicate using digital pulse signalling to exchange authentication and visitor detection events.

---

## ✨ Features

- RFID authentication
- Keypad PIN authentication
- Servo motor door locking mechanism
- IR visitor detection
- ESP8266 Wi-Fi connectivity
- Real-time web dashboard
- Live authentication status monitoring
- Door status monitoring
- Visitor detection notifications
- Failed-attempt alarm system
- LED and buzzer feedback
- Automatic hotspot fallback when Wi-Fi is unavailable
- Local event logging

---

## 🛠 Tech Stack

- Arduino C++
- Arduino Uno
- NodeMCU ESP8266
- RFID RC522
- Servo Motor
- IR Sensor
- 4x4 Keypad
- ESP8266WebServer
- SPI
- MFRC522 Library

---

## 📸 Screenshots

### Hardware Setup

The complete IoT smart door security system, including the Arduino Uno, NodeMCU ESP8266, RFID reader, keypad, servo motor, IR sensor, LEDs and buzzer.

![Hardware Setup](screenshots/Whole%20Setup.jpg)

---

### Pin Connections

Connection diagram showing the wiring between the Arduino Uno, ESP8266, RFID reader, keypad, servo motor, LEDs, buzzer and IR sensor.

![Pin Connections](screenshots/Connection%20Setup.jpg)

---

### Wi-Fi Dashboard

A real-time dashboard hosted on the ESP8266 displaying the system status, authentication method, RFID activity, failed login attempts, visitor detection, door status and alarm state.

![Wi-Fi Dashboard](screenshots/Wifi%20DashBoard%20.jpg)

---

## 📂 Project Structure

```text
iot-smart-door-security-system/
│
├── README.md
├── source/
│   ├── Arduino.ino
│   └── ESP8266.ino
│
└── screenshots/
    ├── Whole Setup.jpg
    ├── Connection Setup.jpg
    └── Wifi DashBoard.jpg
```

---

## 🔌 Hardware Components

- Arduino Uno
- NodeMCU ESP8266
- RC522 RFID Reader
- 4×4 Matrix Keypad
- Servo Motor
- IR Sensor
- Green & Red LEDs
- Buzzer
- Breadboard and Resistors

---

## 🌐 System Workflow

1. Visitor approaches the door.
2. IR sensor detects presence.
3. User authenticates using RFID or keypad PIN.
4. Arduino and ESP8266 exchange signals.
5. Access is granted or denied.
6. Servo unlocks the door when authorised.
7. Dashboard updates with live system status.

---

## ▶️ How to Run

1. Upload `Arduino.ino` to the Arduino Uno.
2. Upload `ESP8266.ino` to the NodeMCU ESP8266.
3. Wire the components according to the connection diagram.
4. Configure Wi-Fi credentials if required.
5. Power both boards and access the dashboard using the ESP8266 IP address.

---

## 📚 Programming Concepts Demonstrated

- Embedded Systems
- IoT Development
- Microcontroller Programming
- RFID Authentication
- Web Server Development
- Hardware Integration
- Event-Driven Programming
- Sensor Integration
- Wi-Fi Networking

---

## 🎓 What I Learned

- Integrating multiple hardware components
- Developing embedded software for Arduino and ESP8266
- Creating an IoT dashboard
- Managing communication between microcontrollers
- Debugging hardware and software interactions

---

## 📌 Notes

This project was developed for educational purposes as part of a university IoT module.
