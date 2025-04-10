// AutoFuzzy Basic Example: Simple LED Brightness Control
// Controls LED brightness based on ambient light level.

#include <AutoFuzzy.h> // Include the library

// --- Pin Definitions ---
const int photoresistorPin = A0; // Analog pin for the photoresistor
const int ledPin = 9; // PWM pin for the LED

// --- Fuzzy Logic Object ---
AutoFuzzy fuzzy; // Create an instance of the AutoFuzzy class

// --- Variable Indices ---
// We store the indices returned by addInput/addOutput for clarity,
// though for simple cases like this, you could remember 0 is input, 1 is output.
int lightLevelVar = -1;
int ledBrightnessVar = -1;

void setup()
{
    // --- Initialize Serial Monitor ---
    Serial.begin(9600);
    while (!Serial)
        ; // Wait for serial connection (optional)
    Serial.println("AutoFuzzy Basic Example: LED Brightness Control");

    // --- Initialize Pin Modes ---
    pinMode(ledPin, OUTPUT);
    // Analog input pins are input by default, no need for pinMode(photoresistorPin, INPUT);

    // --- Configure Fuzzy Logic System ---
    Serial.println("Configuring Fuzzy System...");
    FuzzyResult result; // Variable to store results of fuzzy operations

    // 1. Add Input Variable
    // Name: "Light", Range: 0 (dark) to 100 (bright) - we'll map the analogRead later.
    lightLevelVar = fuzzy.addInput("Light", 0.0f, 100.0f);
    if (lightLevelVar < 0) {
        Serial.println("Error adding input variable 'Light'");
        while (1)
            ; // Halt execution
    }
    Serial.println(" - Input 'Light' added (Index " + String(lightLevelVar) + ")");

    // 2. Add Output Variable
    // Name: "Brightness", Range: 0 (off) to 255 (full brightness for PWM)
    ledBrightnessVar = fuzzy.addOutput("Brightness", 0.0f, 255.0f);
    if (ledBrightnessVar < 0) {
        Serial.println("Error adding output variable 'Brightness'");
        while (1)
            ; // Halt execution
    }
    Serial.println(" - Output 'Brightness' added (Index " + String(ledBrightnessVar) + ")");

    // 3. Add Membership Functions (MFs) for Input 'Light'
    // Using Triangular MFs: addTriangularMF(variable_name, mf_name, left_point, peak_point, right_point)
    result = fuzzy.addTriangularMF("Light", "Dark", 0.0f, 15.0f, 30.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Dark': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF("Light", "Dim", 20.0f, 50.0f, 80.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Dim': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF("Light", "Bright", 70.0f, 85.0f, 100.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Bright': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Light' added.");

    // 4. Add Membership Functions (MFs) for Output 'Brightness'
    // Using Triangular MFs
    result = fuzzy.addTriangularMF("Brightness", "Off", 0.0f, 0.0f, 50.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Off': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF("Brightness", "Medium", 40.0f, 127.0f, 210.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Medium': " + String(fuzzy.getResultString(result)));
    result = fuzzy.addTriangularMF("Brightness", "Full", 200.0f, 255.0f, 255.0f);
    if (result != FUZZY_OK)
        Serial.println("Error adding MF 'Full': " + String(fuzzy.getResultString(result)));
    Serial.println(" - MFs for 'Brightness' added.");

    // 5. Add Fuzzy Rules
    // Using simple name-based helper: addRule(if_var, if_mf, then_var, then_mf)
    result = fuzzy.addRule("Light", "Dark", "Brightness", "Full");
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 1: " + String(fuzzy.getResultString(result)));
    result = fuzzy.addRule("Light", "Dim", "Brightness", "Medium");
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 2: " + String(fuzzy.getResultString(result)));
    result = fuzzy.addRule("Light", "Bright", "Brightness", "Off");
    if (result != FUZZY_OK)
        Serial.println("Error adding Rule 3: " + String(fuzzy.getResultString(result)));
    Serial.println(" - Fuzzy Rules added.");

    Serial.println("Fuzzy System Configuration Complete.\n");
}

void loop()
{
    // --- Read Sensor Input ---
    // Read the analog value (0-1023)
    int sensorValue = analogRead(photoresistorPin);
    // Map the sensor value to the fuzzy input range (0-100)
    // Note: You might need to reverse the mapping depending on your photoresistor wiring
    // (e.g., map(sensorValue, 0, 1023, 100, 0) if higher reading means darker)
    float currentLightLevel = map(sensorValue, 0, 1023, 0, 100);

    // --- Prepare Fuzzy Input ---
    // Create an array of FuzzyInput structs. For SISO, it has only one element.
    FuzzyInput fuzzyInputs[] = {
        { lightLevelVar, currentLightLevel } // Pair the variable index with its current value
    };
    int numInputs = 1; // Number of inputs in the array

    // --- Evaluate Fuzzy Logic ---
    float calculatedBrightness; // Variable to store the fuzzy output
    // evaluate(input_array, num_inputs, result_variable, output_variable_index)
    FuzzyResult result = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedBrightness, ledBrightnessVar);

    // --- Check Evaluation Result ---
    if (result == FUZZY_OK) {
        // Evaluation successful
        Serial.print("Light Level: " + String(currentLightLevel, 1));
        Serial.println(" %\t Calculated Brightness: " + String(calculatedBrightness, 0));

        // --- Apply Output ---
        // Use the calculated fuzzy output to set the LED brightness
        analogWrite(ledPin, (int)calculatedBrightness);

    } else if (result == FUZZY_ERROR_NO_RULES_FIRED) {
        // This might happen if the input value falls outside all defined input MFs
        // or if the rules don't cover the current situation.
        Serial.println("Warning: No fuzzy rules fired for input " + String(currentLightLevel, 1) + ". Setting LED off.");
        analogWrite(ledPin, 0); // Default action
    } else {
        // An unexpected error occurred during evaluation
        Serial.println("Error during fuzzy evaluation: " + String(fuzzy.getResultString(result)));
        analogWrite(ledPin, 0); // Default safe action
    }

    // --- Delay ---
    delay(500); // Wait half a second before the next reading
}
