# Arduino code for Keypad, IR, and Servo motor access control

#include <Keypad.h>
#include <Servo.h>

// Define keypad size and layout
const byte ROWS = 4;  // Number of keypad rows
const byte COLS = 4;  // Number of keypad columns

char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

// Define the Arduino pins connected to keypad rows and columns
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

// Initialize keypad library with keymap and pins
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Store entered PIN and the correct PIN
String enteredPIN = "";
String correctPIN = "1234";

// Define other hardware pins
#define SIGNAL_PIN 10       // Pin to send pulses to ESP (D3 on ESP8266)
#define SERVO_PIN 11        // Servo motor control pin
#define ESP_GRANTED_PIN 12  // Input from ESP to indicate access granted
#define IR_PIN A0           // IR sensor output pin

Servo doorServo;  // Servo object for controlling door

// Track visitor detection
bool visitorAlreadyDetected = false;
unsigned long lastVisitorTime = 0;

// Track last ESP signal for detecting rising edge
bool lastEspGrantedState = LOW;
unsigned long lastDoorOpenTime = 0;

// Function to open the door for a fixed time
void openDoor() {
// Prevent rapid repeated openings
if (millis() - lastDoorOpenTime < 4000) return;

lastDoorOpenTime = millis();

Serial.println("DOOR OPENING");
doorServo.write(90);   // Open position
delay(3000);           // Keep door open for 3 seconds
doorServo.write(0);    // Close door
Serial.println("DOOR CLOSED");
}

// Send pulse to ESP (simulate keypad or IR input) it Sends 1 pulse if correct wrong for 2 pulse and 3 for visitor detection
void sendPulse(int pulses) {
for (int i = 0; i < pulses; i++) {
pinMode(SIGNAL_PIN, OUTPUT);
digitalWrite(SIGNAL_PIN, LOW);  // Send LOW pulse
delay(120);
pinMode(SIGNAL_PIN, INPUT);     // Release line (do not send 5V)
delay(180);
}
}

void setup() {
Serial.begin(9600);

pinMode(SIGNAL_PIN, INPUT);        // Input from keypad/IR
pinMode(IR_PIN, INPUT_PULLUP);     // IR sensor input
pinMode(ESP_GRANTED_PIN, INPUT);   // Access granted signal from ESP

doorServo.attach(SERVO_PIN);      // Attach servo
doorServo.write(0);                // Start with door closed

Serial.println("Arduino keypad + IR + servo ready");
Serial.println("Enter PIN then press #");
}

void loop() {
// Check for access granted signal from ESP
bool espGrantedState = digitalRead(ESP_GRANTED_PIN);
if (lastEspGrantedState == LOW && espGrantedState == HIGH) {
Serial.println("Access granted signal received from ESP");
openDoor();  // Open the door when signal rises
}
lastEspGrantedState = espGrantedState;

// IR sensor check
int irState = digitalRead(IR_PIN);
if (irState == LOW && !visitorAlreadyDetected && millis() - lastVisitorTime > 4000) {
Serial.println("Visitor detected at door");
sendPulse(3);            // Send 3 pulses for visitor
visitorAlreadyDetected = true;
lastVisitorTime = millis();
}
if (irState == HIGH) visitorAlreadyDetected = false;  // Reset when no motion

// Keypad check
char key = keypad.getKey();
if (key) {
Serial.print("Key pressed: ");
Serial.println(key);

```
if (key == '#') {  // Submit PIN
  if (enteredPIN == correctPIN) {
    Serial.println("PIN_OK sent to ESP8266");
    sendPulse(1);      // 1 pulse for correct PIN
    openDoor();        // Open door immediately
  } else {
    Serial.println("PIN_FAIL sent to ESP8266");
    sendPulse(2);      // 2 pulses for wrong PIN
  }
  enteredPIN = "";   // Clear entered PIN
  Serial.println("Enter PIN then press #");
} else if (key == '*') {  // Clear PIN
  enteredPIN = "";
  Serial.println("PIN cleared");
} else {  // Append number to PIN
  enteredPIN += key;
  Serial.print("Entered: ");
  Serial.println(enteredPIN);
}
```

}
}
