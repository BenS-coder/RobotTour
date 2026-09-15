#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "Math.h"

#define IN1 3
#define IN2 4
#define IN3 5
#define IN4 6
#define IN5 8
#define IN6 9
#define IN7 10
#define IN8 11
const int buttonPin = 2;
int buttonState = 0;  // variable for reading the pushbutton status
ArduinoLEDMatrix matrix;

//Dont use
int stepsFor360 = 7575 * 2;
int stepsForOneSquare = 10900;

const double wheelC = M_PI * 60;
const double wheelDistance = 148;
const double stepsPerOneMm = 4096 / 4 / wheelC;
const double stepsFor90Turn = wheelDistance / 2 * M_PI / 2 * stepsPerOneMm;
const double dowelToAxelDistance = 50;

int leftSteps = 0;
int rightSteps = 0;
boolean leftDirection;
boolean rightDirection;
int counter = 0;

int turnSpeed = 1; //1 is fastest 3 is slower
int numberMmForward = 500 * 19 + 250;
int numberTurns = 12;
int targetTimeMicro = 80 * 1000000;
//Always Minus 5 seconds to target time

const unsigned int delayInMicro = targetTimeMicro / (numberMmForward * stepsPerOneMm + numberTurns * stepsFor90Turn * turnSpeed) /1.1;

//Pre run: Push wheels in, check screws

void setup() {
  matrix.begin();
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT);
  pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT);
  pinMode(IN8, OUTPUT);
  pinMode(buttonPin, INPUT);

  // matrix.beginDraw();
  // matrix.stroke(0xFFFFFFFF);
  // matrix.textScrollSpeed(50);
  // matrix.textFont(Font_4x6);
  // matrix.beginText(0, 1, 0xFFFFFF);
  // matrix.println(delayInMicro);
  // matrix.endText();

  // matrix.endDraw();
}
void loop() {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(60);
  matrix.textFont(Font_4x6);
  matrix.beginText(10, 1, 0xFFFFFF);
  matrix.println(delayInMicro);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
  runRightStepper();
  runLeftStepper();

  while(true) {
    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH) {
      break;
    }
    
  }
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (delayInMicro >= 750) {
    // delay(1000);

    forward(dowelToAxelDistance + 250);  //Needed to the first forward

    left();
    forward(500);
    forward(500);
    left();
    left();
    forward(500);
    forward(500);
    forward(500);
    forward(500);
    left();
    forward(500);
    forward(500);
    left();
    forward(500);
    forward(500);
    left();
    forward(500);
    right();
    forward(500);
    right();
    forward(500);
    forward(500);
    right();
    forward(500);
    right();
    right();
    forward(500);
    forward(500);
    left();
    forward(500);

    forward(500 - dowelToAxelDistance);  //Needed for the last forward
  }
}

void forward(int mm) {

  leftDirection = true;
  rightDirection = true;
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    runLeftStepper();
    runRightStepper();
    delayMicroseconds(delayInMicro);
  }
}

void backward(int mm) {

  leftDirection = false;
  rightDirection = false;
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    runLeftStepper();
    runRightStepper();
    delayMicroseconds(delayInMicro);
  }
}

void right() {
  leftDirection = true;
  rightDirection = false;
  for (int i = 0; i < stepsFor90Turn; i++) {

    runLeftStepper();
    runRightStepper();                      
    delayMicroseconds(delayInMicro*turnSpeed);
  }
}


void left() {
  leftDirection = false;
  rightDirection = true;
  for (int i = 0; i < stepsFor90Turn; i++) {

    runLeftStepper();
    runRightStepper();                      
    delayMicroseconds(delayInMicro*turnSpeed);
  }
}


void run360() {

  //LED
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.beginText(4, 1, 0xFFFFFF);
  const char text[] = "o";
  matrix.textFont(Font_5x7);
  matrix.println(text);

  for (int i = 0; i < stepsFor360; i++) {
    leftDirection = true;
    rightDirection = false;
    runLeftStepper();
    runRightStepper();
    delayMicroseconds(delayInMicro);
  }
}

void runLeftStepper() {
  switch (leftSteps) {
    case 0:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    case 1:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    case 2:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 3:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 4:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 5:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 6:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 7:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    default:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
  }
  incrementLeftStep();
}

void incrementLeftStep() {
  if (!leftDirection) {
    leftSteps++;
  }
  if (leftDirection) {
    leftSteps--;
  }
  if (leftSteps > 7) {
    leftSteps = 0;
  }
  if (leftSteps < 0) {
    leftSteps = 7;
  }
}

void runRightStepper() {
  switch (rightSteps) {
    case 0:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, HIGH);
      break;
    case 1:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, HIGH);
      digitalWrite(IN8, HIGH);
      break;
    case 2:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, HIGH);
      digitalWrite(IN8, LOW);
      break;
    case 3:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, HIGH);
      digitalWrite(IN7, HIGH);
      digitalWrite(IN8, LOW);
      break;
    case 4:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, HIGH);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, LOW);
      break;
    case 5:
      digitalWrite(IN5, HIGH);
      digitalWrite(IN6, HIGH);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, LOW);
      break;
    case 6:
      digitalWrite(IN5, HIGH);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, LOW);
      break;
    case 7:
      digitalWrite(IN5, HIGH);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, HIGH);
      break;
    default:
      digitalWrite(IN5, LOW);
      digitalWrite(IN6, LOW);
      digitalWrite(IN7, LOW);
      digitalWrite(IN8, LOW);
      break;
  }
  incrementRightStep();
}

void incrementRightStep() {
  if (rightDirection) {
    rightSteps++;
  }
  if (!rightDirection) {
    rightSteps--;
  }
  if (rightSteps > 7) {
    rightSteps = 0;
  }
  if (rightSteps < 0) {
    rightSteps = 7;
  }
}
