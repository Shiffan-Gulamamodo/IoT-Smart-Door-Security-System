#include <ESP8266WiFi.h>        // Wi-Fi control library for ESP8266
#include <ESP8266WebServer.h>   // Web server library for ESP8266
#include <SPI.h>                // SPI communication library
#include <MFRC522.h>            // RFID reader library

// -------------------- PIN SETUP --------------------
#define SS_PIN D8               // RFID SDA pin
#define RST_PIN D0              // RFID reset pin
#define GREEN_LED D1            // Green LED for access granted (also triggers Arduino servo)
#define BUZZER D2               // Buzzer pin
#define RED_LED D4              // Red LED for access denied/alarm
#define KEYPAD_SIGNAL D3        // Signal from Arduino (keypad/IR pulses)

// -------------------- WIFI SETTINGS --------------------
const char* ssid = "YOUR_WIFI_SSID";            // Your Wi-Fi SSID
const char* password = "YOUR_WIFI_PASSWORD";   // Wi-Fi password
const char* apSsid = "SmartDoorDashboard"; // Hotspot SSID if Wi-Fi fails
const char* apPassword = "YOUR_AP_PASSWORD";       // Hotspot password

// -------------------- OBJECTS --------------------
ESP8266WebServer server(80);      // Web server on port 80
MFRC522 rfid(SS_PIN, RST_PIN);    // Create RFID object

// -------------------- ACCESS SETTINGS --------------------
String allowedUID = "BC480F06";   // The authorized RFID card
int failedAttempts = 0;           // Counter for wrong attempts
const int maxAttempts = 3;        // Max allowed failed attempts

// -------------------- ARDUINO PULSE VARIABLES --------------------
int pulseCount = 0;               // Counts pulses from Arduino (keypad/IR)
unsigned long lastPulseTime = 0;  // Timestamp of last pulse
bool lastSignalState = HIGH;      // Track last signal state
unsigned long ignorePulsesUntil = 0; // Ignore pulses temporarily after action

// -------------------- DASHBOARD VARIABLES --------------------
String systemStatus = "System ready";
String lastMethod = "None";        // Method of access (RFID, keypad, IR)
String lastUID = "None";           // Last scanned UID
String doorStatus = "Closed";      // Door state
String visitorStatus = "No visitor detected"; // Visitor detection
String alarmStatus = "Off";        // Alarm state
String wifiStatusText = "Starting";

// -------------------- MESSAGE LOG --------------------
const int maxMessages = 20;        // Max number of messages to store
String messages[maxMessages];      // Array to store messages
int messageCount = 0;              // Current number of messages

// -------------------- BUZZER FUNCTION --------------------
void beep(int frequency, int duration) {
  tone(BUZZER, frequency); // Start buzzer
  delay(duration);
  noTone(BUZZER);          // Stop buzzer
}

// -------------------- RESET RFID --------------------
void resetRFID() {
  rfid.PCD_Init();           // Initialize RFID
  delay(50);
  rfid.PCD_AntennaOn();      // Turn on antenna
  rfid.PCD_SetAntennaGain(MFRC522::RxGain_max);
}

// -------------------- ADD MESSAGE --------------------
void addMessage(String msg) {
  // Keep the latest 'maxMessages' entries
  if (messageCount >= maxMessages) {
    for (int i = 1; i < maxMessages; i++) messages[i-1] = messages[i];
    messageCount--;
  }
  messages[messageCount++] = msg; // Add new message
}

