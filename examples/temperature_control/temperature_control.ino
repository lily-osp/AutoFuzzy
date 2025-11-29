/**
 * AutoFuzzy Real-World Example: Smart Temperature Control System
 *
 * This example demonstrates a fuzzy logic temperature controller for an HVAC system.
 * The system considers both current temperature and temperature rate of change
 * to provide smooth, intelligent climate control.
 *
 * Hardware Requirements:
 * - Arduino Mega (recommended) or Uno
 * - Temperature sensor (simulated with analog input)
 * - Heating/Cooling actuator (simulated with PWM output)
 * - LCD display for status (optional)
 */

#include <AutoFuzzy.h>

// System Configuration
const int TEMP_SENSOR_PIN = A0;      // Temperature sensor input
const int HEATER_PIN = 9;            // PWM output for heating element
const int COOLER_PIN = 10;           // PWM output for cooling fan
const int STATUS_LED_PIN = 13;       // System status indicator

// Control Parameters
const float TARGET_TEMPERATURE = 22.0;  // Desired room temperature (°C)
const float TEMP_TOLERANCE = 0.5;       // Acceptable temperature variation
const unsigned long CONTROL_INTERVAL = 2000; // Control loop interval (ms)

// Fuzzy Controller Instance
AutoFuzzy climateController;

// Variable Indices (for efficient access)
int currentTempVar, tempRateVar, heaterVar, coolerVar;

// Timing Variables
unsigned long lastControlTime = 0;
float previousTemperature = 0.0;

/**
 * Initialize the fuzzy climate control system
 */
