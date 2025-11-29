/**
 * AutoFuzzy Real-World Example: Smart Irrigation Control System
 *
 * This example demonstrates fuzzy logic irrigation control for agricultural applications.
 * The system considers soil moisture, temperature, humidity, plant type, and weather
 * conditions to provide optimal watering while conserving water resources.
 *
 * Hardware Requirements:
 * - Arduino Mega (recommended) or Uno
 * - Soil moisture sensors (capacitive/resistive)
 * - Temperature and humidity sensor (DHT22 or similar)
 * - Rain sensor for weather detection
 * - Solenoid valves for water control (simulated with digital outputs)
 * - Flow sensor for water usage monitoring
 */

#include <AutoFuzzy.h>

// System Configuration
const int SOIL_MOISTURE_PIN = A0;    // Soil moisture sensor
const int TEMP_HUMIDITY_PIN = A1;    // DHT22 data pin (simulated)
const int RAIN_SENSOR_PIN = A2;      // Rain detection sensor
const int VALVE_CONTROL_PIN = 9;     // Solenoid valve control
const int STATUS_LED_PIN = 13;       // System status indicator

// Control Parameters
const float OPTIMAL_MOISTURE = 60.0;     // Target soil moisture (%)
const float MOISTURE_TOLERANCE = 10.0;   // Acceptable moisture variation
const float MAX_IRRIGATION_TIME = 300.0; // Maximum watering time (seconds)
const unsigned long CONTROL_INTERVAL = 2000; // Control loop interval (ms)

// Plant Type Settings (adjustable)
const float DROUGHT_TOLERANT_FACTOR = 0.7;  // Water reduction for drought-tolerant plants
const float WATER_INTENSIVE_FACTOR = 1.3;   // Water increase for water-intensive plants

// Fuzzy Controller Instance
AutoFuzzy irrigationController;

// Variable Indices
int soilMoistureVar, airTempVar, humidityVar, rainVar, plantTypeVar, irrigationVar;

// System State Variables
unsigned long irrigationStartTime = 0;
bool irrigationActive = false;
float waterUsageTotal = 0.0;

/**
 * Initialize the fuzzy irrigation control system
 */