// -------------------- WEBSITE DASHBOARD --------------------
String dashboardPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>IoT Security Dashboard</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#eef2f7;margin:0;padding:20px;text-align:center;}";
  html += ".box{background:white;max-width:700px;margin:auto;padding:25px;border-radius:18px;box-shadow:0 4px 14px #999;}";
  html += ".status{font-size:24px;font-weight:bold;padding:15px;border-radius:12px;margin:18px 0;}";
  html += ".granted{background:#d4edda;color:#155724;}";
  html += ".denied{background:#f8d7da;color:#721c24;}";
  html += ".alert{background:#fff3cd;color:#856404;}";
  html += ".info{font-size:18px;text-align:left;margin:10px auto;max-width:500px;}";
  html += ".messages{max-height:150px;overflow-y:auto;background:#f1f1f1;padding:5px;border-radius:4px;margin-top:10px;text-align:left;}";  
  html += ".small{font-size:13px;color:#555;margin-top:20px;}";
  html += "a{display:inline-block;margin-top:14px;padding:10px 15px;background:#0b2a5b;color:white;text-decoration:none;border-radius:8px;}";
  html += "</style></head><body>";
  
  html += "<div class='box'>";
  html += "<h1>IoT Smart Door Security System</h1>";
  html += "<p>RFID + Keypad + IR + Servo Door Lock</p>";

  if (systemStatus.indexOf("GRANTED") >= 0) html += "<div class='status granted'>" + systemStatus + "</div>";
  else if (systemStatus.indexOf("DENIED") >= 0 || systemStatus.indexOf("ALARM") >= 0) html += "<div class='status denied'>" + systemStatus + "</div>";
  else html += "<div class='status alert'>" + systemStatus + "</div>";

  // Display system info
  html += "<div class='info'><b>Wi-Fi Status:</b> "+wifiStatusText+"</div>";
  html += "<div class='info'><b>Last Method:</b> "+lastMethod+"</div>";
  html += "<div class='info'><b>Last RFID UID:</b> "+lastUID+"</div>";
  html += "<div class='info'><b>Failed Attempts:</b> "+String(failedAttempts)+"</div>";
  html += "<div class='info'><b>Visitor Status:</b> "+visitorStatus+"</div>";
  html += "<div class='info'><b>Door Status:</b> "+doorStatus+"</div>";
  html += "<div class='info'><b>Alarm Status:</b> "+alarmStatus+"</div>";

  // Message log
  html += "<h3>Messages</h3><div class='messages'>";
  for (int i = 0; i < messageCount; i++) html += messages[i] + "<br>";
  html += "</div>";

  html += "<a href='/reset'>Reset Dashboard Status</a>";
  html += "<p class='small'>Page refreshes automatically every 2 seconds.</p>";
  html += "</div></body></html>";
  return html;
}

// -------------------- WEB HANDLERS --------------------
void handleRoot() { server.send(200, "text/html", dashboardPage()); }

void handleReset() {
  systemStatus = "System ready";
  lastMethod = "None";
  lastUID = "None";
  doorStatus = "Closed";
  visitorStatus = "No visitor detected";
  alarmStatus = "Off";
  failedAttempts = 0;
  messageCount = 0;
  server.sendHeader("Location", "/");
  server.send(303);
}

// -------------------- SYSTEM EVENTS --------------------
void visitorDetected() {
  ignorePulsesUntil = millis() + 1500;
  pulseCount = 0;
  Serial.println("VISITOR DETECTED AT DOOR");
  Serial.println("Please scan RFID card or enter PIN.");
  systemStatus = "VISITOR DETECTED";
  lastMethod = "IR Sensor";
  visitorStatus = "Visitor at door";
  doorStatus = "Closed";
  alarmStatus = "Off";
  addMessage("Visitor detected by IR sensor");
  beep(1500, 100);
  delay(100);
  beep(1500, 100);
  resetRFID();
}

void accessGranted(String method) {
  ignorePulsesUntil = millis() + 2000;
  pulseCount = 0;
  Serial.print("ACCESS GRANTED by ");
  Serial.println(method);
  systemStatus = "ACCESS GRANTED";
  lastMethod = method;
  failedAttempts = 0;  // Reset failed attempts after access granted
  doorStatus = "Opening / unlocked";
  visitorStatus = "No visitor detected";
  alarmStatus = "Off";
  addMessage("Access granted via " + method);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  beep(2000, 150);
  delay(1500);
  digitalWrite(GREEN_LED, LOW);
  doorStatus = "Auto-closing";
  Serial.println("Door open signal sent to Arduino");
  resetRFID();
}

