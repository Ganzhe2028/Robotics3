#include <Servo.h>

const byte MOUTH_PIN = 9;
const byte NECK_PIN = 7;

const byte FACE_DATA_LENGTH = 19;
const unsigned long SERVO_UPDATE_INTERVAL_MS = 15;

const int NECK_MIN_ANGLE = 20;
const int NECK_MAX_ANGLE = 160;
const int NECK_CENTER_ANGLE = 90;

const int MOUTH_OPEN_INPUT_MIN = 70;
const int MOUTH_CLOSED_ANGLE = 69;
const int MOUTH_MAX_ANGLE = 90;

Servo mouth;
Servo neck;

char inputBuffer[FACE_DATA_LENGTH + 1];
byte inputIndex = 0;
bool inputOverflow = false;

int neckCurrentAngle = NECK_CENTER_ANGLE;
int neckTargetAngle = NECK_CENTER_ANGLE;
int mouthCurrentAngle = MOUTH_CLOSED_ANGLE;
int mouthTargetAngle = MOUTH_CLOSED_ANGLE;

unsigned long lastServoUpdateMs = 0;

void setup() {
  Serial.begin(115200);

  mouth.attach(MOUTH_PIN);
  neck.attach(NECK_PIN);

  mouth.write(mouthCurrentAngle);
  neck.write(neckCurrentAngle);
}

void loop() {
  readFaceTrackingData();
  updateServos();
}

void readFaceTrackingData() {
  while (Serial.available() > 0) {
    char incoming = Serial.read();

    if (incoming == '\n') {
      finishLine();
    } else if (incoming != '\r') {
      appendCharacter(incoming);
    }
  }
}

void appendCharacter(char incoming) {
  if (inputOverflow) {
    return;
  }

  if (inputIndex < FACE_DATA_LENGTH) {
    inputBuffer[inputIndex] = incoming;
    inputIndex++;
    return;
  }

  inputOverflow = true;
}

void finishLine() {
  inputBuffer[inputIndex] = '\0';

  if (!inputOverflow && isValidFaceData()) {
    applyFaceData();
  }

  inputIndex = 0;
  inputOverflow = false;
}

bool isValidFaceData() {
  if (inputIndex != FACE_DATA_LENGTH) {
    return false;
  }

  for (byte i = 0; i < FACE_DATA_LENGTH; i++) {
    if (!isDigit(inputBuffer[i])) {
      return false;
    }
  }

  return true;
}

void applyFaceData() {
  int yaw = twoDigitValueAt(6);
  int mouthOpen = twoDigitValueAt(10);
  int faceVisible = inputBuffer[18] - '0';

  if (faceVisible == 0) {
    neckTargetAngle = NECK_CENTER_ANGLE;
    mouthTargetAngle = MOUTH_CLOSED_ANGLE;
    return;
  }

  neckTargetAngle = mapClamped(yaw, 0, 99, NECK_MIN_ANGLE, NECK_MAX_ANGLE);
  mouthTargetAngle = mouthAngleFor(mouthOpen);
}

int twoDigitValueAt(byte index) {
  return ((inputBuffer[index] - '0') * 10) + (inputBuffer[index + 1] - '0');
}

int mapClamped(int value, int inputMin, int inputMax, int outputMin, int outputMax) {
  value = constrain(value, inputMin, inputMax);
  return map(value, inputMin, inputMax, outputMin, outputMax);
}

int mouthAngleFor(int mouthOpen) {
  if (mouthOpen <= MOUTH_OPEN_INPUT_MIN) {
    return MOUTH_CLOSED_ANGLE;
  }

  return mapClamped(mouthOpen, MOUTH_OPEN_INPUT_MIN, 99, MOUTH_CLOSED_ANGLE, MOUTH_MAX_ANGLE);
}

void updateServos() {
  unsigned long now = millis();
  if (now - lastServoUpdateMs < SERVO_UPDATE_INTERVAL_MS) {
    return;
  }

  lastServoUpdateMs = now;
  moveServoOneStep(neck, neckCurrentAngle, neckTargetAngle);
  moveServoOneStep(mouth, mouthCurrentAngle, mouthTargetAngle);
}

void moveServoOneStep(Servo &servo, int &currentAngle, int targetAngle) {
  if (currentAngle < targetAngle) {
    currentAngle++;
  } else if (currentAngle > targetAngle) {
    currentAngle--;
  } else {
    return;
  }

  servo.write(currentAngle);
}
