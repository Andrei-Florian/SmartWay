// Universum | Universum Projects > SmartWay | functions

// Andrei Florian 5/NOV/2018 - 10/NOV/2018

int R = 3; // R for RGB LED
int G = 4; // G for RGB LED
int B = 5; // B for RGB LED
int motorPin = 0; // vibrating motor

void setOutputs()
{
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(motorPin, OUTPUT);
}

void vibrateMotor() // vibrate the motor cell
{
  digitalWrite(motorPin, HIGH);
  delay(1000);
  digitalWrite(motorPin, LOW);
}

void ledGreen() // LED, Green
{
  analogWrite(R, 0);
  analogWrite(G, 255);
  analogWrite(B, 0);
}

void ledOrange() // LED, Orange
{
  analogWrite(R, 255);
  analogWrite(G, 45);
  analogWrite(B, 0);
}

void ledRed() // LED, Red
{
  analogWrite(R, 255);
  analogWrite(G, 0);
  analogWrite(B, 0);
}

void resetLED() // Turn off LED
{
  analogWrite(R, 0);
  analogWrite(G, 0);
  analogWrite(B, 0);
}
