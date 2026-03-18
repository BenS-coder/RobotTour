#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "Math.h"

const int sf = 0;
const int f = 1;
const int b = 2;
// const int l = 3;
// const int r = 4;
const int rl = 5;
const int rr = 6;
const int lf = 7;
const int p = 8;


const int dirPin1 = 12;   // direction pin
const int stepPin1 = 11;  // step pin
const int dirPin2 = 5;    // direction pin
const int stepPin2 = 6;   // step pin
const int microStep1Pin1 = 10;
const int microStep1Pin2 = 9;
const int microStep2Pin1 = 8;
const int microStep2Pin2 = 7;
const int whiteButtonPin = 4;
const int yellowButtonPin = 3;
const int blueButtonPin = 2;
int whiteButtonState = 0;  // variable for reading the pushbutton status
int yellowButtonState = 0;
int blueButtonState = 0;
ArduinoLEDMatrix matrix;

const double wheelC = M_PI * 60;
double wheelDistance = 145;  //revise
const double stepsPerOneMm = 200 / wheelC * 4;
const double stepsFor90Turn = wheelDistance / 2 * M_PI / 2 * stepsPerOneMm;
const double stepsFor360Turn = stepsFor90Turn * 4;
const double turnForwardMm = wheelDistance / 2;
const double dowelToAxelDistance = 35.5;



int turnSpeed = 1;  //1 is fastest 3 is slower
// int numberMmLinearScaled = 500 * 19 + 250;
double numberMmLinearScaled;
double angleTurnsScaled;
int numberPauses;
int pauseTime = 200 * 1000;  //microseconds.
// double speed[] = { 1, 2, 3 };
double speed[] = { 0.146447, 0.25, 0.37059, 0.5, 0.62941, 0.75, 0.85355, 0.93301, 0.98296, 1 };
double mmRamp = 80;
double angleRamp = 45;

// int instructions[] = {sf,f,rl,rl,lf};
// int instructions[] = {sf,f,f,rl,f,rr,b,b,rl,rl,f,lf};
// int instructions[] = {sf,rl,rl,rl,rl,p,p,rl,rl,rl,rl,p,p,f,r,f,r,f,r,f,r};
// int instructions[] = {rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr};
// int instructions[] = {rr,f,rr,f,rr,f,rr,f};
// int instructions[] = {f,rr,f,rr,f,rr};
// int instructions[] = {sf,f,rr,f,f,rr,rr,f,p,b,b,rr,f,rr,f,f,rr,f,rr,f,p,b,rr,f,f,f,rl,f,f,f,p,b,b,rl,f,f,rr,f,f,rr,f};
// int instructions[] = {sf,rr,f,rl,f,rr,f,rr,f,p,b,rr,f,rr,f,f,rr,f,rr,f,rl,rl,f,rl,f,f,f,f};
// int instructions[] = { sf, rr, f, rl, f, rl, f, rl, rl, f, f, rr, f,  b, rr, f, rr, f, f, rr, f, rr, f, rl, rl, f, rl, f, f, f, rl, f, f, rr, f, rl, f, p, b, rl, f, rl, f, f, rl, f };
int instructions[] = {rl,f,rr,rr,f,b,f,rr,rr,rr};


long targetTimeMicro = 65 * 1000000;
//Always Minus 7-8 seconds to target time
//Better to be overtime than under

// const unsigned int delayInMicro = targetTimeMicro / (numberMmLinearScaled * stepsPerOneMm + angleTurnsScaled * stepsFor90Turn * turnSpeed) / 1.1;
long delayInMicro = 2000;
//Pre run: Push wheels in, check screws, align with ruler

