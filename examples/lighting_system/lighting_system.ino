/**
 * AutoFuzzy Real-World Example: Intelligent Lighting Control System
 *
 * This example demonstrates fuzzy logic lighting control for smart buildings.
 * The system considers ambient light, occupancy, time of day, and energy efficiency
 * to provide optimal lighting conditions while minimizing power consumption.
 *
 * Hardware Requirements:
 * - Arduino Mega (recommended) or Uno
 * - Ambient light sensor (LDR/photocell)
 * - PIR motion sensor for occupancy detection
 * - Real-time clock (RTC) for time-of-day information
 * - LED lighting array (simulated with PWM outputs)
 * - LCD display for status (optional)
 */

#include <AutoFuzzy.h>

// System Configuration
const int AMBIENT_LIGHT_PIN = A0;    // LDR sensor for ambient light
const int MOTION_SENSOR_PIN = 2;     // PIR motion sensor
const int LIGHT_CONTROL_PIN = 9;     // PWM output for LED lighting
const int STATUS_LED_PIN = 13;       // System status indicator

// Control Parameters
const float TARGET_ILLUMINANCE = 500.0;  // Target lux level
const unsigned long OCCUPANCY_TIMEOUT = 300000; // 5 minutes in milliseconds
const unsigned long CONTROL_INTERVAL = 1000;    // Control loop interval (ms)

// Energy Efficiency Settings
const float ENERGY_SAVINGS_MODE = 0.7;  // 30% reduction during unoccupied periods

// Fuzzy Controller Instance
AutoFuzzy lightingController;

// Variable Indices
int ambientLightVar, occupancyVar, timeOfDayVar, illuminanceVar;

// System State Variables
unsigned long lastMotionTime = 0;
bool occupancyDetected = false;
float currentIlluminanceSetpoint = TARGET_ILLUMINANCE;

/**
 * Initialize the fuzzy lighting control system
 */
