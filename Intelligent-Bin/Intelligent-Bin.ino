// ultrasonic ranger sample code

const int Trig = 13; // Trig connected to pin 13
const int Echo = 12; // Echo connected to pin 12

void setup() {
    // initialize serial communication:
  Serial.begin(9600);
  pinMode(Trig, OUTPUT); // UNO's output, ranger's input
  pinMode(Echo, INPUT); // UNO's input, ranger's output
}

void loop() {
  long duration, cm;
  // hi i'm je-deon

  // 1. produce a 5us HIGH pulse in Trig
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(Trig, LOW);

  // 2. measure the duration of the HIGH pulse in Echo
  //      & every 58 us is an obstacle distance of 1 cm
  duration = pulseIn(Echo, HIGH);
  cm = duration / 58;

  // 3. display the obstacle distance in serial monitor 
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  delay(500);
}
