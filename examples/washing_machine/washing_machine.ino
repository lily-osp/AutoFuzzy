/**
 * AutoFuzzy Real-World Example: Intelligent Washing Machine Control
 *
 * This example demonstrates fuzzy logic control for a smart washing machine.
 * The system considers load weight, fabric type, dirt level, and water quality
 * to optimize wash cycles, detergent usage, and energy consumption.
 *
 * Hardware Requirements:
 * - Arduino Mega (recommended) or Uno
 * - Load cell for weight measurement (HX711 or similar)
 * - Conductivity sensor for water quality
 * - Soil sensor for dirt level detection
 * - Temperature sensor for water heating
 * - Motor control for drum speed (simulated)
 * - Valve controls for water inlet (simulated)
 */

#include <AutoFuzzy.h>

// System Configuration
const int LOAD_SENSOR_PIN = A0;       // Load weight sensor
const int SOIL_SENSOR_PIN = A1;       // Dirt level sensor
const int WATER_QUALITY_PIN = A2;     // Water conductivity sensor
const int DRUM_MOTOR_PIN = 9;         // Drum motor PWM control
const int WATER_VALVE_PIN = 10;       // Water inlet valve control
const int STATUS_LED_PIN = 13;        // System status indicator

// Control Parameters
const float MAX_LOAD_WEIGHT = 8.0;    // Maximum load capacity (kg)
const float MIN_LOAD_WEIGHT = 0.5;    // Minimum detectable load (kg)
const float OPTIMAL_WASH_TIME = 45.0; // Base wash cycle time (minutes)
const unsigned long CONTROL_INTERVAL = 5000; // Control loop interval (ms)

// Wash Cycle Phases
enum WashPhase {
    PHASE_IDLE,
    PHASE_FILLING,
    PHASE_WASHING,
    PHASE_RINSING,
    PHASE_SPINNING,
    PHASE_COMPLETE
};

// Fuzzy Controller Instance
AutoFuzzy washingController;

// Variable Indices
int loadWeightVar, fabricTypeVar, dirtLevelVar, waterQualityVar, washTimeVar, detergentVar, drumSpeedVar;

// System State Variables
WashPhase currentPhase = PHASE_IDLE;
unsigned long phaseStartTime = 0;
float currentWashTime = 0.0;
float detergentUsed = 0.0;
float waterConsumed = 0.0;

/**
 * Initialize the fuzzy washing machine control system
 */
