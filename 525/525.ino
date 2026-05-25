#include <Servo.h>

Servo mouth;
Servo neck;

int pos = 0;

void setup() {
  // put your setup code here, to run once:
  mouth.attach(9);
  neck.attach(7);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (pos = 0; pos <= 180; pos++){
    mouth.write(pos);
    neck.write(pos);
    delay(15);
  }

  for (pos = 180; pos >= 0; pos--){
    mouth.write(pos);
    neck.write(pos);
    delay(15);
  }
}
