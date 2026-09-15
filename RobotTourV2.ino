#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "Math.h"

const int sf = 0;
const int f = 1;
const int b = 2;
const int l = 3;
const int r = 4;
const int rl = 5;
const int rr = 6;
const int lf = 7;
const int p = 8;


const int dirPin1 = 12;   // direction pin
const int stepPin1 = 11;  // step pin
const int dirPin2 = 5;   // direction pin
const int stepPin2 = 6;  // step pin
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
double wheelDistance = 148;  //revise
const double stepsPerOneMm = 200 / wheelC * 4;
const double stepsFor90Turn = wheelDistance / 2 * M_PI / 2 * stepsPerOneMm;
const double turnForwardMm = wheelDistance / 2;
const double dowelToAxelDistance = 35.5;



int turnSpeed = 1;  //1 is fastest 3 is slower
// int numberMmLinear = 500 * 19 + 250;
int numberMmLinear;
int numberTurns;
int numberPauses;
int pauseTime = 200 * 1000; //microseconds.

int instructions[] = {sf,rr,f,rl,f,rl,f,rr,rr,f,f,rr,f,p,b,rr,f,rr,f,f,rr,f,rr,f,b,rr,f,f,f,rl,f,f,rr,f,rl,f,b,};


int targetTimeMicro = 50 * 1000000;
//Always Minus 5-6 seconds to target time
//Better to be overtime than under

const unsigned int delayInMicro = targetTimeMicro / (numberMmLinear * stepsPerOneMm + numberTurns * stepsFor90Turn * turnSpeed) / 1.1;
// int delayInMicro = 2000;
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

  // matrix.beginDraw();
  // matrix.stroke(0xFFFFFFFF);
  // matrix.textScrollSpeed(50);
  // matrix.textFont(Font_4x6);
  // matrix.beginText(0, 1, 0xFFFFFF);
  // matrix.println(delayInMicro);
  // matrix.endText();

  // matrix.endDraw();

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
      // timingTest();
      break;
    }
    
  }
}

void initializeDelay() {
  numberMmLinear = 0;
  numberTurns = 0;
  bool lastTurn = false;
  int size = sizeof(instructions) / sizeof(instructions[0]);
  for (int i = 0; i < size; i++) {
    if (instructions[i] == sf) {
      int totalMm = dowelToAxelDistance + 250;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      numberMmLinear += totalMm;
      lastTurn = false;
    } else if (instructions[i] == f || instructions[i] == b) {
      int totalMm = 500;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      numberMmLinear += totalMm;
      lastTurn = false;
    } else if (instructions[i] == l || instructions[i] == r) {
      numberTurns += 2;
      lastTurn = true;
    } else if (instructions[i] == rl || instructions[i] == rr) {
      numberTurns += 1;
      lastTurn = false;
    } else if (instructions[i] == lf) {
      int totalMm = 500 - dowelToAxelDistance;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      numberMmLinear += totalMm;
      lastTurn = false;
    } else if (instructions[i] == p) {
      numberPauses++;
    }
  }
  targetTimeMicro -= numberPauses * pauseTime;
  delayInMicro = targetTimeMicro / (numberMmLinear * stepsPerOneMm + numberTurns * stepsFor90Turn * turnSpeed);
}

void runInstructions() {
  bool lastTurn = false;
  int size = sizeof(instructions) / sizeof(instructions[0]);
  for (int i = 0; i < size; i++) {
    if (instructions[i] == sf) {
      int totalMm = dowelToAxelDistance + 250;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      lastTurn = false;
      forward(totalMm);
    } else if (instructions[i] == f || instructions[i] == b) {
      int totalMm = 500;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      lastTurn = false;
      if (instructions[i] == f) {
        forward(totalMm);
      } else {
        backward(totalMm);
      }
      
    } else if (instructions[i] == l) {
      pushLeft();
      lastTurn = true;
    } else if (instructions[i] == r) {
      pushRight();
      lastTurn = true;
    } else if (instructions[i] == rl) {
      left();
      lastTurn = false;
    } else if (instructions[i] == rr) {
      right();
      lastTurn = false;
    } else if (instructions[i] == lf) {
      int totalMm = 500 - dowelToAxelDistance;
      if (lastTurn) {
        totalMm -= turnForwardMm;
      }
      if (i < size - 1) {
        if (instructions[i + 1] == l || instructions[i+1] == r) {
          totalMm -= turnForwardMm;
        }
      }
      lastTurn = false;
      forward(totalMm);
    } else if (instructions[i] == p ) {
      delayMicroseconds(pauseTime);
    }
  }
}

void timingTest() {
  
  // instructions[] = {l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l};
  // instructions = {rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr,rr};
  // targetTimeMicro = 30 * 1000000;


  initializeDelay();
  runInstructions();
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

void forward(int mm) {

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

void backward(int mm) {

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
  wheelDistance = 146.8;
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


void right() {
  wheelDistance = 146.8;
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

void pushRight() {
  wheelDistance = 148;
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
