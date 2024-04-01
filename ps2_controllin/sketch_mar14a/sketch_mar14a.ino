void setup() {
  // put your setup code here, to run once:
  pinMode(8, OUTPUT);
  pinMode(7, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  digitalWrite(7, 1);
  
  PinStatus val = digitalRead(8);

  if (val != LOW) {
    Serial.println(val);
  }
}
