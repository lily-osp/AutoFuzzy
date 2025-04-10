// AutoFuzzy Advanced Example: Greenhouse Climate Control
// Controls both a heater and a ventilator based on temperature and humidity.

#include <AutoFuzzy.h>
// #include <DHT.h> // Uncomment if using a real sensor

// --- Pin Definitions & Sensor/Actuator Setup ---
// const int dhtPin = 2;
// const int heaterPin = 9;  // PWM pin for heater representation (e.g., red LED)
// const int ventPin = 10; // PWM pin for ventilator representation (e.g., blue LED)
// #define DHTTYPE DHT22
// DHT dht(dhtPin, DHTTYPE); // Uncomment for real sensor

// --- Fuzzy Logic Object ---
AutoFuzzy fuzzy;

// --- Variable Indices ---
int tempVar = -1;
int humidityVar = -1;
int heaterVar = -1;
int ventVar = -1;

// --- MF Indices (store for complex rule setup) ---
// Temp
int tempVeryColdMf = -1, tempColdMf = -1, tempOptimalMf = -1, tempWarmMf = -1, tempHotMf = -1;
// Humidity
int humVeryDryMf = -1, humDryMf = -1, humOptimalMf = -1, humHumidMf = -1, humVeryHumidMf = -1;
// Heater
int heatOffMf = -1, heatLowMf = -1, heatHighMf = -1;
// Ventilator
int ventOffMf = -1, ventLowMf = -1, ventHighMf = -1;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println("AutoFuzzy Advanced Example: Greenhouse Control");

    // dht.begin(); // Uncomment for real sensor
    // pinMode(heaterPin, OUTPUT); // Uncomment for actuators
    // pinMode(ventPin, OUTPUT);   // Uncomment for actuators

    // --- Configure Fuzzy Logic System ---
    Serial.println("Configuring Fuzzy System...");
    FuzzyResult result;

    // 1. Add Input Variables
    tempVar = fuzzy.addInput("Temperature", 0.0f, 50.0f);
    if (tempVar < 0) {
        Serial.println("Error adding 'Temperature'");
        while (1)
            ;
    }
    humidityVar = fuzzy.addInput("Humidity", 0.0f, 100.0f);
    if (humidityVar < 0) {
        Serial.println("Error adding 'Humidity'");
        while (1)
            ;
    }
    Serial.println(" - Inputs Added.");

    // 2. Add Output Variables (MIMO)
    heaterVar = fuzzy.addOutput("HeaterPower", 0.0f, 100.0f); // Heater power %
    if (heaterVar < 0) {
        Serial.println("Error adding 'HeaterPower'");
        while (1)
            ;
    }
    ventVar = fuzzy.addOutput("VentSpeed", 0.0f, 100.0f); // Ventilator speed %
    if (ventVar < 0) {
        Serial.println("Error adding 'VentSpeed'");
        while (1)
            ;
    }
    Serial.println(" - Outputs Added.");

    // 3. Add Membership Functions (Using indices directly for adding)
    // Temperature MFs (Mix of Triangular/Trapezoidal)
    result = fuzzy.addTriangularMF(tempVar, "VeryCold", 0.0f, 0.0f, 10.0f);
    tempVeryColdMf = fuzzy.findMF(tempVar, "VeryCold");
    result = fuzzy.addTrapezoidalMF(tempVar, "Cold", 8.0f, 12.0f, 16.0f, 20.0f);
    tempColdMf = fuzzy.findMF(tempVar, "Cold");
    result = fuzzy.addTrapezoidalMF(tempVar, "Optimal", 18.0f, 22.0f, 26.0f, 30.0f);
    tempOptimalMf = fuzzy.findMF(tempVar, "Optimal");
    result = fuzzy.addTrapezoidalMF(tempVar, "Warm", 28.0f, 32.0f, 36.0f, 40.0f);
    tempWarmMf = fuzzy.findMF(tempVar, "Warm");
    result = fuzzy.addTriangularMF(tempVar, "Hot", 38.0f, 50.0f, 50.0f);
    tempHotMf = fuzzy.findMF(tempVar, "Hot");
    Serial.println(" - Temperature MFs Added.");

    // Humidity MFs
    result = fuzzy.addTriangularMF(humidityVar, "VeryDry", 0.0f, 0.0f, 20.0f);
    humVeryDryMf = fuzzy.findMF(humidityVar, "VeryDry");
    result = fuzzy.addTrapezoidalMF(humidityVar, "Dry", 15.0f, 25.0f, 35.0f, 45.0f);
    humDryMf = fuzzy.findMF(humidityVar, "Dry");
    result = fuzzy.addTrapezoidalMF(humidityVar, "Optimal", 40.0f, 50.0f, 60.0f, 70.0f);
    humOptimalMf = fuzzy.findMF(humidityVar, "Optimal");
    result = fuzzy.addTrapezoidalMF(humidityVar, "Humid", 65.0f, 75.0f, 85.0f, 95.0f);
    humHumidMf = fuzzy.findMF(humidityVar, "Humid");
    result = fuzzy.addTriangularMF(humidityVar, "VeryHumid", 90.0f, 100.0f, 100.0f);
    humVeryHumidMf = fuzzy.findMF(humidityVar, "VeryHumid");
    Serial.println(" - Humidity MFs Added.");

    // Heater Power MFs
    result = fuzzy.addTriangularMF(heaterVar, "Off", 0.0f, 0.0f, 20.0f);
    heatOffMf = fuzzy.findMF(heaterVar, "Off");
    result = fuzzy.addTrapezoidalMF(heaterVar, "Low", 10.0f, 30.0f, 50.0f, 70.0f);
    heatLowMf = fuzzy.findMF(heaterVar, "Low");
    result = fuzzy.addTriangularMF(heaterVar, "High", 60.0f, 100.0f, 100.0f);
    heatHighMf = fuzzy.findMF(heaterVar, "High");
    Serial.println(" - Heater MFs Added.");

    // Ventilator Speed MFs
    result = fuzzy.addTriangularMF(ventVar, "Off", 0.0f, 0.0f, 20.0f);
    ventOffMf = fuzzy.findMF(ventVar, "Off");
    result = fuzzy.addTrapezoidalMF(ventVar, "Low", 10.0f, 30.0f, 50.0f, 70.0f);
    ventLowMf = fuzzy.findMF(ventVar, "Low");
    result = fuzzy.addTriangularMF(ventVar, "High", 60.0f, 100.0f, 100.0f);
    ventHighMf = fuzzy.findMF(ventVar, "High");
    Serial.println(" - Ventilator MFs Added.");

    // Check if all MF indices were found (basic check)
    if (tempVeryColdMf < 0 || humOptimalMf < 0 || heatLowMf < 0 || ventHighMf < 0) {
        Serial.println("Error: Could not find all required MF indices!");
        while (1)
            ;
    }

    // 4. Add Fuzzy Rules (MIMO - Rules may affect one or both outputs)
    // Note: A single logical condition (IF...) often translates to multiple fuzzy rules,
    // one for each output variable affected by that condition.

    // Rule Set 1: IF Temp is VeryCold THEN Heater is High, Vent is Off
    AutoFuzzy::Antecedent rs1_if = { tempVar, tempVeryColdMf };
    AutoFuzzy::Consequent rs1_then_heat = { heaterVar, heatHighMf };
    AutoFuzzy::Consequent rs1_then_vent = { ventVar, ventOffMf };
    result = fuzzy.addRule(rs1_if, rs1_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R1H");
    result = fuzzy.addRule(rs1_if, rs1_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R1V");

    // Rule Set 2: IF Temp is Cold THEN Heater is Low, Vent is Off
    AutoFuzzy::Antecedent rs2_if = { tempVar, tempColdMf };
    AutoFuzzy::Consequent rs2_then_heat = { heaterVar, heatLowMf };
    AutoFuzzy::Consequent rs2_then_vent = { ventVar, ventOffMf };
    result = fuzzy.addRule(rs2_if, rs2_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R2H");
    result = fuzzy.addRule(rs2_if, rs2_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R2V");

    // Rule Set 3: IF Temp is Optimal AND Humidity is Optimal THEN Heater is Off, Vent is Off
    AutoFuzzy::Antecedent rs3_ifs[] = { { tempVar, tempOptimalMf }, { humidityVar, humOptimalMf } };
    AutoFuzzy::Consequent rs3_then_heat = { heaterVar, heatOffMf };
    AutoFuzzy::Consequent rs3_then_vent = { ventVar, ventOffMf };
    result = fuzzy.addRule(rs3_ifs, 2, FUZZY_AND, rs3_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R3H");
    result = fuzzy.addRule(rs3_ifs, 2, FUZZY_AND, rs3_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R3V");

    // Rule Set 4: IF Temp is Warm OR Humidity is Humid THEN Heater is Off, Vent is Low (Using OR)
    AutoFuzzy::Antecedent rs4_ifs[] = { { tempVar, tempWarmMf }, { humidityVar, humHumidMf } }; // Define conditions
    AutoFuzzy::Consequent rs4_then_heat = { heaterVar, heatOffMf };
    AutoFuzzy::Consequent rs4_then_vent = { ventVar, ventLowMf };
    // Apply the same OR condition to both outputs
    result = fuzzy.addRule(rs4_ifs, 2, FUZZY_OR, rs4_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R4H"); // Use FUZZY_OR
    result = fuzzy.addRule(rs4_ifs, 2, FUZZY_OR, rs4_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R4V"); // Use FUZZY_OR

    // Rule Set 5: IF Temp is Hot THEN Heater is Off, Vent is High
    AutoFuzzy::Antecedent rs5_if = { tempVar, tempHotMf };
    AutoFuzzy::Consequent rs5_then_heat = { heaterVar, heatOffMf };
    AutoFuzzy::Consequent rs5_then_vent = { ventVar, ventHighMf };
    result = fuzzy.addRule(rs5_if, rs5_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R5H");
    result = fuzzy.addRule(rs5_if, rs5_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R5V");

    // Rule Set 6: IF Humidity is VeryHumid THEN Heater is Off, Vent is High
    AutoFuzzy::Antecedent rs6_if = { humidityVar, humVeryHumidMf };
    AutoFuzzy::Consequent rs6_then_heat = { heaterVar, heatOffMf };
    AutoFuzzy::Consequent rs6_then_vent = { ventVar, ventHighMf };
    result = fuzzy.addRule(rs6_if, rs6_then_heat);
    if (result != FUZZY_OK)
        Serial.println("Err R6H");
    result = fuzzy.addRule(rs6_if, rs6_then_vent);
    if (result != FUZZY_OK)
        Serial.println("Err R6V");

    Serial.println(" - Fuzzy Rules Added.");

    // --- Optional: Auto Tuning Setup (More Detailed Placeholder) ---
    /*
    Serial.println("Setting up AutoTune...");
    const int NUM_TRAIN_SETS_HEATER = 4; // Example: Tune heater first
    // Training data: Input pairs {temp, humidity}
    float heater_train_inputs_data[NUM_TRAIN_SETS_HEATER][2] = {
        { 5.0, 50.0}, // Very Cold, Optimal Hum -> Expect High Heat
        {15.0, 60.0}, // Cold, Optimal Hum      -> Expect Low Heat
        {25.0, 55.0}, // Optimal Temp/Hum      -> Expect Off Heat
        {35.0, 80.0}  // Warm, Humid           -> Expect Off Heat
    };
    float* heater_train_inputs[NUM_TRAIN_SETS_HEATER];
    for(int i=0; i<NUM_TRAIN_SETS_HEATER; ++i) heater_train_inputs[i] = heater_train_inputs_data[i];

    // Expected heater outputs for the above inputs
    float heater_train_outputs[NUM_TRAIN_SETS_HEATER] = { 95.0, 40.0, 5.0, 0.0 }; // Target % power

    Serial.println("Running AutoTune for Heater...");
    // Tune parameters associated with the heaterVar output
    result = fuzzy.autoTune(heater_train_inputs, heater_train_outputs, NUM_TRAIN_SETS_HEATER, heaterVar, 1000, 0.08f, 0.08f);
    if (result == FUZZY_OK) {
        Serial.println("Heater AutoTune completed.");
    } else {
        Serial.println("Heater AutoTune failed: " + String(fuzzy.getResultString(result)));
    }

    // --- IMPORTANT ---
    // You would typically need separate training data and tuning runs for EACH output variable (heater, ventilator),
    // as their ideal parameters might differ. Tuning them simultaneously based on a combined
    // error metric is possible but more complex (multi-objective optimization).
    // This simple example only shows tuning for one output.
    */

    Serial.println("Fuzzy System Configuration Complete.\n");
}

void loop()
{
    // --- Read Sensor Inputs ---
    // Placeholder values if no sensor
    float currentTemp = 15.0; // Example: Cold
    float currentHumidity = 80.0; // Example: Humid

    /* // Uncomment for real DHT sensor
    currentHumidity = dht.readHumidity();
    currentTemp = dht.readTemperature();
    if (isnan(currentHumidity) || isnan(currentTemp)) {
        Serial.println("Failed to read from DHT sensor!");
        delay(2000);
        return;
    }
    */

    // --- Prepare Fuzzy Inputs (same inputs for both evaluations) ---
    FuzzyInput fuzzyInputs[] = {
        { tempVar, currentTemp },
        { humidityVar, currentHumidity }
    };
    int numInputs = 2;

    // --- Evaluate Fuzzy Logic (MIMO requires separate calls per output) ---
    float calculatedHeaterPower;
    float calculatedVentSpeed;
    FuzzyResult heaterResult, ventResult;

    // Evaluate for Heater Output (using heaterVar index)
    heaterResult = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedHeaterPower, heaterVar);

    // Evaluate for Ventilator Output (using ventVar index)
    ventResult = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedVentSpeed, ventVar);

    // --- Process Results ---
    Serial.print("Temp: " + String(currentTemp, 1) + " C, ");
    Serial.print("Humidity: " + String(currentHumidity, 1) + " %\t => ");

    // Process Heater Result
    if (heaterResult == FUZZY_OK) {
        Serial.print("Heater: " + String(calculatedHeaterPower, 1) + " %");
        // int heaterPWM = map(calculatedHeaterPower, 0, 100, 0, 255);
        // analogWrite(heaterPin, heaterPWM); // Uncomment for actuator
    } else {
        Serial.print("Heater Err: " + String(fuzzy.getResultString(heaterResult)));
        // analogWrite(heaterPin, 0); // Safe state
    }

    Serial.print(" | ");

    // Process Ventilator Result
    if (ventResult == FUZZY_OK) {
        Serial.println("Vent: " + String(calculatedVentSpeed, 1) + " %");
        // int ventPWM = map(calculatedVentSpeed, 0, 100, 0, 255);
        // analogWrite(ventPin, ventPWM); // Uncomment for actuator
    } else {
        Serial.println("Vent Err: " + String(fuzzy.getResultString(ventResult)));
        // analogWrite(ventPin, 0); // Safe state
    }

    // --- Delay ---
    delay(3000); // Wait 3 seconds
}
