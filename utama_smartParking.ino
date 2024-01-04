#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>/isBooking
#include <Ultrasonic.h>
#include <Servo.h>

#define FIREBASE_HOST "smart-parking-8f0b3-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "fyTzLCoAcvjzTkBo4zGPNik8nzLHxY4JwrPoUGEC"
#define WIFI_SSID "xnxx"
#define WIFI_PASSWORD "123456789"

#define TRIGGER_PIN D4
#define ECHO_PIN D3
const int servoPin = D5;
Servo myServo;

#define NUM_SENSORS 8
int sensorPins[NUM_SENSORS] = {D1, D2};
const char* parentNode = "statusParking";
const char* room1Node = "room1";
const String isBookingNode = String(parentNode) + "/" + String(room1Node) + "/isBooking1"; // Updated path
const int ledPin = D6;  // Replace with actual LED pin

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);


void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myServo.attach(servoPin);

  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  connectToWiFi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  updateParkingStatus();
  checkDistance();
  checkIsBooking();
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WiFi.localIP());
}

void updateParkingStatus() {
  // Assuming both D1 and D2 detection indicates occupied parking
  int sensorValueD1 = digitalRead(sensorPins[0]);
  int sensorValueD2 = digitalRead(sensorPins[1]);

  // If both D1 and D2 detect an object, set parking 1 status to true
  bool parking1Status = (sensorValueD1 == LOW) && (sensorValueD2 == LOW);
  
  // Update Firebase node for parking 1
  Firebase.setBool((String(parentNode) + "/" + String(room1Node) + "/parking1").c_str(), parking1Status);
  Serial.println(parking1Status ? "true" : "false");
  delay(100);

  // If parking1 is true, set isBooking to false
  if (parking1Status) {
    Firebase.setBool(isBookingNode.c_str(), false);
    Serial.println("Setting isBooking to false because parking1 is true");
  }
}


void checkIsBooking() {
  bool isBooking = Firebase.getBool(isBookingNode.c_str());

  Serial.print("Is Booking: ");
  Serial.println(isBooking ? "true" : "false"); // Print the received value for clarity
  
  if (isBooking == true) {
    // Turn off LED
    digitalWrite(ledPin, HIGH);
    Serial.println("Room is booking, LED off");
  } else if (isBooking == false) {
    // Turn on LED
    digitalWrite(ledPin, LOW);
    Serial.println("Room is not booking, LED on");
  } else {
    Serial.println("Invalid data received from Firebase");
  }
}

void checkDistance() {
  float distance_cm = ultrasonic.read();
  Serial.print("Distance: ");
  Serial.println(distance_cm);

  int doorThreshold = 5;
  if (distance_cm < doorThreshold) {
    openDoor();
    delay(1000);
    closeDoor();
  }
}

void openDoor() {
  myServo.write(60);
  delay(500);
}

void closeDoor() {
  myServo.write(0);
  delay(500);
}
