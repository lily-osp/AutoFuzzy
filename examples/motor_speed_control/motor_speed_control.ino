/**
 * AutoFuzzy Real-World Example: Intelligent Motor Speed Control
 *
 * This example demonstrates fuzzy logic motor speed control for industrial applications.
 * The system considers load requirements, temperature, and efficiency factors
 * to provide optimal motor performance and energy management.
 *
 * Hardware Requirements:
 * - Arduino Mega (recommended) or Uno
 * - Motor speed sensor (simulated with analog input)
 * - Temperature sensor for motor monitoring
 * - Load sensor (simulated)
 * - PWM output for motor control
 */

#include <AutoFuzzy.h>

// System Configuration
const int SPEED_SENSOR_PIN = A0;      // Motor speed feedback
const int TEMP_SENSOR_PIN = A1;       // Motor temperature
const int LOAD_SENSOR_PIN = A2;       // Load requirement input
const int MOTOR_PWM_PIN = 9;          // Motor speed control PWM
const int STATUS_LED_PIN = 13;        // System status indicator

// Control Parameters
const float TARGET_SPEED = 1800.0;    // Target motor speed (RPM)
const float SPEED_TOLERANCE = 50.0;   // Acceptable speed variation (RPM)
const float MAX_MOTOR_TEMP = 80.0;    // Maximum safe motor temperature (°C)
const unsigned long CONTROL_INTERVAL = 500; // Control loop interval (ms)

// Fuzzy Controller Instance
AutoFuzzy motorController;

// Variable Indices
int currentSpeedVar, motorTempVar, loadDemandVar, speedControlVar;

// Timing Variables
unsigned long lastControlTime = 0;

/**
 * Initialize the fuzzy motor control system
 */
