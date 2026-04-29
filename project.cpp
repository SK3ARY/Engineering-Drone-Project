#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Servo.h>

Adafruit_BMP085 bmp180;

Servo cameraServo;

const int forceNextStageButtonPin = 5;
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
float differenceThresholdLow = -80;

// Determines how high the payload will go before going
// into freefall mode (Stage 2).
float differenceThresholdHigh = 90;

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

  // Set anchored pressure.
  anchorPressurePa = bmp180.readPressure();

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
  Serial.print("Current pressure: ");
  Serial.print(currentPressurePa);
  Serial.println(" Pa");
  Serial.print("Anchor pressure: ");
  Serial.print(anchorPressurePa);
  Serial.println(" Pa");
  Serial.print("Pressure difference: ");
  Serial.print(pressureDifferencePa);
  Serial.println(" Pa");

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
      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      stage = 1;
      delay(200);
    }
  } else if(stage == 1) {
    // In Stage 1 we want to wait until the payload is at the right height.
    // This sends us to the "Freefall Stage" or Stage 2.
    if(pressureDifferencePa >= differenceThresholdHigh || inches >= 300)
    {
      // Play tone for confirmation
      tone(lightAndBuzzerPin, 440, 200);
      delay(300);

      anchorPressurePa = bmp180.readPressure();
      stage = 2;
      delay(200);
    }
  } else if(stage == 2) {
    // In Stage 2 we want to move the servo and 
    // wait until the payload is close to the ground.
    // This sends us to the "Recovery Stage" or Stage 3.

    // Move servo code here. (Do testing to find the right values)
    cameraServo.write(95);
    delay(100);
    cameraServo.write(100);
    delay(100);

    if(pressureDifferencePa <= differenceThresholdLow || inches <= 10) {
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
