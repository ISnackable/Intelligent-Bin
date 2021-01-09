/**
 * Arduino UNO Pins 
 * 
 * Soft serial Rx (ESP01)       | 2 
 * Soft serial Tx (ESP01)       | 3
 * Red LED                      | 6
 * Green LED                    | 7
 * Ultrasonic ranger: trigPin   | 8
 * Ultrasonic ranger: echoPin   | 9
 * Dip switches                 | 13, 12, 11, 10
*/ 

// Libraries inclusion
#include <SoftwareSerial.h>

// Constant Definition
#define DEBUG true

// Global Variable Declaration
const int redLEDPin = 6; // Red LED 
const int greenLEDPin = 7; // Green LED
const int trigPin = 8; // Ultrasonic ranger: trigPin
const int echoPin = 9; // Ultrasonic ranger: echoPin
const int dipPins[4] = {13, 12, 11, 10};

int errorSend = 0;
int UID = 0;
int greenLEDState = LOW;
long distance;
long height;
long timeIntervalSendData = 1000; // 160 seconds
unsigned long previousTimeBlinkLED = millis();
unsigned long previousTimeSendData = millis();
String thingspeakAPI = ""; // REPLACE WITH YOUR THINGSSPEAK API KEY

// Class Declaration
SoftwareSerial ESP01(2, 3); // RX, TX

void setup() {
  Serial.begin(9600); // initialize serial communication:
  ESP01.begin(9600); 

  pinMode(trigPin, OUTPUT); // UNO's output, ranger's input
  pinMode(echoPin, INPUT); // UNO's input, ranger's output
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);

  for (int i=0; i < 4; i++) {
    pinMode(dipPins[i], INPUT_PULLUP);
  }

  Serial.println("> Setting up intelligent bin...");
  Serial.println("1. Calibrating intelligent bin's height");
  delay(5000); // Wait for 5 Seconds for user to setup properly.
  height = measureDistance();
  // Serial.print(height);
  // Serial.println("cm");

  Serial.println("2. Connecting to Wi-Fi network");
  connectWiFi();

  Serial.println("> Intelligent bin has been sucessfully calibrated!");
}

void loop() {
  unsigned long currentTime = millis();

  // Dip Switch Identifier
  UID = digitalRead(dipPins[0]) + ( digitalRead(dipPins[1]) * 2 ) + ( digitalRead(dipPins[2]) * 4 ) + ( digitalRead(dipPins[3]) * 8 );
  // Serial.println();
  // Serial.print("UID: ");
  // Serial.println(UID);

  // Ultrasonic measure distance
  long distance = measureDistance();
  // Serial.print(distance);
  // Serial.println("cm");

  // LED switches
  controlLED(currentTime, distance);

  // Wi-Fi module, send data to ThingSpeak
  if (currentTime-previousTimeSendData > timeIntervalSendData && !errorSend) {
    previousTimeSendData = currentTime;
    updateData(distance);
  }

  delay(500); // 0.5s
}

long measureDistance() {
  long duration, distance;

  // 1. produce a 5us HIGH pulse in Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // 2. measure the duration of the HIGH pulse in Echo & every 58 us is an obstacle distance of 1 cm
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58;

  return distance;
}

void controlLED(unsigned long currentTime, int distance) {
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, LOW);

  if (distance < (height * 0.10)) {
    // If bin is 90% filled, green LED blink
    if (currentTime-previousTimeBlinkLED > 500) {
      previousTimeBlinkLED = currentTime;
      if (greenLEDState == HIGH) {
        greenLEDState = LOW;
      } else {
        greenLEDState = HIGH;
      }
      digitalWrite(greenLEDPin, greenLEDState);
    }
    Serial.println("Bin is full!");
  } else if (distance < (height * 0.25)) {
    // If bin is 75% filled, yellow LED on
    // digitalWrite(yellowLEDPin, HIGH);
    Serial.println("Bin is partially full.");
  } else {
    digitalWrite(redLEDPin, HIGH);
  }
}

void connectWiFi() {
  sendCommand("AT+RST\r\n",2000,DEBUG); // Reset module
  sendCommand("AT+CWMODE=1\r\n",2000,DEBUG); // Set station mode
  sendCommand("AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n",4000,DEBUG); // Connect Wi-Fi network (REPLACE WITH UR WIFI CREDENTIALS)
  sendCommand("AT+CIPMUX=0\r\n",2000,DEBUG); // Set single-ip connection
  Serial.println("> Wi-Fi Connected!");
  sendCommand("AT+CIFSR\r\n",2000,DEBUG); // SHOW IP ADDRESS
}

void updateData(int distance) {
  // Make TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "54.210.227.170"; // Thingspeak.com's IP address  
  cmd += "\",80\r\n";
  sendCommand(cmd,2000,DEBUG);

  // Prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += thingspeakAPI;
  getStr +="&field1=";
  getStr += distance;
  getStr += "\r\n"; // Last line for Line feed
  
  // Send data length & GET string
  ESP01.print("AT+CIPSEND=");
  ESP01.println(getStr.length());
  delay(500);

  if(ESP01.find(">")) {
    // Serial.print("> ");
    // Serial.println(getStr);
    sendCommand(getStr,2000,DEBUG);
  } else {
    Serial.println("Error sending data, please check WiFi and restart...");
    errorSend = 1;
    // Close connection, wait a while before repeating...
    // sendCommand("AT+CIPCLOSE",2000,DEBUG);
    // connectWiFi();
  }
}

String sendCommand(String command, const int timeout, boolean debug) {
    String response = "";
    ESP01.print(command);
    long int time = millis();

    while((time+timeout) > millis()) {
      while(ESP01.available()) {
        // "Construct" response from ESP01 as follows 
         // - this is to be displayed on Serial Monitor. 
        char c = ESP01.read(); // read the next character.
        response += c;
      }  
    }

    if(debug) {
      Serial.println(response);
    }
    
    return (response);
}