void accessDenied(String method) {
  ignorePulsesUntil = millis() + 1500;
  pulseCount = 0;
  Serial.print("ACCESS DENIED by ");
  Serial.println(method);
  systemStatus = "ACCESS DENIED";
  lastMethod = method;
  doorStatus = "Closed";
  failedAttempts++;
  alarmStatus = "Off";
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  beep(800, 400);
  delay(600);
  digitalWrite(RED_LED, LOW);
  if (failedAttempts >= maxAttempts) {
    Serial.println("ALARM TRIGGERED - 3 WRONG ATTEMPTS");
    systemStatus = "ALARM TRIGGERED";
    alarmStatus = "On";
    addMessage("ALARM TRIGGERED due to 3 wrong attempts!");
    for (int i = 0; i < 10; i++) {
      digitalWrite(RED_LED, HIGH);
      beep(1200, 200);
      digitalWrite(RED_LED, LOW);
      delay(150);
    }
    failedAttempts = 0;
    alarmStatus = "Off";
  }
  resetRFID();
}

// -------------------- RFID CHECK --------------------
void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  Serial.println("RFID card detected...");
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("RFID detected but UID read failed.");
    delay(300);
    resetRFID();
    return;
  }
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  lastUID = uid;
  Serial.print("Scanned UID: "); Serial.println(uid);
  if (uid == allowedUID) accessGranted("RFID");
  else accessDenied("RFID");
  Serial.println("-------------------");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}

// -------------------- ARDUINO PULSE CHECK --------------------
void checkArduinoPulse() {
  if (millis() < ignorePulsesUntil) {
    pulseCount = 0;
    lastSignalState = digitalRead(KEYPAD_SIGNAL);
    return;
  }
  bool currentState = digitalRead(KEYPAD_SIGNAL);
  if (lastSignalState == HIGH && currentState == LOW) {
    pulseCount++;
    lastPulseTime = millis();
    Serial.print("Pulse received. Count: "); Serial.println(pulseCount);
    delay(60);
  }
  lastSignalState = currentState;
  if (pulseCount > 0 && millis() - lastPulseTime > 500) {
    if (pulseCount == 1) accessGranted("KEYPAD");
    else if (pulseCount == 2) accessDenied("KEYPAD");
    else if (pulseCount == 3) visitorDetected();
    else Serial.println("Unknown Arduino signal");
    pulseCount = 0;
  }
}

// -------------------- WIFI SETUP --------------------
void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) { delay(500); wifiAttempts++; }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("Dashboard IP address: "); Serial.println(WiFi.localIP());
    wifiStatusText = "Connected to Wi-Fi";
    systemStatus = "System ready";
  } else {
    Serial.println("WiFi failed. Starting ESP hotspot...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid, apPassword);
    Serial.print("Connect to hotspot: "); Serial.println(apSsid);
    Serial.print("Dashboard IP address: "); Serial.println(WiFi.softAPIP());
    wifiStatusText = "ESP hotspot mode";
    systemStatus = "System ready";
  }
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);
  delay(1500);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(KEYPAD_SIGNAL, INPUT_PULLUP);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);
  SPI.begin();
  SPI.setFrequency(1000000);
  resetRFID();
  Serial.println();
  Serial.println("RFID + Keypad + IR + Servo Security System Ready");
  setupWiFi();
  server.on("/", handleRoot);
  server.on("/reset", handleReset);
  server.begin();
  Serial.println("Web dashboard started");
  Serial.println("Waiting for visitor, RFID scan, or PIN...");
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.print("RFID Version: 0x"); Serial.println(version, HEX);
  if (version == 0xB2) Serial.println("RFID reader detected successfully.");
  else Serial.println("RFID reader problem. Check wiring.");
}

// -------------------- LOOP --------------------
void loop() {
  server.handleClient();
  checkRFID();
  checkArduinoPulse();
}
