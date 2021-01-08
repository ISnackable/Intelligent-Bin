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
#define SSID ""
#define PASSSWORD "" 

// Global Variable Declaration
const int redLEDPin = 6; // Red LED 
const int greenLEDPin = 7; // Green LED
const int trigPin = 8; // Ultrasonic ranger: trigPin
const int echoPin = 9; // Ultrasonic ranger: echoPin
const int dipPins[4] = {13, 12, 11, 10};

int UID = 0;
long distance;
long height;
String thingspeakAPI = "";

// Class Declaration
SoftwareSerial ESP01(2, 3); // RX, TX

void setup() {
  Serial.begin(9600); // initialize serial communication:

  pinMode(trigPin, OUTPUT); // UNO's output, ranger's input
  pinMode(echoPin, INPUT); // UNO's input, ranger's output
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);

  for (int i=0; i < 4; i++) {
    pinMode(dipPins[i], INPUT_PULLUP);
  }

  Serial.println("Setting up intelligent bin...");
  delay(5000);

  height = measureDistance();
  Serial.print(height);
  Serial.println("cm");
}

void loop() {
  UID = digitalRead(dipPins[0]) + ( digitalRead(dipPins[1]) * 2 ) + ( digitalRead(dipPins[2]) * 4 ) + ( digitalRead(dipPins[3]) * 8 );

  long distance = measureDistance();
  
  Serial.print(distance);
  Serial.println("cm");

  controlLED(distance);

  // Serial.println();
  // Serial.print("UID: ");
  // Serial.println(UID);

  sendData("AT+RST\r\n",2000,DEBUG);
  sendData("AT+CWMODE=1\r\n",2000,DEBUG);
  sendData("AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n",4000,DEBUG);  

  sendData("AT+CIPMUX=0\r\n",2000,DEBUG);

  // Make TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "3.208.241.190"; // Thingspeak.com's IP address  

  cmd += "\",80\r\n";
  sendData(cmd,2000,DEBUG);

  // Prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += thingspeakAPI;
  getStr +="&field1=";
  getStr += distance;
  getStr += "\r\n"; // Last line for Line feed
  
  // Send data length & GET string
  ESP01.print("AT+CIPSEND=");
  ESP01.println (getStr.length());
  Serial.print("AT+CIPSEND=");
  Serial.println (getStr.length());  
  delay(500);
  if( ESP01.find( ">" ) )
  {
    Serial.print(">");
    sendData(getStr,2000,DEBUG);
  }

  // Close connection, wait a while before repeating...
  sendData("AT+CIPCLOSE",16000,DEBUG); // thingspeak needs 15 sec delay between updates

  delay(500);
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

void controlLED(int distance) {
  digitalWrite(redLEDPin, LOW);
  digitalWrite(greenLEDPin, LOW);

  if (distance < height * 0.15) {
    digitalWrite(greenLEDPin, HIGH);
    delay(1000);
    digitalWrite(greenLEDPin, LOW);
    Serial.println("Bin is full!");
  } else if (distance < height * 0.25) {
    // digitalWrite(yellowLEDPin, HIGH);
    Serial.println("Bin is partially full.");
  } else {
    digitalWrite(redLEDPin, HIGH);
  }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    ESP01.print(command);
    long int time = millis();

    while( (time+timeout) > millis()) {
      while(ESP01.available()) {
        // "Construct" response from ESP01 as follows 
         // - this is to be displayed on Serial Monitor. 
        char c = ESP01.read(); // read the next character.
        response+=c;
      }  
    }

    if(debug) {
      Serial.print(response);
    }
    
    return (response);
}
