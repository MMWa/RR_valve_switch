
// Arduino pins for the shift register
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8

// 8-bit bus after the 74HC595 shift register
// (not Arduino pins)
// These are used to set the direction of the bridge driver.
#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR3_A 5
#define MOTOR3_B 7
#define MOTOR4_A 0
#define MOTOR4_B 6

// Arduino pins for the PWM signals.
#define MOTOR1_PWM 11
#define MOTOR2_PWM 3
#define MOTOR3_PWM 6
#define MOTOR4_PWM 5
#define SERVO1_PWM 10
#define SERVO2_PWM 9

// Codes for the motor function.
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

int aPin_list[] = {A0, A1, A2, A3};
int pin_state[3];
int pin_state_last[3];
int state_counter = 0;
int motor_list[] = {MOTOR1_A, MOTOR2_A, MOTOR3_A, MOTOR4_A};
int state_sum = 0;
void setup() {
  Serial.begin(9600);
  Serial.println("Simple Motor Shield sketch");
  for (int x = 0; x <= 3; x++) {
    pinMode(aPin_list[x], INPUT);
  }
}

void loop() {
  // poll routine
  for (int x = 0; x <= 3; x++) {
    pin_state[x] = 0;
    pin_state[x] = digitalRead(aPin_list[x]);
  }
  
  state_sum = 0;
  for (int x = 0; x <= 3; x++) {
    if (pin_state[x] == 1) {
      state_sum++;
    }
  }
  // if all valves are suppose to be off lets turn them all on
  if (state_sum == 0){
    for (int x = 0; x <= 3; x++) {
      pin_state[x] = 1;
    }
  }
  
  state_counter = 0;
  // check state
  for (int x = 0; x <= 3; x++) {
    if (pin_state[x] != pin_state_last[x]) {
      state_counter++;
    }
  }
  
  if (state_counter > 0) {
    // do the first fire
    if (state_counter > 1) {
      for (int x = 0; x <= 3; x++) {
        if (pin_state[x] == 1) {
          motor_output(motor_list[x], HIGH, 255);
          // hold at maximum power for 20ms
          delay(20);
        }
      }
    }
  
    for (int x = 0; x <= 3; x++) {
      if (pin_state[x] == 1) {
        Serial.println(x);
        // use the normal value 
        motor_output(motor_list[x], HIGH, 160);
      }
    }
  }

  // copy current state to last state
  for (int x = 0; x <= 3; x++) {
    pin_state_last[x] = pin_state[x];
  }
}




//Pre written code 
// ---------------------------------
// motor_output
//
// The function motor_ouput uses the motor driver to
// drive normal outputs like lights, relays, solenoids,
// DC motors (but not in reverse).
//
// It is also used as an internal helper function
// for the motor() function.
//
// The high_low variable should be set 'HIGH'
// to drive lights, etc.
// It can be set 'LOW', to switch it off,
// but also a 'speed' of 0 will switch it off.
//
// The 'speed' sets the PWM for 0...255, and is for
// both pins of the motor output.
//   For example, if motor 3 side 'A' is used to for a
//   dimmed light at 50% (speed is 128), also the
//   motor 3 side 'B' output will be dimmed for 50%.
// Set to 0 for completelty off (high impedance).
// Set to 255 for fully on.
// Special settings for the PWM speed:
//    Set to -1 for not setting the PWM at all.
//
void motor_output(int output, int high_low, int speed) {
  int motorPWM;

  switch (output) {
    case MOTOR1_A:
    case MOTOR1_B:
      motorPWM = MOTOR1_PWM;
      break;
    case MOTOR2_A:
    case MOTOR2_B:
      motorPWM = MOTOR2_PWM;
      break;
    case MOTOR3_A:
    case MOTOR3_B:
      motorPWM = MOTOR3_PWM;
      break;
    case MOTOR4_A:
    case MOTOR4_B:
      motorPWM = MOTOR4_PWM;
      break;
    default:
      // Use speed as error flag, -3333 = invalid output.
      speed = -3333;
      break;
  }

  if (speed != -3333) {
    // Set the direction with the shift register
    // on the MotorShield, even if the speed = -1.
    // In that case the direction will be set, but
    // not the PWM.
    shiftWrite(output, high_low);

    // set PWM only if it is valid
    if (speed >= 0 && speed <= 255) {
      analogWrite(motorPWM, speed);
    }
  }
}

// ---------------------------------
// shiftWrite
//
// The parameters are just like digitalWrite().
//
// The output is the pin 0...7 (the pin behind
// the shift register).
// The second parameter is HIGH or LOW.
//
// There is no initialization function.
// Initialization is automatically done at the first
// time it is used.
//
void shiftWrite(int output, int high_low) {
  static int latch_copy;
  static int shift_register_initialized = false;

  // Do the initialization on the fly,
  // at the first time it is used.
  if (!shift_register_initialized) {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);

    // Set pins for shift register to default value (low);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    // Enable the shift register, set Enable pin Low.
    digitalWrite(MOTORENABLE, LOW);

    // start with all outputs (of the shift register) low
    latch_copy = 0;

    shift_register_initialized = true;
  }

  // The defines HIGH and LOW are 1 and 0.
  // So this is valid.
  bitWrite(latch_copy, output, high_low);

  // Use the default Arduino 'shiftOut()' function to
  // shift the bits with the MOTORCLK as clock pulse.
  // The 74HC595 shiftregister wants the MSB first.
  // After that, generate a latch pulse with MOTORLATCH.
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5);  // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5);  // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}
