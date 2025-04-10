# AutoFuzzy Library: Usage Guide

This guide details how to use the AutoFuzzy library in your Arduino sketches to implement fuzzy logic controllers.

## Prerequisites

1.  **Install the Library:** Ensure the AutoFuzzy library is correctly installed in your Arduino IDE (See main README.md).
2.  **Include Header:** Add the following line at the top of your sketch:
    ```cpp
    #include <AutoFuzzy.h>
    ```

## Step-by-Step Guide

Here's the typical workflow for setting up and using the AutoFuzzy system:

### 1. Instantiate the AutoFuzzy Object

First, create an instance of the `AutoFuzzy` class. This object will hold your entire fuzzy system configuration.

```cpp
AutoFuzzy fuzzy;
```

### 2. Define Input and Output Variables

Define the linguistic variables your system will use. Inputs typically come from sensors, and outputs control actuators. Use descriptive names.

*   **Add Input Variables:** Use `addInput(name, min_value, max_value)`
*   **Add Output Variables:** Use `addOutput(name, min_value, max_value)`

**Important:** Check the return value! These functions return the variable's index (>=0) on success or -1 on error. Store these indices if you plan to add MFs or rules using them (which is more efficient than using names).

```cpp
int tempVar = -1;       // Variable to store index
int fanSpeedVar = -1; // Variable to store index

void setup() {
  // ... Serial.begin(), etc.

  tempVar = fuzzy.addInput("Temperature", 0.0f, 50.0f); // Range 0-50 C
  if (tempVar < 0) {
    Serial.println("Error: Failed to add input 'Temperature'!");
    while(1); // Halt on critical error
  }

  fanSpeedVar = fuzzy.addOutput("FanSpeed", 0.0f, 100.0f); // Range 0-100 %
  if (fanSpeedVar < 0) {
    Serial.println("Error: Failed to add output 'FanSpeed'!");
    while(1); // Halt
  }
  Serial.println("Variables added successfully.");
}
```

### 3. Define Membership Functions (MFs)

For each variable, define the fuzzy sets (linguistic terms like "Cold", "Optimal", "High") using Membership Functions.

*   **Triangular:** `addTriangularMF(varIndexOrName, mfName, a, b, c)`
*   **Trapezoidal:** `addTrapezoidalMF(varIndexOrName, mfName, a, b, c, d)`

**Important:** Check the `FuzzyResult` return value for errors!

```cpp
void setup() {
  // ... after adding variables ...
  FuzzyResult result; // To store function results

  // Add MFs for Temperature (using index stored in tempVar)
  result = fuzzy.addTrapezoidalMF(tempVar, "Cold", 0.0f, 5.0f, 15.0f, 20.0f);
  if (result != FUZZY_OK) Serial.println("Error adding MF 'Cold': " + String(fuzzy.getResultString(result)));

  result = fuzzy.addTrapezoidalMF(tempVar, "Optimal", 18.0f, 22.0f, 26.0f, 30.0f);
  if (result != FUZZY_OK) Serial.println("Error adding MF 'Optimal': " + String(fuzzy.getResultString(result)));

  result = fuzzy.addTrapezoidalMF(tempVar, "Hot", 28.0f, 35.0f, 45.0f, 50.0f);
  if (result != FUZZY_OK) Serial.println("Error adding MF 'Hot': " + String(fuzzy.getResultString(result)));

  // Add MFs for FanSpeed (using name directly - less efficient but convenient)
  result = fuzzy.addTrapezoidalMF("FanSpeed", "Slow", 0.0f, 10.0f, 25.0f, 40.0f);
  if (result != FUZZY_OK) Serial.println("Error adding MF 'Slow': " + String(fuzzy.getResultString(result)));

  result = fuzzy.addTrapezoidalMF("FanSpeed", "Fast", 60.0f, 80.0f, 95.0f, 100.0f);
  if (result != FUZZY_OK) Serial.println("Error adding MF 'Fast': " + String(fuzzy.getResultString(result)));

  Serial.println("Membership functions defined.");
}
```

### 4. Define Fuzzy Rules

