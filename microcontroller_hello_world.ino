// William Sandoval Casas

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
}

void loop() {
  if (digitalRead(2) == LOW) {
    digitalWrite(8, HIGH);
    delay(250);
    digitalWrite(8, LOW);

    digitalWrite(9, HIGH);
    delay(250);
    digitalWrite(9, LOW);

    digitalWrite(10, HIGH);
    delay(250);
    digitalWrite(10, LOW);
  } else {
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
  }
}
