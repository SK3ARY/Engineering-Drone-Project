#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Servo.h>

Adafruit_BMP085 bmp180;

Servo cameraServo;

const int buttonPin = 6;
const int lightAndBuzzerPin = 7;
const int distanceSensorTrigPin = 8;
const int distanceSensorEchoPin = 9;
const int servoPin = 10;


// Distance sensor variables.
long duration, inches, cm;

// BMP180 variables.
float anchorPressurePa, currentPressurePa, pressureDifferencePa;

// Determines how low the payload will go before going
// into recovery mode (Stage 3).
float differenceThresholdLow = -30;

// Determines how high the payload will go before going
// into freefall mode (Stage 2).
float differenceThresholdHigh = 30;

// Determines how close (in inches) the distance sensor must be
// to change stages to freefall mode (Stage 2).
float distanceSensorThreshold = 10;

int stage = 0;
void setup() {
  // Setup BMP180
  if (!bmp180.begin())
  {
    Serial.println("BMP180 not found!");
    while (1);
  }

  // Setup servo
  cameraServo.attach(servoPin);

  // Output pins.
  pinMode(lightAndBuzzerPin, OUTPUT);
  pinMode(distanceSensorTrigPin, OUTPUT);

  // Input pins.
  pinMode(distanceSensorEchoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

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
  
  // 1. Current Pressure
  Serial.print("Current_Pressure:");
  Serial.print(currentPressurePa);
  Serial.print(","); // Comma separates the variables

  // 2. Anchor Pressure
  Serial.print("Anchor_Pressure:");
  Serial.print(anchorPressurePa);
  Serial.print(",");

  // 3. Pressure Difference (Notice this one ends with println!)
  Serial.print("Pressure_Difference:");
  Serial.println(pressureDifferencePa);

  // Stage logic.
  if(stage == 0)
  {
    // In Stage 0 we want to wait for the button to be pressed.
    // This sends us to the "Set-up Stage" or Stage 1.
    if(digitalRead(buttonPin) == LOW)
    {
      // Set anchored pressure.
      anchorPressurePa = bmp180.readPressure();

      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      stage = 1;
      delay(200);
    }
  } else if(stage == 1) {
    // In Stage 1 we want to wait until the payload is at the right height.
    // This sends us to the "Freefall Stage" or Stage 2.
    if(pressureDifferencePa >= differenceThresholdHigh || digitalRead(buttonPin) == LOW)
    {
      anchorPressurePa = bmp180.readPressure();

      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      stage = 2;
      delay(200);
    }
  } else if(stage == 2) {
    // In Stage 2 we want to move the servo and 
    // wait until the payload is close to the ground.
    // This sends us to the "Recovery Stage" or Stage 3.

    // Move servo code here. (Do testing to find the right values)
    cameraServo.write(90);
    delay(100);
    cameraServo.write(100);
    delay(100);

    // if(pressureDifferencePa <= differenceThresholdLow) {
    if(inches <= distanceSensorThreshold || digitalRead(buttonPin) == LOW) {
      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      stage = 3;
      delay(200);
    }
  } else if(stage == 3) {
    // In Stage 3 we want to play a loud sound and flash an LED
    // until the success button is pressed.
    // This resets our stage back to Stage 0.
    tone(lightAndBuzzerPin, 880, 200);
    delay(300);

    if(digitalRead(buttonPin) == LOW)
    {
      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      stage = 0;
      delay(2000);
    }
  }
}

void setPressure()
{
  currentPressurePa = bmp180.readPressure();
  pressureDifferencePa = anchorPressurePa - currentPressurePa;
}

void setDistanceFromGround()
{
  digitalWrite(distanceSensorTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(distanceSensorTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distanceSensorTrigPin, LOW);

  duration = pulseIn(distanceSensorEchoPin, HIGH, 30000);

  long inchConversion = microsecondsToInches(duration);
  inches = inchConversion == 0 ? 1000 : inchConversion;
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