Create the IF-THEN rules that link your input conditions to output actions.

**Methods:**

*   **Simple SISO Rule (using names):**
    ```cpp
    result = fuzzy.addRule("Temperature", "Cold", "FanSpeed", "Slow");
    if (result != FUZZY_OK) { /* Handle error */ }
    ```

*   **Simple SISO Rule (using indices - recommended):**
    ```cpp
    // Get MF indices first (safer and more efficient)
    int tempColdMf = fuzzy.findMF(tempVar, "Cold");
    int fanSlowMf = fuzzy.findMF(fanSpeedVar, "Slow");
    if (tempColdMf < 0 || fanSlowMf < 0) { /* Handle error: MF not found */ }

    // Define the antecedent (IF part)
    AutoFuzzy::Antecedent if_condition = {tempVar, tempColdMf};
    // Define the consequent (THEN part)
    AutoFuzzy::Consequent then_action = {fanSpeedVar, fanSlowMf};

    // Add the rule
    result = fuzzy.addRule(if_condition, then_action);
    if (result != FUZZY_OK) { /* Handle error */ }
    ```

*   **Multi-Antecedent Rule (using indices):**
    ```cpp
    // Get MF indices needed
    int tempHotMf = fuzzy.findMF(tempVar, "Hot");
    int humidityHumidMf = fuzzy.findMF(humidityVar, "Humid"); // Assume humidityVar exists
    int fanFastMf = fuzzy.findMF(fanSpeedVar, "Fast");
    if (tempHotMf < 0 || humidityHumidMf < 0 || fanFastMf < 0) { /* Handle error */ }

    // Define the antecedents (IF parts)
    AutoFuzzy::Antecedent conditions[] = {
      {tempVar, tempHotMf},         // IF Temperature IS Hot
      {humidityVar, humidityHumidMf} // AND Humidity IS Humid
    };
    int numConditions = 2;

    // Define the consequent (THEN part)
    AutoFuzzy::Consequent action = {fanSpeedVar, fanFastMf}; // THEN FanSpeed IS Fast

    // Add the rule using FUZZY_AND or FUZZY_OR
    result = fuzzy.addRule(conditions, numConditions, FUZZY_AND, action);
    if (result != FUZZY_OK) { /* Handle error */ }
    ```

### 5. Prepare Inputs for Evaluation

In your `loop()`, read your sensor values and package them into an array of `FuzzyInput` structs. Each struct needs the variable index and the current crisp value.

```cpp
void loop() {
  // Read sensor values
  float currentTemp = readTemperatureSensor();
  float currentHumidity = readHumiditySensor(); // Assuming you have this input

  // Create the input array - MUST include values for ALL inputs used by rules
  FuzzyInput currentInputs[] = {
    { (uint8_t)tempVar, currentTemp },         // Use the stored index
    { (uint8_t)humidityVar, currentHumidity }  // Use the stored index
  };
  int numInputsProvided = 2; // Number of structs in the array
  // ...
}
```

### 6. Evaluate the Fuzzy System

Call the `evaluate()` function to calculate the crisp output value for a specific output variable.

*   For **MIMO** systems (Multiple Outputs), you need to call `evaluate()` **once for each output variable** you want to calculate.

```cpp
void loop() {
  // ... read sensors and prepare currentInputs array ...

  float calculatedFanSpeed; // Variable to store the result
  float calculatedHeaterPower; // Example for a second output

  // Evaluate for the FanSpeed output (using fanSpeedVar index)
  FuzzyResult fanResult = fuzzy.evaluate(currentInputs, numInputsProvided, calculatedFanSpeed, fanSpeedVar);

  // Check the result
  if (fanResult == FUZZY_OK) {
    Serial.println("Calculated Fan Speed: " + String(calculatedFanSpeed));
    // Use calculatedFanSpeed to control your fan/actuator
  } else if (fanResult == FUZZY_ERROR_NO_RULES_FIRED) {
    Serial.println("Warning: No rules fired for fan speed. Using default.");
    // Apply a default safe value
  } else {
    Serial.println("Error evaluating fan speed: " + String(fuzzy.getResultString(fanResult)));
    // Apply a default safe value
  }

  // --- If you have another output (e.g., Heater) ---
  /*
  FuzzyResult heaterResult = fuzzy.evaluate(currentInputs, numInputsProvided, calculatedHeaterPower, heaterVar); // Assume heaterVar exists
  if (heaterResult == FUZZY_OK) {
     Serial.println("Calculated Heater Power: " + String(calculatedHeaterPower));
     // Use calculatedHeaterPower
  } else {
     // Handle heater evaluation errors/warnings
  }
  */

  delay(1000); // Wait before next cycle
}
```