bool initializeWashingControl() {
    FuzzyResult result;

    // Define Input Variables
    loadWeightVar = washingController.addInput("LoadWeight", 0.0, 10.0);     // kg
    if (loadWeightVar < 0) return false;

    fabricTypeVar = washingController.addInput("FabricType", 0.0, 1.0);      // 0=delicate, 1=durable
    if (fabricTypeVar < 0) return false;

    dirtLevelVar = washingController.addInput("DirtLevel", 0.0, 100.0);      // % dirt
    if (dirtLevelVar < 0) return false;

    waterQualityVar = washingController.addInput("WaterQuality", 0.0, 100.0); // % purity
    if (waterQualityVar < 0) return false;

    // Define Output Variables
    washTimeVar = washingController.addOutput("WashTime", 20.0, 90.0);       // minutes
    if (washTimeVar < 0) return false;

    detergentVar = washingController.addOutput("Detergent", 10.0, 150.0);    // ml
    if (detergentVar < 0) return false;

    drumSpeedVar = washingController.addOutput("DrumSpeed", 0.0, 1400.0);    // RPM
    if (drumSpeedVar < 0) return false;

    // Load Weight Membership Functions
    result = washingController.addTrapezoidalMF(loadWeightVar, "Light", 0.0, 0.0, 1.5, 3.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(loadWeightVar, "Medium", 2.5, 4.0, 6.0, 7.5);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(loadWeightVar, "Heavy", 7.0, 8.0, 10.0, 10.0);
    if (result != FUZZY_OK) return false;

    // Fabric Type Membership Functions
    result = washingController.addTrapezoidalMF(fabricTypeVar, "Delicate", 0.0, 0.0, 0.3, 0.5);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(fabricTypeVar, "Normal", 0.4, 0.6, 0.8, 0.9);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(fabricTypeVar, "Durable", 0.8, 0.9, 1.0, 1.0);
    if (result != FUZZY_OK) return false;

    // Dirt Level Membership Functions
    result = washingController.addTrapezoidalMF(dirtLevelVar, "Light", 0.0, 0.0, 20.0, 40.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(dirtLevelVar, "Medium", 30.0, 50.0, 70.0, 80.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(dirtLevelVar, "Heavy", 75.0, 85.0, 100.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Water Quality Membership Functions
    result = washingController.addTrapezoidalMF(waterQualityVar, "Poor", 0.0, 0.0, 25.0, 45.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(waterQualityVar, "Fair", 40.0, 55.0, 75.0, 85.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(waterQualityVar, "Good", 80.0, 90.0, 100.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Wash Time Output Membership Functions
    result = washingController.addTrapezoidalMF(washTimeVar, "Quick", 20.0, 20.0, 30.0, 40.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(washTimeVar, "Normal", 35.0, 45.0, 55.0, 65.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(washTimeVar, "Extended", 60.0, 75.0, 90.0, 90.0);
    if (result != FUZZY_OK) return false;

    // Detergent Output Membership Functions
    result = washingController.addTrapezoidalMF(detergentVar, "Low", 10.0, 10.0, 30.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(detergentVar, "Medium", 40.0, 70.0, 100.0, 120.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(detergentVar, "High", 110.0, 130.0, 150.0, 150.0);
    if (result != FUZZY_OK) return false;

    // Drum Speed Output Membership Functions
    result = washingController.addTrapezoidalMF(drumSpeedVar, "Gentle", 0.0, 0.0, 200.0, 400.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(drumSpeedVar, "Normal", 350.0, 600.0, 900.0, 1100.0);
    if (result != FUZZY_OK) return false;

    result = washingController.addTrapezoidalMF(drumSpeedVar, "Intense", 1000.0, 1200.0, 1400.0, 1400.0);
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Wash Time Control

    // Light load with light dirt - quick wash
    AutoFuzzy::Antecedent quickWashAntecedents[] = {
        {loadWeightVar, washingController.findMF(loadWeightVar, "Light")},
        {dirtLevelVar, washingController.findMF(dirtLevelVar, "Light")}
    };
    AutoFuzzy::Consequent quickWashConsequent = {
        washTimeVar, washingController.findMF(washTimeVar, "Quick")
    };
    result = washingController.addRule(quickWashAntecedents, 2, FUZZY_AND, quickWashConsequent);
    if (result != FUZZY_OK) return false;

    // Heavy load with heavy dirt - extended wash
    AutoFuzzy::Antecedent extendedWashAntecedents[] = {
        {loadWeightVar, washingController.findMF(loadWeightVar, "Heavy")},
        {dirtLevelVar, washingController.findMF(dirtLevelVar, "Heavy")}
    };
    AutoFuzzy::Consequent extendedWashConsequent = {
        washTimeVar, washingController.findMF(washTimeVar, "Extended")
    };
    result = washingController.addRule(extendedWashAntecedents, 2, FUZZY_AND, extendedWashConsequent);
    if (result != FUZZY_OK) return false;

    // Normal load - normal wash time
    result = washingController.addRule("LoadWeight", "Medium", "WashTime", "Normal");
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Detergent Control

    // Light dirt with good water - low detergent
    AutoFuzzy::Antecedent lowDetergentAntecedents[] = {
        {dirtLevelVar, washingController.findMF(dirtLevelVar, "Light")},
        {waterQualityVar, washingController.findMF(waterQualityVar, "Good")}
    };
    AutoFuzzy::Consequent lowDetergentConsequent = {
        detergentVar, washingController.findMF(detergentVar, "Low")
    };
    result = washingController.addRule(lowDetergentAntecedents, 2, FUZZY_AND, lowDetergentConsequent);
    if (result != FUZZY_OK) return false;

    // Heavy dirt or poor water - high detergent
    AutoFuzzy::Antecedent highDetergentAntecedents[] = {
        {dirtLevelVar, washingController.findMF(dirtLevelVar, "Heavy")}
    };
    AutoFuzzy::Consequent highDetergentConsequent = {
        detergentVar, washingController.findMF(detergentVar, "High")
    };
    result = washingController.addRule(highDetergentAntecedents, 1, FUZZY_AND, highDetergentConsequent);
    if (result != FUZZY_OK) return false;

    // Medium dirt - medium detergent
    result = washingController.addRule("DirtLevel", "Medium", "Detergent", "Medium");
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Drum Speed Control

    // Delicate fabrics - gentle speed
    result = washingController.addRule("FabricType", "Delicate", "DrumSpeed", "Gentle");
    if (result != FUZZY_OK) return false;

    // Durable fabrics with heavy dirt - intense speed
    AutoFuzzy::Antecedent intenseSpeedAntecedents[] = {
        {fabricTypeVar, washingController.findMF(fabricTypeVar, "Durable")},
        {dirtLevelVar, washingController.findMF(dirtLevelVar, "Heavy")}
    };
    AutoFuzzy::Consequent intenseSpeedConsequent = {
        drumSpeedVar, washingController.findMF(drumSpeedVar, "Intense")
    };
    result = washingController.addRule(intenseSpeedAntecedents, 2, FUZZY_AND, intenseSpeedConsequent);
    if (result != FUZZY_OK) return false;

    // Normal conditions - normal speed
    result = washingController.addRule("FabricType", "Normal", "DrumSpeed", "Normal");
    if (result != FUZZY_OK) return false;

    return true;
}

/**
 * Read load weight from sensor (simulated)
 */
float readLoadWeight() {
    int sensorValue = analogRead(LOAD_SENSOR_PIN);
    // Convert to weight (0-10kg range)
    return map(sensorValue, 0, 1023, 0, 100) / 10.0;
}

/**
 * Read dirt level from sensor (simulated)
 */
float readDirtLevel() {
    int sensorValue = analogRead(SOIL_SENSOR_PIN);
    // Convert to dirt percentage (0-100%)
    return map(sensorValue, 0, 1023, 0, 100);
}

/**
 * Read water quality from sensor (simulated)
 */
float readWaterQuality() {
    int sensorValue = analogRead(WATER_QUALITY_PIN);
    // Convert to quality percentage (0-100%)
    // Higher values = better quality
    return map(sensorValue, 0, 1023, 0, 100);
}

/**
 * Get fabric type setting (simulated - in real system use selector switch)
 */
float getFabricType() {
    // Simulate fabric type selection
    // 0.0 = delicate, 0.5 = normal, 1.0 = durable
    // In real system, read from digital inputs or rotary switch
    return 0.5; // Normal fabrics default
}

/**
 * Apply wash cycle control
 */
void applyWashControl(float washTime, float detergentAmount, float drumSpeed, WashPhase phase) {
    static WashPhase lastPhase = PHASE_IDLE;

    if (phase != lastPhase) {
        // Phase transition
        Serial.print("Phase change: ");
        switch (phase) {
            case PHASE_FILLING: Serial.println("Filling water"); break;
            case PHASE_WASHING: Serial.println("Washing cycle"); break;
            case PHASE_RINSING: Serial.println("Rinsing cycle"); break;
            case PHASE_SPINNING: Serial.println("Spinning cycle"); break;
            case PHASE_COMPLETE: Serial.println("Cycle complete"); break;
        }
        lastPhase = phase;
    }

    // Apply drum speed control
    int motorPWM = map(drumSpeed, 0, 1400, 0, 255);
    analogWrite(DRUM_MOTOR_PIN, motorPWM);

    // Water valve control (simplified)
    if (phase == PHASE_FILLING || phase == PHASE_RINSING) {
        digitalWrite(WATER_VALVE_PIN, HIGH);
        waterConsumed += 0.1; // Simulate water usage
    } else {
        digitalWrite(WATER_VALVE_PIN, LOW);
    }

    // Detergent dispensing (simplified timing)
    static bool detergentDispensed = false;
    if (phase == PHASE_WASHING && !detergentDispensed) {
        detergentUsed += detergentAmount;
        detergentDispensed = true;
        Serial.print("Dispensing ");
        Serial.print(detergentAmount, 1);
        Serial.println("ml detergent");
    }

    // Status indication
    if (phase != PHASE_IDLE && phase != PHASE_COMPLETE) {
        digitalWrite(STATUS_LED_PIN, HIGH);
    } else {
        digitalWrite(STATUS_LED_PIN, LOW);
    }
}

/**
 * Determine current wash phase based on time and settings
 */
WashPhase determineWashPhase(float washTime, unsigned long cycleStartTime) {
    unsigned long elapsedTime = millis() - cycleStartTime;
    float elapsedMinutes = elapsedTime / 60000.0; // Convert to minutes

    if (elapsedMinutes < 2.0) {
        return PHASE_FILLING;
    } else if (elapsedMinutes < washTime * 0.7) {
        return PHASE_WASHING;
    } else if (elapsedMinutes < washTime * 0.85) {
        return PHASE_RINSING;
    } else if (elapsedMinutes < washTime) {
        return PHASE_SPINNING;
    } else {
        return PHASE_COMPLETE;
    }
}

/**
 * Safety checks for washing machine operation
 */
bool checkWashSafety(float loadWeight, float drumSpeed) {
    // Overload protection
    if (loadWeight > MAX_LOAD_WEIGHT) {
        Serial.println("WARNING: Load weight exceeds maximum capacity!");
        return false;
    }

    // Underload protection
    if (loadWeight < MIN_LOAD_WEIGHT) {
        Serial.println("WARNING: Load weight below minimum requirement!");
        return false;
    }

    // Speed limit protection
    if (drumSpeed > 1400.0) {
        Serial.println("WARNING: Drum speed exceeds safe limits!");
        return false;
    }

    return true;
}

/**
 * Monitor wash efficiency and resource usage
 */
void monitorWashEfficiency(float loadWeight, float detergentAmount, float waterConsumed) {
    static unsigned long lastReport = 0;

    // Report efficiency every 30 seconds during wash
    if (millis() - lastReport > 30000 && currentPhase != PHASE_IDLE) {
        Serial.print("Efficiency Report - Load: ");
        Serial.print(loadWeight, 1);
        Serial.print("kg, Detergent: ");
        Serial.print(detergentUsed, 1);
        Serial.print("ml, Water: ");
        Serial.print(waterConsumed, 2);
        Serial.println("L");

        // Efficiency calculations
        float detergentPerKg = detergentUsed / loadWeight;
        float waterPerKg = waterConsumed / loadWeight;

        if (detergentPerKg > 25.0) {
            Serial.println("⚠️ High detergent usage - consider load size");
        }

        if (waterPerKg > 15.0) {
            Serial.println("💧 High water consumption detected");
        }

        lastReport = millis();
    }
}

/**
 * Display washing machine status
 */
void displayWashStatus(float loadWeight, float fabricType, float dirtLevel, float waterQuality,
                      float washTime, float detergentAmount, float drumSpeed, WashPhase phase) {
    Serial.print("Wash Status - Load: ");
    Serial.print(loadWeight, 1);
    Serial.print("kg, Fabric: ");
    Serial.print(fabricType < 0.4 ? "Delicate" : fabricType > 0.6 ? "Durable" : "Normal");
    Serial.print(", Dirt: ");
    Serial.print(dirtLevel, 1);
    Serial.print("%, Water: ");
    Serial.print(waterQuality, 1);
    Serial.println("%");

    Serial.print("Settings - Time: ");
    Serial.print(washTime, 1);
    Serial.print("min, Detergent: ");
    Serial.print(detergentAmount, 1);
    Serial.print("ml, Speed: ");
    Serial.print(drumSpeed, 0);
    Serial.println(" RPM");

    // Phase indicator
    Serial.print("Phase: ");
    switch (phase) {
        case PHASE_IDLE: Serial.println("Idle"); break;
        case PHASE_FILLING: Serial.println("Filling"); break;
        case PHASE_WASHING: Serial.println("Washing"); break;
        case PHASE_RINSING: Serial.println("Rinsing"); break;
        case PHASE_SPINNING: Serial.println("Spinning"); break;
        case PHASE_COMPLETE: Serial.println("Complete"); break;
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("=== AutoFuzzy Intelligent Washing Machine Control ===");
    Serial.print("Capacity: ");
    Serial.print(MAX_LOAD_WEIGHT, 1);
    Serial.println("kg");
    Serial.print("Optimal Cycle: ");
    Serial.print(OPTIMAL_WASH_TIME, 1);
    Serial.println(" minutes");

    // Initialize pins
    pinMode(LOAD_SENSOR_PIN, INPUT);
    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(WATER_QUALITY_PIN, INPUT);
    pinMode(DRUM_MOTOR_PIN, OUTPUT);
    pinMode(WATER_VALVE_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Initialize outputs
    analogWrite(DRUM_MOTOR_PIN, 0);
    digitalWrite(WATER_VALVE_PIN, LOW);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize fuzzy controller
    if (!initializeWashingControl()) {
        Serial.println("ERROR: Failed to initialize washing machine control!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(300);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(300);
        }
    }

    Serial.println("Washing machine control system initialized successfully");
    Serial.println("Ready for intelligent wash cycle optimization...");

    phaseStartTime = millis();
}

void loop() {
    static unsigned long lastControlTime = 0;
    static unsigned long cycleStartTime = 0;
    static bool cycleActive = false;
    unsigned long currentTime = millis();

    // Control loop timing
    if (currentTime - lastControlTime >= CONTROL_INTERVAL) {
        // Read current parameters
        float loadWeight = readLoadWeight();
        float fabricType = getFabricType();
        float dirtLevel = readDirtLevel();
        float waterQuality = readWaterQuality();

        // Safety check
        if (!checkWashSafety(loadWeight, 0.0)) { // Basic check
            applyWashControl(0.0, 0.0, 0.0, PHASE_IDLE);
            Serial.println("EMERGENCY STOP: Safety violation detected");
            delay(2000);
            return;
        }

        // Start cycle if not active and load detected
        if (!cycleActive && loadWeight >= MIN_LOAD_WEIGHT) {
            Serial.println("🚀 Starting intelligent wash cycle");
            cycleActive = true;
            cycleStartTime = currentTime;
            detergentUsed = 0.0;
            waterConsumed = 0.0;
        }

        if (cycleActive) {
            // Prepare fuzzy inputs
            FuzzyInput inputs[] = {
                {loadWeightVar, loadWeight},
                {fabricTypeVar, fabricType},
                {dirtLevelVar, dirtLevel},
                {waterQualityVar, waterQuality}
            };

            // Evaluate fuzzy control outputs
            float washTimeOutput, detergentOutput, drumSpeedOutput;

            FuzzyResult timeResult = washingController.evaluate(inputs, 4, washTimeOutput, washTimeVar);
            FuzzyResult detergentResult = washingController.evaluate(inputs, 4, detergentOutput, detergentVar);
            FuzzyResult speedResult = washingController.evaluate(inputs, 4, drumSpeedOutput, drumSpeedVar);

            if (timeResult == FUZZY_OK && detergentResult == FUZZY_OK && speedResult == FUZZY_OK) {
                // Determine current phase
                currentPhase = determineWashPhase(washTimeOutput, cycleStartTime);

                // Apply control outputs
                if (checkWashSafety(loadWeight, drumSpeedOutput)) {
                    applyWashControl(washTimeOutput, detergentOutput, drumSpeedOutput, currentPhase);
                    displayWashStatus(loadWeight, fabricType, dirtLevel, waterQuality,
                                    washTimeOutput, detergentOutput, drumSpeedOutput, currentPhase);

                    // Monitor efficiency
                    monitorWashEfficiency(loadWeight, detergentOutput, waterConsumed);
                } else {
                    applyWashControl(0.0, 0.0, 0.0, PHASE_IDLE);
                    Serial.println("Cycle stopped due to safety violation");
                }

                // Check for cycle completion
                if (currentPhase == PHASE_COMPLETE) {
                    cycleActive = false;
                    Serial.println("✅ Wash cycle completed successfully");
                    Serial.print("Total Resources - Detergent: ");
                    Serial.print(detergentUsed, 1);
                    Serial.print("ml, Water: ");
                    Serial.print(waterConsumed, 2);
                    Serial.println("L");
                }
            } else {
                // Error handling
                applyWashControl(0.0, 0.0, 0.0, PHASE_IDLE);
                Serial.print("Wash control evaluation error: ");
                if (timeResult != FUZZY_OK) Serial.println("Time calculation failed");
                if (detergentResult != FUZZY_OK) Serial.println("Detergent calculation failed");
                if (speedResult != FUZZY_OK) Serial.println("Speed calculation failed");
            }
        } else {
            // Idle state
            currentPhase = PHASE_IDLE;
            applyWashControl(0.0, 0.0, 0.0, PHASE_IDLE);
        }

        lastControlTime = currentTime;
    }

    // Small delay for stability
    delay(100);
}
