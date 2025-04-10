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
    // Using Trapezoidal MFs for smooth transitions: addTrapezoidalMF(var, name, a, b, c, d)
    // Temperature MFs
    result = fuzzy.addTrapezoidalMF(tempVar, "Cold", 0.0f, 5.0f, 15.0f, 20.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Cold': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(tempVar, "Comfortable", 18.0f, 22.0f, 26.0f, 30.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Comfortable': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(tempVar, "Hot", 28.0f, 35.0f, 45.0f, 50.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Hot': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Temperature' added.");

    // Humidity MFs
    result = fuzzy.addTrapezoidalMF(humidityVar, "Dry", 0.0f, 10.0f, 25.0f, 40.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Dry': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(humidityVar, "Normal", 30.0f, 45.0f, 55.0f, 70.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Normal': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(humidityVar, "Humid", 60.0f, 75.0f, 90.0f, 100.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Humid': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Humidity' added.");

    // Fan Speed MFs
    result = fuzzy.addTrapezoidalMF(fanSpeedVar, "Slow", 0.0f, 10.0f, 25.0f, 40.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Slow': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(fanSpeedVar, "Medium", 30.0f, 45.0f, 55.0f, 70.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Medium': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTrapezoidalMF(fanSpeedVar, "Fast", 60.0f, 80.0f, 95.0f, 100.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Fast': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'FanSpeed' added.");

    // 4. Add Fuzzy Rules (using Antecedent/Consequent structs for multi-input rules)
    // Get MF indices needed for rules (more robust than using names repeatedly)
    int tempColdMf = fuzzy.findMF(tempVar, "Cold");
    int tempComfortMf = fuzzy.findMF(tempVar, "Comfortable");
    int tempHotMf = fuzzy.findMF(tempVar, "Hot");
    int humDryMf = fuzzy.findMF(humidityVar, "Dry");
    int humNormalMf = fuzzy.findMF(humidityVar, "Normal");
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

    // Rule 2: IF Temperature IS Comfortable AND Humidity IS Normal THEN FanSpeed IS Slow
    AutoFuzzy::Antecedent rule2_ifs[] = { { tempVar, tempComfortMf }, { humidityVar, humNormalMf } };
    AutoFuzzy::Consequent rule2_then = { fanSpeedVar, fanSlowMf };
    result = fuzzy.addRule(rule2_ifs, 2, FUZZY_AND, rule2_then); // Use the full version for multi-antecedent
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 2: " + String(fuzzy.getResultString(result)));

    // Rule 3: IF Temperature IS Hot AND Humidity IS Normal THEN FanSpeed IS Medium
    AutoFuzzy::Antecedent rule3_ifs[] = { { tempVar, tempHotMf }, { humidityVar, humNormalMf } };
    AutoFuzzy::Consequent rule3_then = { fanSpeedVar, fanMediumMf };
    result = fuzzy.addRule(rule3_ifs, 2, FUZZY_AND, rule3_then);
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 3: " + String(fuzzy.getResultString(result)));

    // Rule 4: IF Temperature IS Hot AND Humidity IS Humid THEN FanSpeed IS Fast
    AutoFuzzy::Antecedent rule4_ifs[] = { { tempVar, tempHotMf }, { humidityVar, humHumidMf } };
    AutoFuzzy::Consequent rule4_then = { fanSpeedVar, fanFastMf };
    result = fuzzy.addRule(rule4_ifs, 2, FUZZY_AND, rule4_then);
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 4: " + String(fuzzy.getResultString(result)));

    // Rule 5: IF Temperature IS Comfortable AND Humidity IS Humid THEN FanSpeed IS Medium
    AutoFuzzy::Antecedent rule5_ifs[] = { { tempVar, tempComfortMf }, { humidityVar, humHumidMf } };
    AutoFuzzy::Consequent rule5_then = { fanSpeedVar, fanMediumMf };
    result = fuzzy.addRule(rule5_ifs, 2, FUZZY_AND, rule5_then);
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 5: " + String(fuzzy.getResultString(result)));
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