bool initializeMotorControl() {
    FuzzyResult result;

    // Define Input Variables
    currentSpeedVar = motorController.addInput("Speed", 0.0, 3000.0);      // RPM
    if (currentSpeedVar < 0) return false;

    motorTempVar = motorController.addInput("Temperature", 20.0, 100.0);   // °C
    if (motorTempVar < 0) return false;

    loadDemandVar = motorController.addInput("Load", 0.0, 100.0);          // % load requirement
    if (loadDemandVar < 0) return false;

    // Define Output Variables
    speedControlVar = motorController.addOutput("Control", 0.0, 255.0);    // PWM duty cycle
    if (speedControlVar < 0) return false;

    // Speed Membership Functions (RPM ranges)
    result = motorController.addTrapezoidalMF(currentSpeedVar, "VeryLow", 0.0, 0.0, 200.0, 400.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(currentSpeedVar, "Low", 300.0, 600.0, 900.0, 1200.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(currentSpeedVar, "Medium", 1000.0, 1400.0, 1600.0, 2000.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(currentSpeedVar, "High", 1800.0, 2200.0, 2500.0, 2800.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(currentSpeedVar, "VeryHigh", 2600.0, 2900.0, 3000.0, 3000.0);
    if (result != FUZZY_OK) return false;

    // Temperature Membership Functions
    result = motorController.addTrapezoidalMF(motorTempVar, "Normal", 20.0, 20.0, 40.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(motorTempVar, "Warm", 45.0, 55.0, 65.0, 70.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(motorTempVar, "Hot", 65.0, 75.0, 85.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Load Demand Membership Functions
    result = motorController.addTrapezoidalMF(loadDemandVar, "Light", 0.0, 0.0, 15.0, 30.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(loadDemandVar, "Medium", 25.0, 45.0, 55.0, 75.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(loadDemandVar, "Heavy", 70.0, 85.0, 100.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Control Output Membership Functions (PWM values)
    result = motorController.addTrapezoidalMF(speedControlVar, "Stop", 0.0, 0.0, 20.0, 40.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(speedControlVar, "Slow", 30.0, 60.0, 90.0, 120.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(speedControlVar, "Medium", 100.0, 140.0, 160.0, 190.0);
    if (result != FUZZY_OK) return false;

    result = motorController.addTrapezoidalMF(speedControlVar, "Fast", 180.0, 210.0, 235.0, 255.0);
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Motor Control

    // Emergency Stop Rules (Temperature Protection)
    result = motorController.addRule("Temperature", "Hot", "Control", "Stop");
    if (result != FUZZY_OK) return false;

    // High Load Rules
    AutoFuzzy::Antecedent highLoadAntecedents[] = {
        {loadDemandVar, motorController.findMF(loadDemandVar, "Heavy")},
        {motorTempVar, motorController.findMF(motorTempVar, "Normal")}
    };
    AutoFuzzy::Consequent highLoadConsequent = {
        speedControlVar, motorController.findMF(speedControlVar, "Fast")
    };
    result = motorController.addRule(highLoadAntecedents, 2, FUZZY_AND, highLoadConsequent);
    if (result != FUZZY_OK) return false;

    // Medium Load Rules
    result = motorController.addRule("Load", "Medium", "Control", "Medium");
    if (result != FUZZY_OK) return false;

    // Light Load Rules
    result = motorController.addRule("Load", "Light", "Control", "Slow");
    if (result != FUZZY_OK) return false;

    // Speed Maintenance Rules
    AutoFuzzy::Antecedent lowSpeedAntecedents[] = {
        {currentSpeedVar, motorController.findMF(currentSpeedVar, "VeryLow")},
        {loadDemandVar, motorController.findMF(loadDemandVar, "Medium")}
    };
    AutoFuzzy::Consequent speedUpConsequent = {
        speedControlVar, motorController.findMF(speedControlVar, "Fast")
    };
    result = motorController.addRule(lowSpeedAntecedents, 2, FUZZY_AND, speedUpConsequent);
    if (result != FUZZY_OK) return false;

    // Temperature-Based Speed Reduction
    AutoFuzzy::Antecedent warmTempAntecedents[] = {
        {motorTempVar, motorController.findMF(motorTempVar, "Warm")},
        {currentSpeedVar, motorController.findMF(currentSpeedVar, "High")}
    };
    AutoFuzzy::Consequent reduceSpeedConsequent = {
        speedControlVar, motorController.findMF(speedControlVar, "Medium")
    };
    result = motorController.addRule(warmTempAntecedents, 2, FUZZY_AND, reduceSpeedConsequent);
    if (result != FUZZY_OK) return false;

    return true;
}

/**
 * Read motor speed from sensor (simulated)
 */
float readMotorSpeed() {
    int sensorValue = analogRead(SPEED_SENSOR_PIN);
    // Convert to RPM (0-3000 range)
    return map(sensorValue, 0, 1023, 0, 3000);
}

/**
 * Read motor temperature (simulated)
 */
float readMotorTemperature() {
    int sensorValue = analogRead(TEMP_SENSOR_PIN);
    // Convert to temperature (20-100°C range)
    return map(sensorValue, 0, 1023, 200, 1000) / 10.0;
}

/**
 * Read load demand (simulated)
 */
float readLoadDemand() {
    int sensorValue = analogRead(LOAD_SENSOR_PIN);
    // Convert to percentage (0-100%)
    return map(sensorValue, 0, 1023, 0, 100);
}

/**
 * Apply motor speed control
 */
void applyMotorControl(float controlOutput) {
    int pwmValue = (int)controlOutput;
    analogWrite(MOTOR_PWM_PIN, pwmValue);

    // Status indication
    if (pwmValue > 200) {
        digitalWrite(STATUS_LED_PIN, HIGH); // High speed operation
    } else if (pwmValue > 100) {
        // Medium speed - could use different indicator
        digitalWrite(STATUS_LED_PIN, LOW);
    } else {
        // Low speed or stopped
        digitalWrite(STATUS_LED_PIN, LOW);
    }
}

/**
 * Safety check for motor operation
 */
bool checkMotorSafety(float temperature, float speed) {
    // Over-temperature protection
    if (temperature >= MAX_MOTOR_TEMP) {
        Serial.println("WARNING: Motor temperature too high!");
        return false;
    }

    // Speed limit protection
    if (speed > 3000.0) {
        Serial.println("WARNING: Motor speed exceeds safe limits!");
        return false;
    }

    return true;
}

/**
 * Display motor control status
 */
void displayMotorStatus(float speed, float temperature, float load, float controlOutput) {
    Serial.print("Motor Status - Speed: ");
    Serial.print(speed, 0);
    Serial.print(" RPM, Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C, Load: ");
    Serial.print(load, 1);
    Serial.print("%, Control: ");
    Serial.println((int)controlOutput);

    // Performance indicators
    if (abs(speed - TARGET_SPEED) <= SPEED_TOLERANCE) {
        Serial.println("✓ Motor speed within target range");
    } else if (speed < TARGET_SPEED) {
        Serial.println("⬆️ Accelerating to reach target speed");
    } else {
        Serial.println("⬇️ Decelerating to reach target speed");
    }

    // Efficiency indicators
    if (temperature > 70.0) {
        Serial.println("⚠️ Motor running warm - consider reducing load");
    }
    if (controlOutput > 240) {
        Serial.println("🔋 High power consumption");
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("=== AutoFuzzy Intelligent Motor Speed Control ===");
    Serial.print("Target Speed: ");
    Serial.print(TARGET_SPEED, 0);
    Serial.println(" RPM");
    Serial.print("Max Temperature: ");
    Serial.print(MAX_MOTOR_TEMP, 1);
    Serial.println("°C");

    // Initialize pins
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Stop motor initially
    analogWrite(MOTOR_PWM_PIN, 0);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize fuzzy controller
    if (!initializeMotorControl()) {
        Serial.println("ERROR: Failed to initialize motor control system!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(200);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(200);
        }
    }

    Serial.println("Motor control system initialized successfully");
    Serial.println("Starting motor monitoring and control...");

    lastControlTime = millis();
}

void loop() {
    unsigned long currentTime = millis();

    // Control loop timing
    if (currentTime - lastControlTime >= CONTROL_INTERVAL) {
        // Read motor parameters
        float currentSpeed = readMotorSpeed();
        float motorTemperature = readMotorTemperature();
        float loadDemand = readLoadDemand();

        // Safety check
        if (!checkMotorSafety(motorTemperature, currentSpeed)) {
            // Emergency stop
            analogWrite(MOTOR_PWM_PIN, 0);
            digitalWrite(STATUS_LED_PIN, HIGH); // Continuous on for error
            Serial.println("EMERGENCY STOP: Safety violation detected");
            delay(1000);
            return;
        }

        // Prepare fuzzy inputs
        FuzzyInput inputs[] = {
            {currentSpeedVar, currentSpeed},
            {motorTempVar, motorTemperature},
            {loadDemandVar, loadDemand}
        };

        // Evaluate fuzzy control output
        float controlOutput;
        FuzzyResult result = motorController.evaluate(inputs, 3, controlOutput, speedControlVar);

        if (result == FUZZY_OK) {
            applyMotorControl(controlOutput);
            displayMotorStatus(currentSpeed, motorTemperature, loadDemand, controlOutput);
        } else {
            // Error handling - safe state
            analogWrite(MOTOR_PWM_PIN, 0);
            Serial.print("Motor control evaluation error: ");
            Serial.println(motorController.getResultString(result));
        }

        lastControlTime = currentTime;
    }

    // Small delay for stability
    delay(50);
}