bool initializeClimateControl() {
    FuzzyResult result;

    // Define Input Variables
    currentTempVar = climateController.addInput("Temperature", 15.0, 35.0);
    if (currentTempVar < 0) return false;

    tempRateVar = climateController.addInput("TempRate", -2.0, 2.0);
    if (tempRateVar < 0) return false;

    // Define Output Variables
    heaterVar = climateController.addOutput("Heater", 0.0, 255.0);
    if (heaterVar < 0) return false;

    coolerVar = climateController.addOutput("Cooler", 0.0, 255.0);
    if (coolerVar < 0) return false;

    // Temperature Membership Functions
    result = climateController.addTrapezoidalMF(currentTempVar, "Cold", 15.0, 15.0, 19.0, 21.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(currentTempVar, "Comfortable", 20.0, 21.5, 22.5, 24.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(currentTempVar, "Warm", 23.0, 25.0, 29.0, 31.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(currentTempVar, "Hot", 30.0, 32.0, 35.0, 35.0);
    if (result != FUZZY_OK) return false;

    // Temperature Rate Membership Functions
    result = climateController.addTrapezoidalMF(tempRateVar, "Falling", -2.0, -2.0, -0.5, -0.1);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(tempRateVar, "Stable", -0.2, -0.05, 0.05, 0.2);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(tempRateVar, "Rising", 0.1, 0.5, 2.0, 2.0);
    if (result != FUZZY_OK) return false;

    // Heater Control Membership Functions
    result = climateController.addTrapezoidalMF(heaterVar, "Off", 0.0, 0.0, 20.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(heaterVar, "Low", 30.0, 80.0, 120.0, 150.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(heaterVar, "Medium", 120.0, 160.0, 190.0, 210.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(heaterVar, "High", 200.0, 230.0, 255.0, 255.0);
    if (result != FUZZY_OK) return false;

    // Cooler Control Membership Functions
    result = climateController.addTrapezoidalMF(coolerVar, "Off", 0.0, 0.0, 20.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(coolerVar, "Low", 30.0, 80.0, 120.0, 150.0);
    if (result != FUZZY_OK) return false;

    result = climateController.addTrapezoidalMF(coolerVar, "High", 200.0, 230.0, 255.0, 255.0);
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Heater Control
    // Rule 1: IF Temperature IS Cold THEN Heater IS High
    result = climateController.addRule("Temperature", "Cold", "Heater", "High");
    if (result != FUZZY_OK) return false;

    // Rule 2: IF Temperature IS Comfortable AND TempRate IS Falling THEN Heater IS Medium
    AutoFuzzy::Antecedent heaterRule2Antecedents[] = {
        {currentTempVar, climateController.findMF(currentTempVar, "Comfortable")},
        {tempRateVar, climateController.findMF(tempRateVar, "Falling")}
    };
    AutoFuzzy::Consequent heaterRule2Consequent = {
        heaterVar, climateController.findMF(heaterVar, "Medium")
    };
    result = climateController.addRule(heaterRule2Antecedents, 2, FUZZY_AND, heaterRule2Consequent);
    if (result != FUZZY_OK) return false;

    // Rule 3: IF Temperature IS Warm THEN Heater IS Off
    result = climateController.addRule("Temperature", "Warm", "Heater", "Off");
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Cooler Control
    // Rule 1: IF Temperature IS Hot THEN Cooler IS High
    result = climateController.addRule("Temperature", "Hot", "Cooler", "High");
    if (result != FUZZY_OK) return false;

    // Rule 2: IF Temperature IS Warm AND TempRate IS Rising THEN Cooler IS Low
    AutoFuzzy::Antecedent coolerRule2Antecedents[] = {
        {currentTempVar, climateController.findMF(currentTempVar, "Warm")},
        {tempRateVar, climateController.findMF(tempRateVar, "Rising")}
    };
    AutoFuzzy::Consequent coolerRule2Consequent = {
        coolerVar, climateController.findMF(coolerVar, "Low")
    };
    result = climateController.addRule(coolerRule2Antecedents, 2, FUZZY_AND, coolerRule2Consequent);
    if (result != FUZZY_OK) return false;

    // Rule 3: IF Temperature IS Comfortable THEN Cooler IS Off
    result = climateController.addRule("Temperature", "Comfortable", "Cooler", "Off");
    if (result != FUZZY_OK) return false;

    return true;
}

/**
 * Read temperature from sensor (simulated)
 */
float readTemperature() {
    // Simulate temperature reading with some noise
    int sensorValue = analogRead(TEMP_SENSOR_PIN);
    float voltage = sensorValue * (5.0 / 1023.0);
    float temperature = voltage * 10.0 + 10.0; // Convert to °C range

    // Add realistic bounds
    return constrain(temperature, 15.0, 35.0);
}

/**
 * Calculate temperature rate of change
 */
float calculateTemperatureRate(float currentTemp, float previousTemp, unsigned long deltaTime) {
    if (deltaTime == 0) return 0.0;

    // Calculate rate in °C per second
    float rate = (currentTemp - previousTemp) / (deltaTime / 1000.0);

    // Limit rate to realistic bounds
    return constrain(rate, -2.0, 2.0);
}

/**
 * Apply heating control output
 */
void applyHeatingControl(float heaterOutput) {
    int pwmValue = (int)heaterOutput;
    analogWrite(HEATER_PIN, pwmValue);

    // Visual feedback
    if (pwmValue > 128) {
        digitalWrite(STATUS_LED_PIN, HIGH); // Heating active
    }
}

/**
 * Apply cooling control output
 */
void applyCoolingControl(float coolerOutput) {
    int pwmValue = (int)coolerOutput;
    analogWrite(COOLER_PIN, pwmValue);

    // Visual feedback (could use different LED patterns)
    if (pwmValue > 128) {
        // Cooling active - could blink LED or use different indicator
    }
}

/**
 * Display system status
 */
void displaySystemStatus(float temperature, float tempRate, float heaterOutput, float coolerOutput) {
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print("°C, Rate: ");
    Serial.print(tempRate, 2);
    Serial.print("°C/s, Heater: ");
    Serial.print((int)heaterOutput);
    Serial.print(", Cooler: ");
    Serial.println((int)coolerOutput);

    // Target temperature indicator
    if (abs(temperature - TARGET_TEMPERATURE) <= TEMP_TOLERANCE) {
        Serial.println("✓ Temperature within target range");
    } else if (temperature < TARGET_TEMPERATURE) {
        Serial.println("🔥 Heating to reach target temperature");
    } else {
        Serial.println("❄️ Cooling to reach target temperature");
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    while (!Serial); // Wait for serial connection

    Serial.println("=== AutoFuzzy Smart Temperature Control System ===");
    Serial.print("Target Temperature: ");
    Serial.print(TARGET_TEMPERATURE, 1);
    Serial.println("°C");

    // Initialize pins
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(COOLER_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Turn off all outputs initially
    analogWrite(HEATER_PIN, 0);
    analogWrite(COOLER_PIN, 0);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize fuzzy controller
    if (!initializeClimateControl()) {
        Serial.println("ERROR: Failed to initialize climate control system!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(100);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(100);
        }
    }

    Serial.println("Climate control system initialized successfully");
    Serial.println("Starting temperature monitoring and control...");

    // Initialize temperature tracking
    previousTemperature = readTemperature();
    lastControlTime = millis();
}

void loop() {
    unsigned long currentTime = millis();

    // Control loop timing
    if (currentTime - lastControlTime >= CONTROL_INTERVAL) {
        // Read current temperature
        float currentTemperature = readTemperature();

        // Calculate temperature rate of change
        unsigned long deltaTime = currentTime - lastControlTime;
        float temperatureRate = calculateTemperatureRate(currentTemperature, previousTemperature, deltaTime);

        // Prepare fuzzy inputs
        FuzzyInput inputs[] = {
            {currentTempVar, currentTemperature},
            {tempRateVar, temperatureRate}
        };

        // Evaluate fuzzy control outputs
        float heaterOutput, coolerOutput;

        FuzzyResult heaterResult = climateController.evaluate(inputs, 2, heaterOutput, heaterVar);
        FuzzyResult coolerResult = climateController.evaluate(inputs, 2, coolerOutput, coolerVar);

        // Apply control outputs if evaluation successful
        if (heaterResult == FUZZY_OK && coolerResult == FUZZY_OK) {
            applyHeatingControl(heaterOutput);
            applyCoolingControl(coolerOutput);

            // Display system status
            displaySystemStatus(currentTemperature, temperatureRate, heaterOutput, coolerOutput);
        } else {
            // Error handling - safe state
            analogWrite(HEATER_PIN, 0);
            analogWrite(COOLER_PIN, 0);

            Serial.print("Control evaluation error: ");
            if (heaterResult != FUZZY_OK) {
                Serial.print("Heater - ");
                Serial.println(climateController.getResultString(heaterResult));
            }
            if (coolerResult != FUZZY_OK) {
                Serial.print("Cooler - ");
                Serial.println(climateController.getResultString(coolerResult));
            }
        }

        // Update tracking variables
        previousTemperature = currentTemperature;
        lastControlTime = currentTime;
    }

    // Small delay for stability
    delay(100);
}
