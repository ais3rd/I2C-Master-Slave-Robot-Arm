#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08

// --- PIN DEFINITIONS FOR POTENTIOMETERS ---
#define POT_PINCH A0
#define POT_WRIST A2
#define POT_ELBOW A1a
#define POT_WAIST A3

// --- POTENTIOMETER CALIBRATION (0 - 1023) ---
const int IN_PINCH_MIN = 120,  IN_PINCH_MAX = 0;
const int IN_WRIST_MIN = 1023, IN_WRIST_MAX = 756;
const int IN_ELBOW_MIN = 388,  IN_ELBOW_MAX = 711;
const int IN_WAIST_MIN = 339,  IN_WAIST_MAX = 499;

// --- SERVO MICROSECOND LIMITS ---
const int OUT_PINCH_MIN = 1500, OUT_PINCH_MAX = 900;
const int OUT_WRIST_MIN = 2400, OUT_WRIST_MAX = 1300;
const int OUT_ELBOW_MIN = 500,  OUT_ELBOW_MAX = 2300;
const int OUT_WAIST_MIN = 700,  OUT_WAIST_MAX = 2300;

// --- ANTI-STUTTER & NOISE FILTER TUNING ---
const float ANALOG_FILTER_ALPHA = 0.12;  // Low-pass filter (lower = smoother signal)
const float WAIST_FILTER_ALPHA  = 0.05;  // Specific filter for waist stability
const int   WAIST_DEADBAND      = 3;     // Ignores small analog noise twitches

// --- GLOBAL VARIABLES ---
float filteredPinch, filteredWrist, filteredElbow, filteredWaist;
int lastRawWaist = 0;

// Helper to map and constrain values safely within output limits
int mapLimits(int val, int inMin, int inMax, int outMin, int outMax) {
  int mappedVal = map(val, inMin, inMax, outMin, outMax);
  return constrain(mappedVal, min(outMin, outMax), max(outMin, outMax));
}

void setup() {
  Wire.begin(); // Start I2C bus as Master

  // Initialize filters with bootup readings
  filteredPinch = analogRead(POT_PINCH);
  filteredWrist = analogRead(POT_WRIST);
  filteredElbow = analogRead(POT_ELBOW);
  filteredWaist = analogRead(POT_WAIST);
  lastRawWaist  = (int)filteredWaist;
}

void loop() {
  // 1. Read raw analog inputs
  int rawPinch = analogRead(POT_PINCH);
  int rawWrist = analogRead(POT_WRIST);
  int rawElbow = analogRead(POT_ELBOW);
  int rawWaist = analogRead(POT_WAIST);

  // 2. Filter noise using Exponential Moving Average (EMA)
  filteredPinch += (rawPinch - filteredPinch) * ANALOG_FILTER_ALPHA;
  filteredWrist += (rawWrist - filteredWrist) * ANALOG_FILTER_ALPHA;
  filteredElbow += (rawElbow - filteredElbow) * ANALOG_FILTER_ALPHA;

  if (abs(rawWaist - lastRawWaist) > WAIST_DEADBAND) {
    filteredWaist += (rawWaist - filteredWaist) * WAIST_FILTER_ALPHA;
    lastRawWaist = rawWaist;
  }

  // 3. Map filtered readings to target servo pulse widths (microseconds)
  uint16_t targets[4];
  targets[0] = mapLimits((int)filteredPinch, IN_PINCH_MIN, IN_PINCH_MAX, OUT_PINCH_MIN, OUT_PINCH_MAX);
  targets[1] = mapLimits((int)filteredWrist, IN_WRIST_MIN, IN_WRIST_MAX, OUT_WRIST_MIN, OUT_WRIST_MAX);
  targets[2] = mapLimits((int)filteredElbow, IN_ELBOW_MIN, IN_ELBOW_MAX, OUT_ELBOW_MIN, OUT_ELBOW_MAX);
  targets[3] = mapLimits((int)filteredWaist, IN_WAIST_MIN, IN_WAIST_MAX, OUT_WAIST_MIN, OUT_WAIST_MAX);

  // 4. Send 8-byte payload (4 x 16-bit integers) over I2C to Slave Arduino (0x08)
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write((uint8_t*)targets, sizeof(targets));
  Wire.endTransmission();

  delay(15);
}