#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp180;

const int forceNextStageButtonPin = 5;
const int buttonPin = 6;
const int lightAndBuzzerPin = 7;
const int distanceSensorTrigPin = 8;
const int distanceSensorEchoPin = 9;

// Distance sensor variables.
long duration, inches, cm;

// BMP180 variables.
float pressurePa, pressureHPa;

int stage = 0;
void setup() {
  // Setup BMP180
  if (!bmp180.begin())
  {
    Serial.println("BMP180 not found!");
    while (1);
  }

  // Output pins.
  pinMode(lightAndBuzzerPin, OUTPUT);
  pinMode(distanceSensorTrigPin, OUTPUT);

  // Input pins.
  pinMode(distanceSensorEchoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(forceNextStageButtonPin, INPUT_PULLUP);

  Serial.begin(115200);
}

void loop() {
  // Set the distance & pressure for later use.
  setDistanceFromGround();
  setPressure();

  // Show distance sensor readings. (for debugging)
  Serial.print("Inches: ");
  Serial.println(inches);
  Serial.print("Centimeters: ");
  Serial.println(cm);
  Serial.print("Stage: ");
  Serial.println(stage);
  Serial.print("Pressure(s): ");
  Serial.print(pressurePa);
  Serial.print(" Pa || ");
  Serial.print(pressureHPa);
  Serial.println(" hPa");

  Serial.println(digitalRead(buttonPin));

  // If the "Force Next Stage" button is pressed
  // we want to advance to the next stage
  // regardless of if "Next Stage Criteria"
  // has been met.
  if(digitalRead(forceNextStageButtonPin) == LOW)
  {
    stage = stage == 3 ? 0 : (stage + 1);
  }

  // Stage logic.
  if(stage == 0)
  {
    // In Stage 0 we want to wait for the button to be pressed.
    // This sends us to the "Set-up Stage" or Stage 1.
    if(digitalRead(buttonPin) == LOW)
    {
      stage = 1;
      delay(200);
    }
  } else if(stage == 1) {
    // In Stage 1 we want to wait until the payload is at the right height.
    // This sends us to the "Freefall Stage" or Stage 2.
    if(inches >= 50)
    {
      stage = 2;
      delay(200);
    }
  } else if(stage == 2) {
    // In Stage 2 we want to wait until the payload is close to the ground.
    // This sends us to the "Recovery Stage" or Stage 3.
    if(inches <= 10) {
      stage = 3;
      delay(200);
    }
  } else if(stage == 3) {
    // In Stage 3 we want to play a loud sound and flash an LED
    // until the success button is pressed.
    // This resets our stage back to Stage 0.
    if(digitalRead(buttonPin) == LOW)
    {
      stage = 0;
      delay(2000);
    }
  }
}

void setPressure()
{
  pressurePa = bmp180.readPressure();
  pressureHPa = pressurePa / 100.0;
}

void setDistanceFromGround()
{
  digitalWrite(distanceSensorTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(distanceSensorTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distanceSensorTrigPin, LOW);

  duration = pulseIn(distanceSensorEchoPin, HIGH, 30000);

  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  delay(200);
}

long microsecondsToInches(long microseconds)
{
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 29 / 2;
}
