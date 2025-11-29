// AutoFuzzy Intermediate Example: Fan Speed Control
// Controls fan speed based on temperature and humidity.

#include <AutoFuzzy.h>
// #include <DHT.h> // Uncomment if using a DHT sensor

// --- Pin Definitions & Sensor Setup ---
// const int dhtPin = 2;       // Pin connected to DHT sensor data line
// const int fanPin = 9;       // PWM pin for fan control (e.g., LED)
// #define DHTTYPE DHT11     // Or DHT22, DHT21
// DHT dht(dhtPin, DHTTYPE); // Uncomment if using a DHT sensor

// --- Fuzzy Logic Object ---
AutoFuzzy fuzzy; // Create an instance

// --- Variable Indices ---
// Store indices for better readability and management
int tempVar = -1;
int humidityVar = -1;
int fanSpeedVar = -1;

void setup()
{
    // --- Initialize Serial Monitor & Sensor ---
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println("AutoFuzzy Intermediate Example: Fan Speed Control");

    // dht.begin(); // Uncomment if using a DHT sensor
    // pinMode(fanPin, OUTPUT); // Uncomment if controlling a device like an LED

    // --- Configure Fuzzy Logic System ---
    Serial.println("Configuring Fuzzy System...");
    FuzzyResult result;

    // 1. Add Input Variables
    tempVar = fuzzy.addInput("Temperature", 0.0f, 50.0f); // Range 0-50 Celsius
    if (tempVar < 0) {
        Serial.println("Error adding 'Temperature'");
        while (1)
            ;
    }
    humidityVar = fuzzy.addInput("Humidity", 0.0f, 100.0f); // Range 0-100 %RH
    if (humidityVar < 0) {
        Serial.println("Error adding 'Humidity'");
        while (1)
            ;
    }
    Serial.println(" - Inputs 'Temperature' (Idx " + String(tempVar) + "), 'Humidity' (Idx " + String(humidityVar) + ") added.");

    // 2. Add Output Variable
    fanSpeedVar = fuzzy.addOutput("FanSpeed", 0.0f, 100.0f); // Range 0-100 %
    if (fanSpeedVar < 0) {
        Serial.println("Error adding 'FanSpeed'");
        while (1)
            ;
    }
    Serial.println(" - Output 'FanSpeed' (Idx " + String(fanSpeedVar) + ") added.");

    // 3. Add Membership Functions (MFs)
    // Using Triangular MFs for memory efficiency: addTriangularMF(var, name, a, b, c)
    // Temperature MFs
    result = fuzzy.addTriangularMF(tempVar, "Cold", 0.0f, 15.0f, 25.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Cold': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(tempVar, "Warm", 20.0f, 30.0f, 40.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Warm': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(tempVar, "Hot", 35.0f, 45.0f, 50.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Hot': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Temperature' added.");

    // Humidity MFs
    result = fuzzy.addTriangularMF(humidityVar, "Dry", 0.0f, 25.0f, 50.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Dry': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(humidityVar, "Normal", 40.0f, 60.0f, 80.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Normal': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(humidityVar, "Humid", 70.0f, 85.0f, 100.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Humid': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Humidity' added.");

    // Fan Speed MFs
    result = fuzzy.addTriangularMF(fanSpeedVar, "Slow", 0.0f, 20.0f, 40.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Slow': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(fanSpeedVar, "Medium", 30.0f, 50.0f, 70.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Medium': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF(fanSpeedVar, "Fast", 60.0f, 80.0f, 100.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Fast': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'FanSpeed' added.");

    // 4. Add Fuzzy Rules (using Antecedent/Consequent structs for multi-input rules)
    // Get MF indices needed for rules (more robust than using names repeatedly)
    int tempColdMf = fuzzy.findMF(tempVar, "Cold");
    int tempWarmMf = fuzzy.findMF(tempVar, "Warm");
    int tempHotMf = fuzzy.findMF(tempVar, "Hot");
    int humDryMf = fuzzy.findMF(humidityVar, "Dry");
    int humHumidMf = fuzzy.findMF(humidityVar, "Humid");
    int fanSlowMf = fuzzy.findMF(fanSpeedVar, "Slow");
    int fanMediumMf = fuzzy.findMF(fanSpeedVar, "Medium");
    int fanFastMf = fuzzy.findMF(fanSpeedVar, "Fast");

    // Rule 1: IF Temperature IS Cold THEN FanSpeed IS Slow (Single antecedent)
    AutoFuzzy::Antecedent rule1_if = { tempVar, tempColdMf };
    AutoFuzzy::Consequent rule1_then = { fanSpeedVar, fanSlowMf };
    result = fuzzy.addRule(rule1_if, rule1_then); // Use the simple overload for SISO rules
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 1: " + String(fuzzy.getResultString(result)));

    // Rule 2: IF Temperature IS Warm AND Humidity IS Humid THEN FanSpeed IS Medium
    AutoFuzzy::Antecedent rule2_ifs[] = { { tempVar, tempWarmMf }, { humidityVar, humHumidMf } };
    AutoFuzzy::Consequent rule2_then = { fanSpeedVar, fanMediumMf };
    result = fuzzy.addRule(rule2_ifs, 2, FUZZY_AND, rule2_then); // Use the full version for multi-antecedent
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 2: " + String(fuzzy.getResultString(result)));

    // Rule 3: IF Temperature IS Hot THEN FanSpeed IS Fast
    AutoFuzzy::Antecedent rule3_if = { tempVar, tempHotMf };
    AutoFuzzy::Consequent rule3_then = { fanSpeedVar, fanFastMf };
    result = fuzzy.addRule(rule3_if, rule3_then);
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 3: " + String(fuzzy.getResultString(result)));

    Serial.println(" - Fuzzy Rules added.");

    // --- Optional: Auto Tuning Placeholder ---
    // autoTune requires training data (input sets and expected outputs).
    // Here's a conceptual placeholder showing the structure:
    /*
    Serial.println("Running AutoTune (heuristic optimization)...");
    const int NUM_TRAINING_SETS = 3;
    // Example training inputs [ {temp1, hum1}, {temp2, hum2}, ... ]
    float train_inputs_data[NUM_TRAINING_SETS][2] = {
        {15.0, 60.0}, // Cold, Normal -> Expect Slow
        {28.0, 50.0}, // Comfortable/Hot border, Normal -> Expect Medium
        {38.0, 80.0}  // Hot, Humid -> Expect Fast
    };
    // Need to convert to array of pointers for the function
    float* train_inputs[NUM_TRAINING_SETS];
    for(int i=0; i<NUM_TRAINING_SETS; ++i) train_inputs[i] = train_inputs_data[i];

    // Example training outputs (expected fan speeds for the inputs above)
    float train_outputs[NUM_TRAINING_SETS] = {
        20.0, // Expected slow fan speed %
        60.0, // Expected medium fan speed %
        90.0  // Expected fast fan speed %
    };

    // Run the tuning process for the fanSpeedVar output
    // autoTune(trainingInputs, trainingOutputs, numSets, outputVarIndex, iterations, mutationRate, mutationRange)
    result = fuzzy.autoTune(train_inputs, train_outputs, NUM_TRAINING_SETS, fanSpeedVar, 500, 0.1f, 0.1f);

    if (result == FUZZY_OK) {
        Serial.println("AutoTune completed.");
    } else {
        Serial.println("AutoTune failed: " + String(fuzzy.getResultString(result)));
    }
    // Note: Effective tuning requires *good* and *representative* training data.
    // The results are heuristic - they improve parameters but don't guarantee optimality.
    */

    Serial.println("Fuzzy System Configuration Complete.\n");
}

void loop()
{
    // --- Read Sensor Inputs ---
    // Use placeholder values if no sensor is connected
    float currentTemp = 28.5; // Placeholder Celsius
    float currentHumidity = 65.0; // Placeholder %RH

    /* // Uncomment if using DHT sensor
    currentHumidity = dht.readHumidity();
    currentTemp = dht.readTemperature(); // Read temperature as Celsius (the default)

    // Check if any reads failed and exit early (to try again).
    if (isnan(currentHumidity) || isnan(currentTemp)) {
        Serial.println("Failed to read from DHT sensor!");
        delay(2000); // Wait before retrying
        return;
    }
    */

    // --- Prepare Fuzzy Inputs ---
    // Create an array of FuzzyInput structs for *all* inputs
    FuzzyInput fuzzyInputs[] = {
        { tempVar, currentTemp }, // Pair temperature index with its value
        { humidityVar, currentHumidity } // Pair humidity index with its value
    };
    int numInputs = 2; // Number of input pairs in the array

    // --- Evaluate Fuzzy Logic ---
    float calculatedFanSpeed; // Variable for the output
    // Evaluate for the fanSpeedVar output index
    FuzzyResult result = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedFanSpeed, fanSpeedVar);

    // --- Process Result ---
    Serial.print("Temp: " + String(currentTemp, 1) + " C, ");
    Serial.print("Humidity: " + String(currentHumidity, 1) + " %\t => ");

    if (result == FUZZY_OK) {
        Serial.println("Calculated Fan Speed: " + String(calculatedFanSpeed, 1) + " %");
        // --- Apply Output ---
        // int fanPWM = map(calculatedFanSpeed, 0, 100, 0, 255); // Map % to PWM
        // analogWrite(fanPin, fanPWM); // Uncomment to control device
    } else if (result == FUZZY_ERROR_NO_RULES_FIRED) {
        Serial.println("Warning: No rules fired. Setting fan slow.");
        // analogWrite(fanPin, 50); // Default action (e.g., low speed)
    } else {
        Serial.println("Error during fuzzy evaluation: " + String(fuzzy.getResultString(result)));
        // analogWrite(fanPin, 0); // Default safe action
    }

    // --- Delay ---
    delay(2000); // Wait 2 seconds
}