### 7. (Optional) Auto Tuning

If you have training data (known input sets and their corresponding desired output values), you can use `autoTune()` to attempt optimization of MF parameters. **This is computationally intensive and uses dynamic memory – run in `setup()` if possible.**

```cpp
void setup() {
  // ... complete steps 1-4 (define vars, MFs, rules) ...

  // --- Prepare Training Data ---
  const int NUM_SETS = 3;
  // Input data: columns must match order inputs were added (e.g., Temp, Humidity)
  float train_inputs_data[NUM_SETS][2] = {
    {15.0, 60.0}, // Cold, Normal
    {28.0, 50.0}, // Warm, Normal
    {38.0, 80.0}  // Hot, Humid
  };
  // Convert to array of pointers for the function
  float* train_inputs[NUM_SETS];
  for(int i=0; i<NUM_SETS; ++i) train_inputs[i] = train_inputs_data[i];

  // Expected output data (e.g., for FanSpeed)
  float train_outputs[NUM_SETS] = { 20.0, 60.0, 90.0 }; // Expected % fan speeds

  // --- Run Tuning ---
  Serial.println("Running AutoTune...");
  // Tune parameters focusing on fanSpeedVar output
  FuzzyResult tuneResult = fuzzy.autoTune(
                                train_inputs,
                                train_outputs,
                                NUM_SETS,
                                fanSpeedVar, // Target output variable index
                                500,        // Iterations (adjust as needed)
                                0.1f,       // Mutation Rate (0.0-1.0)
                                0.1f        // Mutation Range (0.0-1.0)
                               );

  if (tuneResult == FUZZY_OK) {
    Serial.println("AutoTune completed.");
    // The MFs in the 'fuzzy' object are now potentially modified.
  } else {
    Serial.println("AutoTune failed: " + String(fuzzy.getResultString(tuneResult)));
  }

  Serial.println("Setup complete.");
}
```

**Important `autoTune` Notes:**
*   It's heuristic, not guaranteed to find the best solution.
*   Requires good, representative training data.
*   Tunes parameters for *all* MFs, aiming to improve performance for the *specified* output variable.
*   Can take significant time and RAM.

## Configuration (Overrides)

You can change default limits (max variables, rules, etc.) by defining macros *before* `#include <AutoFuzzy.h>`:

```cpp
#define FUZZY_MAX_RULES 100 // Allow up to 100 rules
#define FUZZY_MAX_VARS 15   // Allow up to 15 variables
#include <AutoFuzzy.h>
```
(See main README.md for default values and other defines). Beware of increased RAM usage.

## Tips and Best Practices

*   **Check ALL Return Values:** Especially during setup (`addInput`, `addOutput`, `add...MF`, `addRule`). Use `getResultString()` for debugging.
*   **Use Indices:** Adding MFs/Rules using variable/MF indices is generally safer and more efficient than using names. Use `findVariable()` and `findMF()` to get indices.
*   **Start Simple:** Build and test your fuzzy system incrementally.
*   **Ensure MF Overlap:** Input variables should ideally have some degree of membership (>0) in at least one MF across their expected range to avoid `FUZZY_ERROR_NO_RULES_FIRED`. MFs should overlap smoothly.
*   **Rule Coverage:** Ensure your rules logically cover the combinations of inputs you expect to encounter.
*   **Monitor Memory:** Especially if increasing limits or using `autoTune`, keep an eye on RAM usage.