bool initializeLightingControl() {
    FuzzyResult result;

    // Define Input Variables
    ambientLightVar = lightingController.addInput("Ambient", 0.0, 1000.0);    // Lux from daylight
    if (ambientLightVar < 0) return false;

    occupancyVar = lightingController.addInput("Occupancy", 0.0, 1.0);         // 0=unoccupied, 1=occupied
    if (occupancyVar < 0) return false;

    timeOfDayVar = lightingController.addInput("TimeOfDay", 0.0, 24.0);        // Hours (0-24)
    if (timeOfDayVar < 0) return false;

    // Define Output Variables
    illuminanceVar = lightingController.addOutput("Illuminance", 0.0, 1000.0); // Target lux level
    if (illuminanceVar < 0) return false;

    // Ambient Light Membership Functions
    result = lightingController.addTrapezoidalMF(ambientLightVar, "Dark", 0.0, 0.0, 50.0, 100.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(ambientLightVar, "Dim", 80.0, 150.0, 250.0, 350.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(ambientLightVar, "Bright", 300.0, 500.0, 700.0, 900.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(ambientLightVar, "VeryBright", 800.0, 950.0, 1000.0, 1000.0);
    if (result != FUZZY_OK) return false;

    // Occupancy Membership Functions
    result = lightingController.addTrapezoidalMF(occupancyVar, "Unoccupied", 0.0, 0.0, 0.1, 0.3);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(occupancyVar, "Occupied", 0.7, 0.9, 1.0, 1.0);
    if (result != FUZZY_OK) return false;

    // Time of Day Membership Functions
    result = lightingController.addTrapezoidalMF(timeOfDayVar, "Night", 0.0, 0.0, 6.0, 8.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(timeOfDayVar, "Morning", 7.0, 9.0, 11.0, 13.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(timeOfDayVar, "Afternoon", 12.0, 14.0, 16.0, 18.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(timeOfDayVar, "Evening", 17.0, 19.0, 21.0, 23.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(timeOfDayVar, "LateNight", 22.0, 23.0, 24.0, 24.0);
    if (result != FUZZY_OK) return false;

    // Illuminance Output Membership Functions
    result = lightingController.addTrapezoidalMF(illuminanceVar, "Off", 0.0, 0.0, 10.0, 25.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(illuminanceVar, "Low", 20.0, 100.0, 200.0, 300.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(illuminanceVar, "Medium", 250.0, 400.0, 500.0, 600.0);
    if (result != FUZZY_OK) return false;

    result = lightingController.addTrapezoidalMF(illuminanceVar, "High", 550.0, 700.0, 850.0, 1000.0);
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Lighting Control

    // Night Time Rules (High priority)
    AutoFuzzy::Antecedent nightDarkAntecedents[] = {
        {ambientLightVar, lightingController.findMF(ambientLightVar, "Dark")},
        {timeOfDayVar, lightingController.findMF(timeOfDayVar, "Night")}
    };
    AutoFuzzy::Consequent nightLightingConsequent = {
        illuminanceVar, lightingController.findMF(illuminanceVar, "High")
    };
    result = lightingController.addRule(nightDarkAntecedents, 2, FUZZY_AND, nightLightingConsequent);
    if (result != FUZZY_OK) return false;

    // Occupied Space Rules
    result = lightingController.addRule("Occupancy", "Occupied", "Illuminance", "High");
    if (result != FUZZY_OK) return false;

    // Morning/Daylight Rules
    AutoFuzzy::Antecedent morningDimAntecedents[] = {
        {ambientLightVar, lightingController.findMF(ambientLightVar, "Dim")},
        {timeOfDayVar, lightingController.findMF(timeOfDayVar, "Morning")},
        {occupancyVar, lightingController.findMF(occupancyVar, "Occupied")}
    };
    AutoFuzzy::Consequent morningLightingConsequent = {
        illuminanceVar, lightingController.findMF(illuminanceVar, "Medium")
    };
    result = lightingController.addRule(morningDimAntecedents, 3, FUZZY_AND, morningLightingConsequent);
    if (result != FUZZY_OK) return false;

    // Afternoon Bright Light Rules
    AutoFuzzy::Antecedent afternoonBrightAntecedents[] = {
        {ambientLightVar, lightingController.findMF(ambientLightVar, "Bright")},
        {timeOfDayVar, lightingController.findMF(timeOfDayVar, "Afternoon")},
        {occupancyVar, lightingController.findMF(occupancyVar, "Occupied")}
    };
    AutoFuzzy::Consequent afternoonLightingConsequent = {
        illuminanceVar, lightingController.findMF(illuminanceVar, "Low")
    };
    result = lightingController.addRule(afternoonBrightAntecedents, 3, FUZZY_AND, afternoonLightingConsequent);
    if (result != FUZZY_OK) return false;

    // Unoccupied Space Rules (Energy Saving)
    AutoFuzzy::Antecedent unoccupiedAntecedents[] = {
        {occupancyVar, lightingController.findMF(occupancyVar, "Unoccupied")},
        {timeOfDayVar, lightingController.findMF(timeOfDayVar, "Evening")}
    };
    AutoFuzzy::Consequent unoccupiedLightingConsequent = {
        illuminanceVar, lightingController.findMF(illuminanceVar, "Low")
    };
    result = lightingController.addRule(unoccupiedAntecedents, 2, FUZZY_AND, unoccupiedLightingConsequent);
    if (result != FUZZY_OK) return false;

    // Late Night Minimum Lighting
    AutoFuzzy::Antecedent lateNightAntecedents[] = {
        {timeOfDayVar, lightingController.findMF(timeOfDayVar, "LateNight")},
        {occupancyVar, lightingController.findMF(occupancyVar, "Unoccupied")}
    };
    AutoFuzzy::Consequent lateNightLightingConsequent = {
        illuminanceVar, lightingController.findMF(illuminanceVar, "Off")
    };
    result = lightingController.addRule(lateNightAntecedents, 2, FUZZY_AND, lateNightLightingConsequent);
    if (result != FUZZY_OK) return false;

    return true;
}

/**
 * Read ambient light level (simulated)
 */
float readAmbientLight() {
    int sensorValue = analogRead(AMBIENT_LIGHT_PIN);
    // Convert to lux (0-1000 range)
    // Inverted mapping for LDR (higher resistance in dark = lower voltage)
    return map(sensorValue, 0, 1023, 1000, 0);
}

/**
 * Check occupancy status
 */
bool checkOccupancy() {
    return digitalRead(MOTION_SENSOR_PIN) == HIGH;
}

/**
 * Get current time of day (simulated - in real system use RTC)
 */
float getTimeOfDay() {
    // Simulate time progression for demonstration
    // In real system, use RTC library
    unsigned long currentMillis = millis();
    float secondsInDay = 86400.0; // 24 hours * 3600 seconds
    float currentSecond = fmod(currentMillis / 1000.0, secondsInDay);
    return (currentSecond / 3600.0); // Convert to hours (0-24)
}

/**
 * Update occupancy status with timeout
 */
void updateOccupancyStatus() {
    bool currentMotion = checkOccupancy();

    if (currentMotion) {
        occupancyDetected = true;
        lastMotionTime = millis();
    } else {
        // Check if occupancy timeout has expired
        if (millis() - lastMotionTime > OCCUPANCY_TIMEOUT) {
            occupancyDetected = false;
        }
    }
}

/**
 * Apply lighting control output
 */
void applyLightingControl(float illuminanceOutput) {
    // Convert lux to PWM value (simplified mapping)
    int pwmValue = map(illuminanceOutput, 0, 1000, 0, 255);
    analogWrite(LIGHT_CONTROL_PIN, pwmValue);

    // Status indication
    if (illuminanceOutput > 500) {
        digitalWrite(STATUS_LED_PIN, HIGH); // Bright lighting
    } else if (illuminanceOutput > 100) {
        digitalWrite(STATUS_LED_PIN, LOW);  // Dim lighting
    } else {
        // Very dim or off - could blink LED
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            lastBlink = millis();
        }
    }
}

/**
 * Calculate energy efficiency metrics
 */
void calculateEnergyEfficiency(float ambientLight, float illuminanceOutput, bool occupied) {
    static float totalEnergyConsumption = 0.0;
    static unsigned long lastCalculation = 0;
    static int sampleCount = 0;

    // Simple energy calculation (PWM value as proxy for power)
    float currentPower = illuminanceOutput / 1000.0; // Normalized 0-1
    totalEnergyConsumption += currentPower;
    sampleCount++;

    // Report efficiency every 30 seconds
    if (millis() - lastCalculation > 30000) {
        float averageEfficiency = totalEnergyConsumption / sampleCount;

        Serial.print("Energy Efficiency - Average Power: ");
        Serial.print(averageEfficiency, 3);
        Serial.print(", Occupancy: ");
        Serial.print(occupied ? "Present" : "Absent");
        Serial.print(", Ambient Light: ");
        Serial.print(ambientLight, 0);
        Serial.println(" lux");

        if (!occupied && averageEfficiency > 0.3) {
            Serial.println("⚠️ High energy use in unoccupied space");
        } else if (occupied && ambientLight > 700 && averageEfficiency > 0.5) {
            Serial.println("💡 Could reduce artificial lighting (bright ambient)");
        }

        lastCalculation = millis();
    }
}

/**
 * Display lighting system status
 */
void displayLightingStatus(float ambientLight, float timeOfDay, bool occupied, float illuminanceOutput) {
    Serial.print("Lighting Status - Ambient: ");
    Serial.print(ambientLight, 0);
    Serial.print(" lux, Time: ");
    Serial.print(timeOfDay, 1);
    Serial.print("h, Occupancy: ");
    Serial.print(occupied ? "Present" : "Absent");
    Serial.print(", Output: ");
    Serial.print(illuminanceOutput, 0);
    Serial.println(" lux");

    // Comfort indicators
    if (illuminanceOutput >= TARGET_ILLUMINANCE * 0.9) {
        Serial.println("✓ Adequate illumination for work tasks");
    } else if (illuminanceOutput > TARGET_ILLUMINANCE * 0.5) {
        Serial.println("⚠️ Reduced illumination - monitor comfort");
    } else {
        Serial.println("❌ Insufficient illumination");
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("=== AutoFuzzy Intelligent Lighting Control System ===");
    Serial.print("Target Illuminance: ");
    Serial.print(TARGET_ILLUMINANCE, 0);
    Serial.println(" lux");
    Serial.print("Occupancy Timeout: ");
    Serial.print(OCCUPANCY_TIMEOUT / 1000);
    Serial.println(" seconds");

    // Initialize pins
    pinMode(AMBIENT_LIGHT_PIN, INPUT);
    pinMode(MOTION_SENSOR_PIN, INPUT);
    pinMode(LIGHT_CONTROL_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Turn off lighting initially
    analogWrite(LIGHT_CONTROL_PIN, 0);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize fuzzy controller
    if (!initializeLightingControl()) {
        Serial.println("ERROR: Failed to initialize lighting control system!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(300);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(300);
        }
    }

    Serial.println("Lighting control system initialized successfully");
    Serial.println("Starting intelligent lighting management...");

    lastMotionTime = millis();
}

void loop() {
    static unsigned long lastControlTime = 0;
    unsigned long currentTime = millis();

    // Update occupancy status
    updateOccupancyStatus();

    // Control loop timing
    if (currentTime - lastControlTime >= CONTROL_INTERVAL) {
        // Read environmental parameters
        float ambientLight = readAmbientLight();
        float timeOfDay = getTimeOfDay();
        float occupancyStatus = occupancyDetected ? 1.0 : 0.0;

        // Prepare fuzzy inputs
        FuzzyInput inputs[] = {
            {ambientLightVar, ambientLight},
            {occupancyVar, occupancyStatus},
            {timeOfDayVar, timeOfDay}
        };

        // Evaluate fuzzy control output
        float illuminanceOutput;
        FuzzyResult result = lightingController.evaluate(inputs, 3, illuminanceOutput, illuminanceVar);

        if (result == FUZZY_OK) {
            // Apply energy-saving adjustments for unoccupied periods
            if (!occupancyDetected) {
                illuminanceOutput *= ENERGY_SAVINGS_MODE;
            }

            applyLightingControl(illuminanceOutput);
            displayLightingStatus(ambientLight, timeOfDay, occupancyDetected, illuminanceOutput);

            // Monitor energy efficiency
            calculateEnergyEfficiency(ambientLight, illuminanceOutput, occupancyDetected);
        } else {
            // Error handling - safe state (minimum lighting)
            analogWrite(LIGHT_CONTROL_PIN, 25); // Very dim emergency lighting
            Serial.print("Lighting control evaluation error: ");
            Serial.println(lightingController.getResultString(result));
        }

        lastControlTime = currentTime;
    }

    // Small delay for stability
    delay(100);
}
