#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Servo.h>

// Note definitions for jingle
#define N_R            0
#define N_G4           392
#define N_A4           440
#define N_AS4          466
#define N_B4           494
#define N_C5           523
#define N_D5           587
#define N_E5           659
#define N_F5           698
#define N_G5           784
#define N_A5           880
#define N_C6           1047
#define N_D6           1175
#define N_F6           1397

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

// Notes for jingle
int melody_1[] = {
  N_R, N_A4, N_AS4, N_C5, N_D5, N_E5, N_F5, N_G5,
  N_A5, N_A5, N_A5, N_A5, N_F5, N_R, N_A5, N_A5,
  N_A5, N_A5, N_G5, N_F5, N_A4, N_AS4, N_C5, N_D5,
  N_C5, N_R, N_F6, N_R, N_D6, N_R, N_C6, N_R,
  N_G4, N_A4, N_B4, N_C5, N_D5, N_E5, N_F5, N_G5,
  N_A5, N_A5, N_A5, N_A5, N_F5, N_R, N_A5, N_A5,
  N_A5, N_A5, N_G5, N_F5, N_A4, N_AS4, N_C5, N_D5,
  N_C5, N_R, N_F6, N_R, N_D6, N_R, N_C6, N_R,
  N_G4, N_A4, N_B4, N_C5, N_D5, N_E5, N_F5, N_G5
};

// Note durations in milliseconds:
int noteDurations_1[] = {
  79, 79, 79, 79, 79, 79, 79, 79,
  313, 313, 313, 625, 625, 626, 469, 313,
  157, 313, 313, 313, 625, 625, 625, 313,
  625, 314, 156, 314, 156, 314, 156, 470,
  79, 79, 79, 79, 79, 79, 79, 79,
  313, 313, 313, 625, 625, 626, 469, 313,
  157, 313, 313, 313, 625, 625, 625, 313,
  625, 314, 156, 314, 156, 314, 156, 470,
  79, 79, 79, 79, 79, 79, 79, 78
};

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
    playJingle();
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

void playJingle()
{
  int thisNote = 0;
  while(thisNote < 72)
  {
    if(digitalRead(buttonPin) == LOW) return;

    int noteDuration = noteDurations_1[thisNote];
    tone(8, melody_1[thisNote], noteDuration * 0.8);
    int pauseBetweenNotes = noteDuration * 0.9;
    delay(pauseBetweenNotes);
    noTone(8);
    thisNote = thisNote == 71 ? 8 : thisNote + 1;
  }
}