void setup() {
  matrix.begin();
  Serial.begin(9600);

  pinMode(microStep1Pin1, OUTPUT);
  pinMode(microStep1Pin2, OUTPUT);
  pinMode(microStep2Pin1, OUTPUT);
  pinMode(microStep2Pin2, OUTPUT);


  digitalWrite(microStep1Pin1, LOW);
  digitalWrite(microStep1Pin2, HIGH);
  digitalWrite(microStep2Pin1, LOW);
  digitalWrite(microStep2Pin2, HIGH);

  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  digitalWrite(stepPin1, LOW);
  digitalWrite(stepPin2, LOW);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);


  pinMode(whiteButtonPin, INPUT_PULLUP);
  pinMode(yellowButtonPin, INPUT_PULLUP);
  pinMode(blueButtonPin, INPUT_PULLUP);

  initializeDelay();
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

  digitalWrite(stepPin1, HIGH);
  digitalWrite(stepPin2, HIGH);
  delayMicroseconds(700);
  digitalWrite(stepPin1, LOW);
  digitalWrite(stepPin2, LOW);
  delayMicroseconds(700);

  while (true) {
    whiteButtonState = digitalRead(whiteButtonPin);
    blueButtonState = digitalRead(blueButtonPin);
    yellowButtonState = digitalRead(yellowButtonPin);
    if (whiteButtonState == LOW) {
      delay(200);
      runInstructions();
      break;
    } else if (yellowButtonState == LOW) {
      delay(200);
      runTest();
      break;
    } else if (blueButtonState == LOW) {
      speedTest();
      break;
    }
  }
}
void initializeDelay() {
  numberMmLinearScaled = 0;
  angleTurnsScaled = 0;
  int size = sizeof(instructions) / sizeof(instructions[0]);
  int speedSize = sizeof(speed) / sizeof(speed[0]) - 1;

  for (int i = 0; i < size; i++) {
    if (instructions[i] == sf) {
      double totalMm = dowelToAxelDistance + 250 - 2 * mmRamp;
      for (int i = 0; i < speedSize; i++) {
        totalMm += (mmRamp / speedSize) / speed[i] * 2;
      }
      while (i < size - 1 && (instructions[i + 1] == f || instructions[i + 1] == lf)) {
        if (instructions[i + 1] == f) {
          totalMm += 500;
        } else {
          totalMm += 500 - dowelToAxelDistance;
        }
        i++;
      }
      numberMmLinearScaled += totalMm;
    } else if (instructions[i] == f) {
      double totalMm = 500 - 2 * mmRamp;
      for (int i = 0; i < speedSize; i++) {
        totalMm += (mmRamp / speedSize) / speed[i] * 2;
      }
      while (i < size - 1 && (instructions[i + 1] == f || instructions[i + 1] == lf)) {
        if (instructions[i + 1] == f) {
          totalMm += 500;
        } else {
          totalMm += 500 - dowelToAxelDistance;
        }
        i++;
      }
      numberMmLinearScaled += totalMm;
    } else if (instructions[i] == b) {
      double totalMm = 500 - 2 * mmRamp;
      for (int i = 0; i < speedSize; i++) {
        totalMm += (mmRamp / speedSize) / speed[i] * 2;
      }
      while (i < size - 1 && instructions[i + 1] == b) {
        totalMm += 500;
        i++;
      }
      numberMmLinearScaled += totalMm;
    } else if (instructions[i] == rl) {
      double turns = 90 - 2 * angleRamp;
      for (int i = 0; i < speedSize; i++) {
        turns += (angleRamp / speedSize) / speed[i] * 2;
      }
      while (i < size - 1 && (instructions[i + 1] == rl)) {
        turns += 90;
        i++;
      }
    } else if (instructions[i] == rr) {
      double turns = 90 - 2 * angleRamp;
      for (int i = 0; i < speedSize; i++) {
        turns += (angleRamp / speedSize) / speed[i] * 2;
      }
      while (i < size - 1 && (instructions[i + 1] == rr)) {
        turns += 90;
        i++;
      }
      angleTurnsScaled += turns;
    } else if (instructions[i] == lf) {
      double totalMm = 500 - dowelToAxelDistance - 2 * mmRamp;
      for (int i = 0; i < speedSize; i++) {
        totalMm += (mmRamp / speedSize) / speed[i] * 2;
      }
      numberMmLinearScaled += totalMm;
    } else if (instructions[i] == p) {
      numberPauses++;
    }
  }
  
  // targetTimeMicro -= numberPauses * pauseTime;
  // delayInMicro = targetTimeMicro / (numberMmLinearScaled * stepsPerOneMm + angleTurnsScaled / 90 * stepsFor90Turn);
  delayInMicro = targetTimeMicro / (numberMmLinearScaled * stepsPerOneMm + angleTurnsScaled / 90 * stepsFor90Turn);
  Serial.println("numberMmLinearScaled: ");
  Serial.println(numberMmLinearScaled);
  Serial.println("angleTurnsScaled: ");
  Serial.println(angleTurnsScaled);
  Serial.println(targetTimeMicro);
  Serial.println(delayInMicro);
}
void runInstructions() {
  int size = sizeof(instructions) / sizeof(instructions[0]);
  int speedSize = sizeof(speed) / sizeof(speed[0]) - 1;
  for (int i = 0; i < size; i++) {
    if (instructions[i] == sf) {
      int totalMm = dowelToAxelDistance + 250;
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[i]);
      }
      while (i < size - 1 && (instructions[i + 1] == f || instructions[i + 1] == lf)) {
        if (instructions[i + 1] == f) {
          totalMm += 500;
        } else {
          totalMm += 500 - dowelToAxelDistance;
        }
        i++;
      }
      forward(totalMm - 2 * mmRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == f) {
      int totalMm = 500;
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[i]);
      }
      while (i < size - 1 && (instructions[i + 1] == f || instructions[i + 1] == lf)) {
        if (instructions[i + 1] == f) {
          totalMm += 500;
        } else {
          totalMm += 500 - dowelToAxelDistance;
        }
        i++;
      }
      forward(totalMm - 2 * mmRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == b) {
      int totalMm = 500;
      for (int i = 0; i < speedSize; i++) {
        backward(mmRamp / speedSize, speed[i]);
      }
      while (i < size - 1 && instructions[i + 1] == b) {
        totalMm += 500;
        i++;
      }
      backward(totalMm - 2 * mmRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        backward(mmRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == rl) {
      int totalAngle = 90;
      for (int i = 0; i < speedSize; i++) {
        left(angleRamp / speedSize, speed[i]);
      }
      while (i < size - 1 && instructions[i + 1] == rl) {
        totalAngle += 90;
        i++;
      }
      left(totalAngle - 2 * angleRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        left(angleRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == rr) {
      int totalAngle = 90;
      for (int i = 0; i < speedSize; i++) {
        right(angleRamp / speedSize, speed[i]);
      }
      while (i < size - 1 && instructions[i + 1] == rr) {
        totalAngle += 90;
        i++;
      }
      right(totalAngle - 2 * angleRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        right(angleRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == lf) {
      int totalMm = 500 - dowelToAxelDistance;
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[speedSize - i - 1]);
      }
      forward(totalMm - 2 * mmRamp, speed[speedSize]);
      for (int i = 0; i < speedSize; i++) {
        forward(mmRamp / speedSize, speed[speedSize - i - 1]);
      }
    } else if (instructions[i] == p) {
      delayMicroseconds(pauseTime);
    }
  }
}
void speedTest() {

  // instructions[] = {l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l};
  // instructions = {rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr};
  // targetTimeMicro = 30 * 1000000;


  delayInMicro = 1000;
  int speedSize = sizeof(speed) / sizeof(speed[0]) - 1;

  for (int i = 0; i < speedSize; i++) {
    forward(mmRamp / speedSize, speed[i]);
  }
  forward(500 - 2 * mmRamp, speed[speedSize]);
  for (int i = 0; i < speedSize; i++) {
    forward(mmRamp / speedSize, speed[speedSize - i - 1]);
  }

  // right(360, 1);
  for (int i = 0; i < speedSize; i++) {
    right(angleRamp / speedSize, speed[i]);
  }
  right(360 - 2 * angleRamp, speed[speedSize]);
  for (int i = 0; i < speedSize; i++) {
    right(angleRamp / speedSize, speed[speedSize - i - 1]);
  }
}
void runTest() {
  int save = delayInMicro;
  delayInMicro = 2000;
  forward(dowelToAxelDistance);
  right();
  right();
  right();
  right();

  forward(500);
  pushRight();
  forward(500);
  pushRight();
  forward(500);
  pushRight();
  forward(500);
  pushRight();

  left();
  left();
  left();
  left();

  delayInMicro = save;
}
void forward(double mm) {

  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / 2);
  }
}
void forward() {

  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  for (int i = 0; i < stepsPerOneMm * 500; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / 2);
  }
}
void forward(double mm, double speed) {

  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / speed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / speed / 2);
  }
}
void backward(double mm, double speed) {

  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / speed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / speed / 2);
  }
}
void backward(double mm) {

  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  for (int i = 0; i < stepsPerOneMm * mm; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / 2);
  }
}
void left() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, HIGH);
  for (int i = 0; i < stepsFor90Turn; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
  }
}
void left(double angle, double speed) {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, HIGH);
  for (int i = 0; i < stepsFor360Turn * angle / 360; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / speed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / speed / 2);
  }
}
void right() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, LOW);
  for (int i = 0; i < stepsFor90Turn; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
  }
}
void right(double angle, double speed) {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, LOW);
  for (int i = 0; i < stepsFor360Turn * angle / 360; i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro / speed / 2);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro / speed / 2);
  }
}
void pushRight() {
  digitalWrite(dirPin2, LOW);
  digitalWrite(dirPin1, LOW);
  for (int i = 0; i < stepsFor90Turn * 2; i++) {
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
  }
}
void pushLeft() {
  digitalWrite(dirPin2, HIGH);
  digitalWrite(dirPin1, HIGH);
  for (int i = 0; i < stepsFor90Turn * 2; i++) {
    digitalWrite(stepPin1, HIGH);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
    digitalWrite(stepPin1, LOW);
    delayMicroseconds(delayInMicro * turnSpeed / 2);
  }
}