bool initializeIrrigationControl() {
    FuzzyResult result;

    // Define Input Variables
    soilMoistureVar = irrigationController.addInput("SoilMoisture", 0.0, 100.0);  // % moisture
    if (soilMoistureVar < 0) return false;

    airTempVar = irrigationController.addInput("AirTemp", 10.0, 45.0);            // °C
    if (airTempVar < 0) return false;

    humidityVar = irrigationController.addInput("Humidity", 0.0, 100.0);          // % RH
    if (humidityVar < 0) return false;

    rainVar = irrigationController.addInput("Rain", 0.0, 1.0);                    // 0=no rain, 1=rain
    if (rainVar < 0) return false;

    plantTypeVar = irrigationController.addInput("PlantType", 0.0, 1.0);          // 0=drought-tolerant, 1=water-intensive
    if (plantTypeVar < 0) return false;

    // Define Output Variables
    irrigationVar = irrigationController.addOutput("Irrigation", 0.0, 100.0);     // % valve opening
    if (irrigationVar < 0) return false;

    // Soil Moisture Membership Functions
    result = irrigationController.addTrapezoidalMF(soilMoistureVar, "VeryDry", 0.0, 0.0, 15.0, 25.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(soilMoistureVar, "Dry", 20.0, 30.0, 40.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(soilMoistureVar, "Optimal", 45.0, 55.0, 65.0, 75.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(soilMoistureVar, "Wet", 70.0, 80.0, 90.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Air Temperature Membership Functions
    result = irrigationController.addTrapezoidalMF(airTempVar, "Cool", 10.0, 10.0, 18.0, 22.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(airTempVar, "Moderate", 20.0, 25.0, 30.0, 35.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(airTempVar, "Hot", 32.0, 38.0, 45.0, 45.0);
    if (result != FUZZY_OK) return false;

    // Humidity Membership Functions
    result = irrigationController.addTrapezoidalMF(humidityVar, "Low", 0.0, 0.0, 25.0, 40.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(humidityVar, "Moderate", 35.0, 50.0, 65.0, 75.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(humidityVar, "High", 70.0, 85.0, 100.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Rain Membership Functions
    result = irrigationController.addTrapezoidalMF(rainVar, "NoRain", 0.0, 0.0, 0.2, 0.4);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(rainVar, "Raining", 0.6, 0.8, 1.0, 1.0);
    if (result != FUZZY_OK) return false;

    // Plant Type Membership Functions
    result = irrigationController.addTrapezoidalMF(plantTypeVar, "DroughtTolerant", 0.0, 0.0, 0.3, 0.5);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(plantTypeVar, "WaterIntensive", 0.5, 0.7, 1.0, 1.0);
    if (result != FUZZY_OK) return false;

    // Irrigation Output Membership Functions
    result = irrigationController.addTrapezoidalMF(irrigationVar, "None", 0.0, 0.0, 5.0, 10.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(irrigationVar, "Light", 8.0, 20.0, 35.0, 50.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(irrigationVar, "Moderate", 40.0, 55.0, 70.0, 80.0);
    if (result != FUZZY_OK) return false;

    result = irrigationController.addTrapezoidalMF(irrigationVar, "Heavy", 75.0, 85.0, 95.0, 100.0);
    if (result != FUZZY_OK) return false;

    // Fuzzy Rules for Irrigation Control

    // Emergency Rules - Skip irrigation when raining
    result = irrigationController.addRule("Rain", "Raining", "Irrigation", "None");
    if (result != FUZZY_OK) return false;

    // Very Dry Soil Rules (High Priority)
    AutoFuzzy::Antecedent veryDrySoilAntecedents[] = {
        {soilMoistureVar, irrigationController.findMF(soilMoistureVar, "VeryDry")},
        {airTempVar, irrigationController.findMF(airTempVar, "Hot")},
        {humidityVar, irrigationController.findMF(humidityVar, "Low")}
    };
    AutoFuzzy::Consequent heavyIrrigationConsequent = {
        irrigationVar, irrigationController.findMF(irrigationVar, "Heavy")
    };
    result = irrigationController.addRule(veryDrySoilAntecedents, 3, FUZZY_AND, heavyIrrigationConsequent);
    if (result != FUZZY_OK) return false;

    // Dry Soil with Hot Weather
    AutoFuzzy::Antecedent dryHotAntecedents[] = {
        {soilMoistureVar, irrigationController.findMF(soilMoistureVar, "Dry")},
        {airTempVar, irrigationController.findMF(airTempVar, "Hot")}
    };
    AutoFuzzy::Consequent moderateIrrigationConsequent = {
        irrigationVar, irrigationController.findMF(irrigationVar, "Moderate")
    };
    result = irrigationController.addRule(dryHotAntecedents, 2, FUZZY_AND, moderateIrrigationConsequent);
    if (result != FUZZY_OK) return false;

    // Optimal Conditions - Light watering
    AutoFuzzy::Antecedent optimalConditionsAntecedents[] = {
        {soilMoistureVar, irrigationController.findMF(soilMoistureVar, "Dry")},
        {airTempVar, irrigationController.findMF(airTempVar, "Moderate")},
        {humidityVar, irrigationController.findMF(humidityVar, "Moderate")}
    };
    AutoFuzzy::Consequent lightIrrigationConsequent = {
        irrigationVar, irrigationController.findMF(irrigationVar, "Light")
    };
    result = irrigationController.addRule(optimalConditionsAntecedents, 3, FUZZY_AND, lightIrrigationConsequent);
    if (result != FUZZY_OK) return false;

    // Plant Type Adjustments
    AutoFuzzy::Antecedent droughtTolerantAntecedents[] = {
        {plantTypeVar, irrigationController.findMF(plantTypeVar, "DroughtTolerant")},
        {soilMoistureVar, irrigationController.findMF(soilMoistureVar, "Dry")}
    };
    result = irrigationController.addRule(droughtTolerantAntecedents, 2, FUZZY_AND, lightIrrigationConsequent);
    if (result != FUZZY_OK) return false;

    // Wet Soil Rules - No irrigation needed
    result = irrigationController.addRule("SoilMoisture", "Wet", "Irrigation", "None");
    if (result != FUZZY_OK) return false;

    return true;
}

/**
 * Read soil moisture level (simulated)
 */
float readSoilMoisture() {
    int sensorValue = analogRead(SOIL_MOISTURE_PIN);
    // Convert to percentage (0-100%)
    // Note: Real sensors may need calibration
    return map(sensorValue, 0, 1023, 100, 0); // Inverted for capacitive sensors
}

/**
 * Read air temperature (simulated)
 */
float readAirTemperature() {
    int sensorValue = analogRead(TEMP_HUMIDITY_PIN);
    // Convert to temperature (10-45°C range)
    return map(sensorValue, 0, 1023, 100, 450) / 10.0;
}

/**
 * Read air humidity (simulated)
 */
float readAirHumidity() {
    // In real system, this would come from DHT sensor
    // Simulated based on temperature (higher temp = lower humidity)
    float temperature = readAirTemperature();
    return constrain(100.0 - (temperature - 10.0) * 2.0, 20.0, 90.0);
}

/**
 * Check for rain (simulated)
 */
float checkRain() {
    int sensorValue = analogRead(RAIN_SENSOR_PIN);
    // Convert to binary rain detection (0=no rain, 1=rain)
    return (sensorValue < 500) ? 1.0 : 0.0; // Threshold-based detection
}

/**
 * Get plant type setting (simulated - in real system use DIP switches or menu)
 */
float getPlantType() {
    // Simulate plant type selection
    // 0.0 = drought tolerant, 1.0 = water intensive
    // In real system, read from digital inputs or EEPROM
    return 0.5; // Moderate default
}

/**
 * Apply irrigation control
 */
void applyIrrigationControl(float irrigationOutput) {
    // Convert percentage to PWM value for valve control
    int pwmValue = map(irrigationOutput, 0, 100, 0, 255);

    if (pwmValue > 10) { // Threshold to avoid valve chatter
        if (!irrigationActive) {
            // Starting irrigation
            irrigationActive = true;
            irrigationStartTime = millis();
            Serial.println("🌿 Starting irrigation cycle");
        }
        analogWrite(VALVE_CONTROL_PIN, pwmValue);
        digitalWrite(STATUS_LED_PIN, HIGH); // Irrigation active indicator
    } else {
        if (irrigationActive) {
            // Stopping irrigation
            irrigationActive = false;
            float duration = (millis() - irrigationStartTime) / 1000.0;
            waterUsageTotal += duration * (irrigationOutput / 100.0); // Estimate water usage
            Serial.print("🌿 Irrigation cycle completed - Duration: ");
            Serial.print(duration, 1);
            Serial.println(" seconds");
        }
        analogWrite(VALVE_CONTROL_PIN, 0);
        digitalWrite(STATUS_LED_PIN, LOW);
    }
}

/**
 * Safety check for irrigation system
 */
bool checkIrrigationSafety(float irrigationOutput, unsigned long currentTime) {
    // Maximum irrigation time protection
    if (irrigationActive) {
        float elapsedTime = (currentTime - irrigationStartTime) / 1000.0;
        if (elapsedTime > MAX_IRRIGATION_TIME) {
            Serial.println("WARNING: Maximum irrigation time exceeded!");
            return false;
        }
    }

    // Soil saturation protection
    float moisture = readSoilMoisture();
    if (moisture > 95.0 && irrigationOutput > 50.0) {
        Serial.println("WARNING: Soil may be over-saturated!");
        return false;
    }

    return true;
}

/**
 * Monitor water usage and efficiency
 */
void monitorWaterEfficiency(float moistureLevel, float irrigationOutput, bool raining) {
    static unsigned long lastReport = 0;
    static float moistureHistory[10] = {0};
    static int historyIndex = 0;

    // Store moisture history for trend analysis
    moistureHistory[historyIndex] = moistureLevel;
    historyIndex = (historyIndex + 1) % 10;

    // Report efficiency every 60 seconds
    if (millis() - lastReport > 60000) {
        Serial.print("Water Efficiency Report - Total Usage: ");
        Serial.print(waterUsageTotal, 2);
        Serial.print(" liters, Current Moisture: ");
        Serial.print(moistureLevel, 1);
        Serial.print("%, Irrigation Output: ");
        Serial.print(irrigationOutput, 1);
        Serial.println("%");

        if (raining && irrigationOutput > 20.0) {
            Serial.println("⚠️ Irrigating during rain - consider adjusting rules");
        }

        if (abs(moistureLevel - OPTIMAL_MOISTURE) > MOISTURE_TOLERANCE) {
            Serial.println("⚠️ Soil moisture outside optimal range");
        }

        lastReport = millis();
    }
}

/**
 * Display irrigation system status
 */
void displayIrrigationStatus(float moisture, float temperature, float humidity, float rain, float irrigationOutput) {
    Serial.print("Irrigation Status - Moisture: ");
    Serial.print(moisture, 1);
    Serial.print("%, Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity, 1);
    Serial.print("%, Rain: ");
    Serial.print(rain > 0.5 ? "Yes" : "No");
    Serial.print(", Irrigation: ");
    Serial.print(irrigationOutput, 1);
    Serial.println("%");

    // Irrigation recommendations
    if (abs(moisture - OPTIMAL_MOISTURE) <= MOISTURE_TOLERANCE) {
        Serial.println("✓ Soil moisture within optimal range");
    } else if (moisture < OPTIMAL_MOISTURE - MOISTURE_TOLERANCE) {
        Serial.println("💧 Soil moisture below optimal - irrigation recommended");
    } else {
        Serial.println("🚫 Soil moisture above optimal - irrigation not needed");
    }

    if (rain > 0.5 && irrigationOutput > 10.0) {
        Serial.println("🌧️ Raining detected - natural irrigation occurring");
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("=== AutoFuzzy Smart Irrigation Control System ===");
    Serial.print("Target Moisture: ");
    Serial.print(OPTIMAL_MOISTURE, 1);
    Serial.println("%");
    Serial.print("Maximum Irrigation Time: ");
    Serial.print(MAX_IRRIGATION_TIME, 1);
    Serial.println(" seconds");

    // Initialize pins
    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(TEMP_HUMIDITY_PIN, INPUT);
    pinMode(RAIN_SENSOR_PIN, INPUT);
    pinMode(VALVE_CONTROL_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Ensure valve is closed initially
    analogWrite(VALVE_CONTROL_PIN, 0);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize fuzzy controller
    if (!initializeIrrigationControl()) {
        Serial.println("ERROR: Failed to initialize irrigation control system!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(500);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(500);
        }
    }

    Serial.println("Irrigation control system initialized successfully");
    Serial.println("Starting intelligent irrigation management...");

    irrigationStartTime = millis();
}

void loop() {
    static unsigned long lastControlTime = 0;
    unsigned long currentTime = millis();

    // Control loop timing
    if (currentTime - lastControlTime >= CONTROL_INTERVAL) {
        // Read environmental parameters
        float soilMoisture = readSoilMoisture();
        float airTemperature = readAirTemperature();
        float airHumidity = readAirHumidity();
        float rainStatus = checkRain();
        float plantType = getPlantType();

        // Safety check
        if (!checkIrrigationSafety(0.0, currentTime)) { // Check current state
            applyIrrigationControl(0.0); // Emergency stop
            Serial.println("EMERGENCY STOP: Safety violation detected");
            delay(5000); // Wait before retry
            return;
        }

        // Prepare fuzzy inputs
        FuzzyInput inputs[] = {
            {soilMoistureVar, soilMoisture},
            {airTempVar, airTemperature},
            {humidityVar, airHumidity},
            {rainVar, rainStatus},
            {plantTypeVar, plantType}
        };

        // Evaluate fuzzy control output
        float irrigationOutput;
        FuzzyResult result = irrigationController.evaluate(inputs, 5, irrigationOutput, irrigationVar);

        if (result == FUZZY_OK) {
            // Apply plant type adjustments
            if (plantType < 0.4) { // Drought tolerant
                irrigationOutput *= DROUGHT_TOLERANT_FACTOR;
            } else if (plantType > 0.6) { // Water intensive
                irrigationOutput *= WATER_INTENSIVE_FACTOR;
            }

            // Safety check on adjusted output
            if (checkIrrigationSafety(irrigationOutput, currentTime)) {
                applyIrrigationControl(irrigationOutput);
                displayIrrigationStatus(soilMoisture, airTemperature, airHumidity, rainStatus, irrigationOutput);

                // Monitor water efficiency
                monitorWaterEfficiency(soilMoisture, irrigationOutput, rainStatus > 0.5);
            } else {
                applyIrrigationControl(0.0);
                Serial.println("Irrigation output rejected by safety check");
            }
        } else {
            // Error handling - safe state
            applyIrrigationControl(0.0);
            Serial.print("Irrigation control evaluation error: ");
            Serial.println(irrigationController.getResultString(result));
        }

        lastControlTime = currentTime;
    }

    // Small delay for stability
    delay(200);
}
