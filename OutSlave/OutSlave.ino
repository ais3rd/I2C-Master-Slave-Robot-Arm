#include <Wire.h>
#include <Servo.h>

#define I2C_SLAVE_ADDR 0x08

// --- PIN DEFINITIONS ---
#define SERVO_PINCH 3
#define SERVO_WRIST 5
#define SERVO_ELBOW 6
#define SERVO_WAIST 9

// --- MOTION TUNING ---
const float MOTION_SMOOTHING       = 0.15;
const float WAIST_MOTION_SMOOTHING = 0.08;

// --- GLOBAL VARIABLES ---
Servo pinchServo, wristServo, elbowServo, waistServo;

// Interrupt-updated targets
volatile uint16_t targetPinch = 1200;
volatile uint16_t targetWrist = 1850;
volatile uint16_t targetElbow = 1400;
volatile uint16_t targetWaist = 1500;

float currentPinch, currentWrist, currentElbow, currentWaist;

void setup() {
  Wire.begin(I2C_SLAVE_ADDR); // Join I2C bus as Slave at 0x08
  Wire.onReceive(receiveEvent);

  pinchServo.attach(SERVO_PINCH, 500, 2500);
  wristServo.attach(SERVO_WRIST, 500, 2500);
  elbowServo.attach(SERVO_ELBOW, 500, 2500);
  waistServo.attach(SERVO_WAIST, 500, 2500);

  currentPinch = targetPinch;
  currentWrist = targetWrist;
  currentElbow = targetElbow;
  currentWaist = targetWaist;
}

void loop() {
  // 1. Smooth Motion Interpolation
  currentPinch += (targetPinch - currentPinch) * MOTION_SMOOTHING;
  currentWrist += (targetWrist - currentWrist) * MOTION_SMOOTHING;
  currentElbow += (targetElbow - currentElbow) * MOTION_SMOOTHING;
  currentWaist += (targetWaist - currentWaist) * WAIST_MOTION_SMOOTHING;

  // 2. Drive Servos
  pinchServo.write((int)currentPinch);
  wristServo.write((int)currentWrist);
  elbowServo.write((int)currentElbow);
  waistServo.write((int)currentWaist);

  delay(15);
}

// Handler for incoming data over I2C
void receiveEvent(int numBytes) {
  if (numBytes == 8) { // Expecting 4 x 16-bit uint16_t targets
    uint16_t receivedTargets[4];
    Wire.readBytes((char*)receivedTargets, sizeof(receivedTargets));

    targetPinch = receivedTargets[0];
    targetWrist = receivedTargets[1];
    targetElbow = receivedTargets[2];
    targetWaist = receivedTargets[3];
  }
}