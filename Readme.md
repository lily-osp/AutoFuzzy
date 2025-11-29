# AutoFuzzy

**Version:** 1.2.0

AutoFuzzy is a robust and flexible Arduino library providing fuzzy logic capabilities with multiple-input/multiple-output support, enhanced rule definition, better error handling, and a heuristic parameter tuning mechanism. It's designed to simplify implementing fuzzy controllers for Arduino projects while offering more advanced control and customization compared to basic implementations.

## Table of Contents

- [Key Features](#key-features)
- [Installation](#installation)
- [Core Fuzzy Logic Concepts](#core-fuzzy-logic-concepts)
- [API Reference](#api-reference)
  - [Class Initialization](#class-initialization)
  - [Variable Management](#variable-management)
  - [Membership Function Management](#membership-function-management)
  - [Rule Management](#rule-management)
  - [Fuzzy Inference](#fuzzy-inference)
  - [Automated Tuning](#automated-tuning)
  - [Utility](#utility)
- [Membership Function Details](#membership-function-details)
  - [Triangular Membership Function](#triangular-membership-function)
  - [Trapezoidal Membership Function](#trapezoidal-membership-function)
- [Fuzzy Logic Operations Math](#fuzzy-logic-operations-math)
- [Configuration](#configuration)
- [Basic Usage Example](#basic-usage-example)
- [Advanced Usage](#advanced-usage)
- [Error Handling](#error-handling)
- [Memory Usage & Performance](#memory-usage--performance)
- [Limitations](#limitations)
- [Troubleshooting & Best Practices](#troubleshooting--best-practices)
- [Contributing](#contributing)
- [License](#license)

## Key Features

- **Flexible API:** Easy-to-use functions for defining inputs, outputs, membership functions, and rules
- **Multiple Inputs/Outputs (MIMO):** Supports systems with multiple sensor inputs and multiple control outputs
- **Multi-Antecedent Rules:** Define complex rules like `IF temperature IS hot AND humidity IS high THEN ...` using `AND` or `OR` logic
- **Standard Membership Functions:** Includes Triangular and Trapezoidal shapes
- **Robust Error Handling:** Functions return `FuzzyResult` codes, with a helper to get descriptive error messages
- **Heuristic Parameter Tuning:** `autoTune()` function attempts to optimize Membership Function parameters using random mutation and a fitness function based on training data
- **Configurable Limits:** Maximum number of variables, MFs, rules, etc., can be adjusted via preprocessor defines
- **Clear Structure:** Uses enums (`MfType`, `FuzzyOperator`, `FuzzyResult`) and structs (`FuzzyInput`, `Antecedent`, `Consequent`) for better code readability and safety
- **Memory Conscious:** Designed with Arduino limitations in mind (though `autoTune` uses dynamic memory)

## Installation

1. Download the latest release ZIP file from the repository
2. In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...`
3. Select the downloaded ZIP file (`AutoFuzzy-main.zip` or similar)
4. The library will appear in `Sketch > Include Library > AutoFuzzy`
5. (Optional) Restart the Arduino IDE

Alternatively, you can clone or download this repository and place the `AutoFuzzy` folder in your Arduino libraries directory.

## Core Fuzzy Logic Concepts

AutoFuzzy implements a Mamdani-style fuzzy inference system with some simplifications common for microcontrollers. Here's a brief overview:

1.  **Linguistic Variables:** These are the inputs (e.g., "Temperature", "Humidity") and outputs (e.g., "Fan Speed", "Heater Power") of your system. Each variable has a defined numerical range (min/max).
2.  **Membership Functions (MFs):** These functions define fuzzy sets (linguistic terms like "Cold", "Optimal", "High") for each variable. They map a crisp numerical input value to a "degree of membership" (between 0 and 1) for that fuzzy set. AutoFuzzy supports:
    *   **Triangular:** Defined by 3 points (start, peak, end).
    *   **Trapezoidal:** Defined by 4 points (start, plateau-start, plateau-end, end).
3.  **Fuzzy Rules:** These are IF-THEN statements that connect input fuzzy sets to output fuzzy sets. Examples:
    *   `IF Temperature IS Cold THEN HeaterPower IS High`
    *   `IF Temperature IS Hot AND Humidity IS Humid THEN VentSpeed IS Fast`
    AutoFuzzy allows multiple `AND`/`OR` conditions (antecedents) per rule.
4.  **Fuzzy Inference Process:**
    *   **Fuzzification:** For each input variable, its crisp numerical value is compared against its MFs to determine the degree of membership for each fuzzy set (e.g., Temperature=15°C might be 0.8 "Cold" and 0.2 "Optimal").
    *   **Rule Evaluation (Activation):** For each rule, the degrees of membership from the fuzzified inputs are combined using fuzzy operators (`AND` usually MIN, `OR` usually MAX) to determine the rule's activation strength (a value between 0 and 1).
    *   **Aggregation:** (Implicit in AutoFuzzy's defuzzification) The output fuzzy sets suggested by all activated rules are combined.
    *   **Defuzzification:** The combined fuzzy output is converted back into a single crisp numerical value that can be used to control an actuator. AutoFuzzy uses a simplified **Center of Sums** (or weighted average) method.

## API Reference

### Class Initialization

```cpp
#include <AutoFuzzy.h>

AutoFuzzy fuzzy; // Create an instance
```

### Variable Management

**`int addInput(const char* name, float min = 0.0f, float max = 1.0f)`**
*   **Purpose:** Adds an input (antecedent) linguistic variable to the system.
*   **Parameters:**
    *   `name`: A unique C-string name for the variable (max length `FUZZY_MAX_NAME_LEN - 1`).
    *   `min`: The minimum expected numerical value for this input.
    *   `max`: The maximum expected numerical value for this input.
*   **Returns:** The integer index of the added variable (>= 0) on success, or -1 on error (e.g., too many variables, duplicate name, null name).

**`int addOutput(const char* name, float min = 0.0f, float max = 1.0f)`**
*   **Purpose:** Adds an output (consequent) linguistic variable to the system.
*   **Parameters:** Same as `addInput`.
*   **Returns:** The integer index of the added variable (>= 0) on success, or -1 on error.

**`int findVariable(const char* name) const`**
* **Purpose:** Finds the index of a variable (input or output) by its name
* **Parameters:**
  * `name`: The name of the variable to find
* **Returns:** The integer index of the variable if found, or -1 if not found or name is null

### Membership Function (MF) Management

**`FuzzyResult addTriangularMF(int varIndex, const char* mfName, float a, float b, float c)`**
**`FuzzyResult addTriangularMF(const char* varName, const char* mfName, float a, float b, float c)`**
* **Purpose:** Adds a triangular membership function to a specified variable
* **Parameters:**
  * `varIndex` / `varName`: Index or name of the variable to add the MF to
  * `mfName`: A unique C-string name for this MF within this variable (max length `FUZZY_MAX_NAME_LEN - 1`)
  * `a`: The left foot of the triangle (membership starts increasing from 0)
  * `b`: The peak of the triangle (membership is 1.0)
  * `c`: The right foot of the triangle (membership returns to 0). Must satisfy `a <= b <= c`
* **Returns:** `FuzzyResult` enum value:
  * `FUZZY_OK`: Success
  * `FUZZY_ERROR_VAR_NOT_FOUND`: Variable not found
  * `FUZZY_ERROR_TOO_MANY_MFS`: Max MFs for this variable reached
  * `FUZZY_ERROR_INVALID_PARAMS`: Invalid parameters (e.g., `a > b`, duplicate `mfName` for this variable)
  * `FUZZY_ERROR_NULL_POINTER`: `mfName` was null

**`FuzzyResult addTrapezoidalMF(int varIndex, const char* mfName, float a, float b, float c, float d)`**
**`FuzzyResult addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d)`**
*   **Purpose:** Adds a trapezoidal membership function to a specified variable.
*   **Parameters:**
    *   `varIndex` / `varName`: Index or name of the variable.
    *   `mfName`: A unique C-string name for this MF within this variable.
    *   `a`: The left foot of the trapezoid (membership starts increasing from 0).
    *   `b`: The left shoulder (membership reaches 1.0).
    *   `c`: The right shoulder (membership starts decreasing from 1.0).
    *   `d`: The right foot of the trapezoid (membership returns to 0). Must satisfy `a <= b <= c <= d`.
*   **Returns:** `FuzzyResult` enum value (similar to `addTriangularMF`).

**`int findMF(int varIndex, const char* mfName) const`**
*   **Purpose:** Finds the index of a membership function within a specific variable by its name.
*   **Parameters:**
    *   `varIndex`: The index of the variable containing the MF.
    *   `mfName`: The name of the membership function to find.
*   **Returns:** The integer index of the MF (>= 0) within the variable's MF list if found, or -1 if not found, variable index is invalid, or `mfName` is null.

### Rule Management

**Helper Structs:**

```cpp
// Represents one IF condition (antecedent)
struct AutoFuzzy::Antecedent {
    int varIndex; // Index of the *input* variable
    int mfIndex;  // Index of the MF for this variable
};

// Represents the THEN action (consequent)
struct AutoFuzzy::Consequent {
    int varIndex; // Index of the *output* variable
    int mfIndex;  // Index of the MF for this variable
};
```

**`FuzzyResult addRule(const Antecedent antecedents[], int numAntecedents, FuzzyOperator op, const Consequent& consequent)`**
*   **Purpose:** Adds a fuzzy rule with one or more antecedents combined by a logical operator.
*   **Parameters:**
    *   `antecedents[]`: An array of `Antecedent` structs defining the IF conditions.
    *   `numAntecedents`: The number of elements in the `antecedents` array (must be > 0 and <= `FUZZY_MAX_ANTECEDENTS_PER_RULE`).
    *   `op`: The fuzzy operator (`FUZZY_AND` or `FUZZY_OR`) used to combine the `antecedents`.
    *   `consequent`: A `Consequent` struct defining the THEN action.
*   **Returns:** `FuzzyResult` enum value:
    *   `FUZZY_OK`: Success.
    *   `FUZZY_ERROR_TOO_MANY_RULES`: Max rules reached.
    *   `FUZZY_ERROR_TOO_MANY_ANTECEDENTS`: `numAntecedents` is invalid.
    *   `FUZZY_ERROR_NULL_POINTER`: `antecedents` array was null.
    *   `FUZZY_ERROR_INVALID_RULE`: An antecedent referred to an output variable, or the consequent referred to an input variable.
    *   `FUZZY_ERROR_VAR_NOT_FOUND`: An index in `antecedents` or `consequent` was invalid.
    *   `FUZZY_ERROR_MF_NOT_FOUND`: An MF index was invalid for its variable.

**`FuzzyResult addRule(const Antecedent& antecedent, const Consequent& consequent)`**
*   **Purpose:** Convenience overload for adding a simple rule with only one antecedent (SISO-like part). Equivalent to calling the multi-antecedent version with `numAntecedents = 1` and `op = FUZZY_AND`.
*   **Parameters:**
    *   `antecedent`: The single `Antecedent` struct.
    *   `consequent`: The `Consequent` struct.
*   **Returns:** `FuzzyResult` (as above).

**`FuzzyResult addRule(const char* ifVarName, const char* ifMfName, const char* thenVarName, const char* thenMfName)`**
*   **Purpose:** Convenience overload to add a simple, single-antecedent rule using variable and MF names directly. Less efficient than using indices.
*   **Parameters:**
    *   `ifVarName`: Name of the input variable for the IF condition.
    *   `ifMfName`: Name of the MF for the IF condition.
    *   `thenVarName`: Name of the output variable for the THEN action.
    *   `thenMfName`: Name of the MF for the THEN action.
*   **Returns:** `FuzzyResult` (as above).

### Fuzzy Inference

**Helper Struct:**

```cpp
// Represents a single crisp input value for evaluation
typedef struct {
    uint8_t varIndex; // Index of the input variable this value belongs to
    float value;      // The current numerical value
} FuzzyInput;
```

**`FuzzyResult evaluate(const FuzzyInput inputs[], int numInputs, float& resultValue, int outputVarIndex = 0)`**
*   **Purpose:** Performs the complete fuzzy inference process (fuzzification, rule evaluation, defuzzification) for a specific output variable based on the current crisp inputs.
*   **Parameters:**
    *   `inputs[]`: An array of `FuzzyInput` structs containing the current numerical value for *each* input variable needed by the rules affecting the target output.
    *   `numInputs`: The number of elements in the `inputs` array.
    *   `resultValue`: A reference to a float where the calculated crisp output value will be stored.
    *   `outputVarIndex`: The index of the *output* variable you want to calculate the value for. Defaults to 0 (the first added output variable).
*   **Returns:** `FuzzyResult` enum value:
    *   `FUZZY_OK`: Success, `resultValue` contains the calculated output.
    *   `FUZZY_ERROR_NULL_POINTER`: `inputs` array was null.
    *   `FUZZY_ERROR_VAR_NOT_FOUND`: `outputVarIndex` was invalid or not an output variable.
    *   `FUZZY_ERROR_NO_RULES_FIRED`: None of the rules affecting the target output had a non-zero activation strength. `resultValue` might be set to the output variable's minimum.
    *   `FUZZY_ERROR_DIVIDE_BY_ZERO`: The sum of rule activation weights was zero during defuzzification. `resultValue` might be set to the output variable's minimum.
    *   Other errors might propagate from internal calls (e.g., `FUZZY_ERROR_MF_NOT_FOUND` if a rule points to a non-existent MF).

### Automated Tuning (Heuristic)

**`FuzzyResult autoTune(float** trainingInputs**, float* trainingOutputs, int numSets, int outputVarIndex, int iterations = 100, float mutationRate = 0.1f, float mutationRange = 0.1f)`**
*   **Purpose:** Attempts to heuristically optimize the parameters of *all* membership functions (for both inputs and outputs) to better match provided training data for a *specific output variable*. Uses random mutation and fitness evaluation (Mean Squared Error). **This is not a guaranteed optimizer.**
*   **Parameters:**
    *   `trainingInputs`: A 2D array (array of float pointers) representing the input data sets. `trainingInputs[i][j]` should be the value for the j-th *input variable* in the i-th training set. The order of columns (j) must match the order in which input variables were added.
    *   `trainingOutputs`: A 1D array of floats representing the *expected* output values for the target `outputVarIndex` corresponding to each input set in `trainingInputs`. `trainingOutputs[i]` is the desired output for `trainingInputs[i]`.
    *   `numSets`: The number of training data sets (rows in `trainingInputs`, elements in `trainingOutputs`).
    *   `outputVarIndex`: The index of the output variable the tuning process should focus on when calculating fitness.
    *   `iterations`: The number of tuning cycles to perform. More iterations take longer but might yield better results.
    *   `mutationRate`: The probability (0.0 to 1.0) that any single MF parameter will be mutated in an iteration.
    *   `mutationRange`: The maximum fractional change (0.0 to 1.0) applied during mutation, relative to the variable's total range (`max - min`).
*   **Returns:** `FuzzyResult` enum value:
    *   `FUZZY_OK`: Tuning process completed (doesn't guarantee improvement).
    *   `FUZZY_ERROR_INVALID_PARAMS`: `numSets` was <= 0, or training data pointers were null.
    *   `FUZZY_ERROR_VAR_NOT_FOUND`: `outputVarIndex` was invalid or not an output variable.
    *   `FUZZY_ERROR_NULL_POINTER`: Memory allocation failed internally.
    *   Evaluation errors (`FUZZY_ERROR_NO_RULES_FIRED`, etc.) might occur during fitness calculation if the mutated parameters create invalid states.

### Utility

**`const char* getResultString(FuzzyResult result) const`**
*   **Purpose:** Converts a `FuzzyResult` enum value into a human-readable C-string description.
*   **Parameters:**
    *   `result`: The `FuzzyResult` code to convert.
*   **Returns:** A pointer to a constant string describing the result code.

---

## Membership Function Details

### Triangular Membership Function

*   **Definition:** Defined by three points `a`, `b`, `c` where `a <= b <= c`.
    *   `a`: Left foot (membership = 0)
    *   `b`: Peak (membership = 1)
    *   `c`: Right foot (membership = 0)
*   **Formula:**
    ```
    μ(x) =
      { 0                      if x <= a or x >= c
      { (x - a) / (b - a)      if a < x < b
      { (c - x) / (c - b)      if b <= x < c
    ```
    (Handles `b-a=0` or `c-b=0` by returning 1 if `x=b`)
*   **Shape:** A simple triangle.
*   **API:** `addTriangularMF(...)`

### Trapezoidal Membership Function

*   **Definition:** Defined by four points `a`, `b`, `c`, `d` where `a <= b <= c <= d`.
    *   `a`: Left foot (membership = 0)
    *   `b`: Left shoulder (membership reaches 1)
    *   `c`: Right shoulder (membership leaves 1)
    *   `d`: Right foot (membership = 0)
*   **Formula:**
    ```
    μ(x) =
      { 0                      if x <= a or x >= d
      { (x - a) / (b - a)      if a < x < b
      { 1                      if b <= x <= c
      { (d - x) / (d - c)      if c < x < d
    ```
    (Handles `b-a=0` or `d-c=0` by returning 1 if `x` is on the plateau)
*   **Shape:** A trapezoid with a flat top (plateau) where membership is 1.
*   **API:** `addTrapezoidalMF(...)`

---

## Fuzzy Logic Operations Math

### Fuzzy Operators (Rule Antecedent Combination)

When a rule has multiple antecedents (e.g., `IF condition1 AND condition2`), their individual membership degrees (`μ1`, `μ2`) need to be combined to get the rule's activation strength.

*   **`FUZZY_AND`:** Uses the **MIN** operator (T-Norm).
    *   `Activation = min(μ1, μ2, ...)`
*   **`FUZZY_OR`:** Uses the **MAX** operator (T-CoNorm / S-Norm).
    *   `Activation = max(μ1, μ2, ...)`

### Defuzzification (Output Calculation)

AutoFuzzy uses a simplified **Center of Sums** (also related to Weighted Average or simplified Center of Gravity) method.

1.  **Calculate Rule Activations:** For every rule `i` that affects the target output variable, calculate its activation strength `w_i` (as described above).
2.  **Find Consequent MF Centroids:** For each activated rule `i`, find the representative crisp value (centroid) `c_i` of its consequent membership function (e.g., the `THEN FanSpeed IS Fast` part).
    *   For **Triangular MF (a, b, c):** Centroid `c_i` is approximated as the peak `b`.
    *   For **Trapezoidal MF (a, b, c, d):** Centroid `c_i` is approximated as the center of the plateau `(b + c) / 2`.
3.  **Calculate Weighted Sum:** The final crisp output value is calculated as:

    ```
    Output = Σ (w_i * c_i) / Σ (w_i)
    ```

    Where the sum `Σ` is over all rules `i` affecting the target output variable that fired (i.e., `w_i > 0`).

    *Note: This avoids the complex integration needed for the true Center of Gravity of the aggregated output shape, making it suitable for microcontrollers.*

---

## Configuration

You can change the maximum limits of the library *before* including `AutoFuzzy.h` in your sketch by defining override macros. This is especially useful for larger Arduino boards (Mega, ESP32, etc.) that have more RAM.

```cpp
// Define limits BEFORE including the header
#define FUZZY_MAX_VARS 6           // Max total variables (inputs + outputs)
#define FUZZY_MAX_MF_PER_VAR 5     // Max MFs per variable
#define FUZZY_MAX_RULES 15         // Max total rules
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 3  // Max conditions per rule
#define FUZZY_MAX_NAME_LEN 15      // Max name length

#include <AutoFuzzy.h> // Now include the library

// ... rest of your sketch
```

### Default Limits (Arduino Mega Compatible)

The library uses optimized defaults for Arduino Mega (8KB RAM) to support complex fuzzy systems:

*   `FUZZY_MAX_VARS`: 6 (total input + output variables)
*   `FUZZY_MAX_MF_PER_VAR`: 5 (membership functions per variable)
*   `FUZZY_MAX_RULES`: 15 (total rules)
*   `FUZZY_MAX_ANTECEDENTS_PER_RULE`: 3 (conditions per rule)
*   `FUZZY_MAX_NAME_LEN`: 12 (character limit for names)

### Recommended Limits for Different Boards

**Arduino Uno/Nano (2KB RAM - Basic Applications):**
```cpp
#define FUZZY_MAX_VARS 3
#define FUZZY_MAX_MF_PER_VAR 3
#define FUZZY_MAX_RULES 8
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 2
```

**Arduino Mega (8KB RAM - Standard Applications):**
```cpp
#define FUZZY_MAX_VARS 8
#define FUZZY_MAX_MF_PER_VAR 6
#define FUZZY_MAX_RULES 20
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 4
```

**ESP32 (520KB RAM - Advanced Applications):**
```cpp
#define FUZZY_MAX_VARS 12
#define FUZZY_MAX_MF_PER_VAR 8
#define FUZZY_MAX_RULES 50
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 5
```

**Note:** Examples are configured for Arduino Mega. Adjust limits based on your specific application requirements and available memory.

---

## Basic Usage Example (SISO)

```cpp
#include <AutoFuzzy.h>

AutoFuzzy fuzzy;
const int sensorPin = A0;
const int ledPin = 9;

int sensorVar = -1;
int ledVar = -1;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // 1. Define Variables
  sensorVar = fuzzy.addInput("Sensor", 0, 1023);
  ledVar = fuzzy.addOutput("LED", 0, 255);
  if (sensorVar < 0 || ledVar < 0) { Serial.println("Var Error"); while(1); }

  // 2. Define Membership Functions
  fuzzy.addTriangularMF(sensorVar, "Low", 0, 100, 300);
  fuzzy.addTriangularMF(sensorVar, "Mid", 200, 500, 800);
  fuzzy.addTriangularMF(sensorVar, "High", 700, 900, 1023);
  fuzzy.addTriangularMF(ledVar, "Dim", 0, 50, 100);
  fuzzy.addTriangularMF(ledVar, "Bright", 80, 180, 255);
  // TODO: Add error checking for MF additions

  // 3. Define Rules (using names for simplicity here)
  fuzzy.addRule("Sensor", "Low", "LED", "Dim");
  fuzzy.addRule("Sensor", "Mid", "LED", "Bright");
  fuzzy.addRule("Sensor", "High", "LED", "Dim");
  // TODO: Add error checking for rule additions
}

void loop() {
  float sensorValue = analogRead(sensorPin);

  // Prepare input struct
  FuzzyInput inputs[] = {{ (uint8_t)sensorVar, sensorValue }}; // Cast index safely
  float calculatedLedValue;

  // Evaluate for the LED output
  FuzzyResult result = fuzzy.evaluate(inputs, 1, calculatedLedValue, ledVar);

  if (result == FUZZY_OK) {
    Serial.print("Sensor: " + String(sensorValue));
    Serial.println(" -> LED: " + String(calculatedLedValue));
    analogWrite(ledPin, (int)calculatedLedValue);
  } else {
    Serial.println("Eval Error: " + String(fuzzy.getResultString(result)));
    analogWrite(ledPin, 0); // Safe state
  }
  delay(500);
}
```

---

## Advanced Usage

### Multiple Inputs & Outputs (MIMO)

```cpp
// ... (Setup inputs tempVar, humidityVar and outputs heaterVar, ventVar as in advanced example) ...

void loop() {
    float currentTemp = readTemperature();    // Read sensors
    float currentHumidity = readHumidity();

    // Prepare inputs (used for both evaluations)
    FuzzyInput fuzzyInputs[] = {
        { (uint8_t)tempVar, currentTemp },
        { (uint8_t)humidityVar, currentHumidity }
    };
    int numInputs = 2;

    float calculatedHeater, calculatedVent;
    FuzzyResult heaterRes, ventRes;

    // Evaluate for Heater output
    heaterRes = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedHeater, heaterVar);
    // Evaluate for Ventilator output
    ventRes = fuzzy.evaluate(fuzzyInputs, numInputs, calculatedVent, ventVar);

    // Process heaterRes and calculatedHeater
    // Process ventRes and calculatedVent
    // Apply outputs to actuators
    delay(2000);
}
```

### Multi-Antecedent Rules

```cpp
// ... (Assume tempVar, humidityVar, fanSpeedVar indices and MFs are defined) ...

// Get MF indices once
int tempHotMf = fuzzy.findMF(tempVar, "Hot");
int humHumidMf = fuzzy.findMF(humidityVar, "Humid");
int fanFastMf = fuzzy.findMF(fanSpeedVar, "Fast");

// Define rule conditions
AutoFuzzy::Antecedent conditions[] = {
  { tempVar, tempHotMf },      // Condition 1: Temp IS Hot
  { humidityVar, humHumidMf } // Condition 2: Humidity IS Humid
};
// Define rule action
AutoFuzzy::Consequent action = { fanSpeedVar, fanFastMf }; // THEN FanSpeed IS Fast

// Add the rule combining conditions with AND
FuzzyResult result = fuzzy.addRule(conditions, 2, FUZZY_AND, action);
if (result != FUZZY_OK) {
  Serial.println("Error adding complex rule: " + String(fuzzy.getResultString(result)));
}
```

---

## `autoTune()` Deep Dive

The `autoTune()` function provides a *heuristic* way to adjust MF parameters. It's **not** a guaranteed optimizer like gradient descent or a full genetic algorithm, but rather a randomized hill-climbing approach.

*   **Purpose:** To shift and reshape existing MFs so the fuzzy system's output more closely matches a set of known-good input/output examples (training data).
*   **Mechanism:**
    1.  Calculates the current system "fitness" based on the provided training data (how well it matches the expected outputs). Fitness is typically the Mean Squared Error (MSE).
    2.  Enters a loop for the specified number of `iterations`.
    3.  In each iteration:
        *   Makes a *copy* of the current best MF parameters found so far.
        *   **Mutates** the parameters of potentially *all* MFs randomly. Each parameter (`a`, `b`, `c`, `d`) has a `mutationRate` chance of being changed by a random amount up to `mutationRange` of the variable's span.
        *   Ensures mutated parameters stay within the variable's `min`/`max` and maintain correct order (e.g., `a <= b <= c`).
        *   Calculates the fitness of the system with the *mutated* parameters.
        *   If the mutated parameters yield a *better* fitness (lower MSE) than the previous best, they become the new "best". Otherwise, the mutation is discarded, and the system reverts to the previous best parameters.
    4.  After all iterations, the system is left with the best set of parameters found during the process.
*   **Fitness Function (Mean Squared Error - MSE):**
    ```
    MSE = (1 / numSets) * Σ [ (trainingOutputs[i] - evaluate(trainingInputs[i]))^2 ]
    ```
    The goal of `autoTune` is to minimize this MSE value for the specified `outputVarIndex`.
*   **Training Data:** You MUST provide representative training data:
    *   `trainingInputs`: Sets of input values covering the expected operating range.
    *   `trainingOutputs`: The corresponding *desired* crisp output value for each input set. Getting good training data is often the hardest part. It might come from simulations, expert knowledge, or logs of a well-performing manual controller.
*   **Limitations:**
    *   **Heuristic:** May get stuck in local minima (good, but not the best possible solution). Results depend on the starting parameters and random chance.
    *   **Tunes Everything:** It mutates parameters for MFs of *all* variables (inputs and outputs), even those not directly related to the `outputVarIndex` used for fitness calculation. This might sometimes be undesirable.
    *   **Requires Good Data:** Performance is highly dependent on the quality and coverage of the training data.
    *   **Time/Memory:** Can take significant time, especially with many iterations. Uses dynamic memory (`new`/`delete[]`) which might be problematic on very low-memory Arduinos.
    *   **Single Objective:** Optimizes based on the error for only *one* output variable at a time. For MIMO systems, you might need separate tuning runs focusing on each output, or a more complex multi-objective approach (not implemented).

---

## Error Handling

Most functions that can fail return a `FuzzyResult` enum value. Always check this value!

```cpp
FuzzyResult result = fuzzy.addRule(/* ... */);
if (result != FUZZY_OK) {
  Serial.print("Failed to add rule! Error: ");
  Serial.println(fuzzy.getResultString(result)); // Get descriptive string
  // Handle error appropriately (e.g., halt, use defaults)
}
```

**Common `FuzzyResult` Codes:**

*   `FUZZY_OK`: Operation successful.
*   `FUZZY_ERROR_TOO_MANY_...`: A limit (variables, MFs, rules, antecedents) was exceeded.
*   `FUZZY_ERROR_VAR_NOT_FOUND`: Specified variable name or index doesn't exist or is the wrong type (input/output).
*   `FUZZY_ERROR_MF_NOT_FOUND`: Specified MF name or index doesn't exist for the given variable.
*   `FUZZY_ERROR_INVALID_PARAMS`: MF parameters out of order (`a > b`), duplicate name, invalid range.
*   `FUZZY_ERROR_INVALID_RULE`: Rule structure violation (e.g., input used in consequent).
*   `FUZZY_ERROR_NULL_POINTER`: A required pointer argument (like a name or array) was null.
*   `FUZZY_ERROR_NO_RULES_FIRED`: During `evaluate`, no rules contributed to the output.
*   `FUZZY_ERROR_DIVIDE_BY_ZERO`: During `evaluate`, the sum of rule weights was zero.

---

## Memory Usage & Performance

*   **Static RAM:** Primarily determined by the `FUZZY_MAX_...` limits. Estimate:
    *   `Variable` struct: ~25-30 bytes + `FUZZY_MAX_MF_PER_VAR` * (~25 bytes)
    *   `Rule` struct: ~15-25 bytes (depends on `FUZZY_MAX_ANTECEDENTS_PER_RULE`)
    *   Total can be several KB with default limits. Measure using `freeMemory()` if concerned.
*   **Dynamic RAM:** Used only by `autoTune` to store backup parameters and temporary input arrays (`new`/`delete[]`). Size depends on `FUZZY_MAX_VARS`, `FUZZY_MAX_MF_PER_VAR`, and the number of input variables.
*   **Performance:**
    *   `evaluate()`: Time complexity is roughly proportional to `(Number of Rules affecting output) * (Avg Antecedents per Rule)`. Generally fast enough for most Arduino control loops.
    *   `autoTune()`: Can be slow, depending heavily on `iterations`, `numSets`, and the complexity of the fuzzy system evaluation. Run during `setup()` if possible, or offline.

---

## Limitations

*   Fixed set of MF shapes (Triangular, Trapezoidal).
*   Fixed fuzzy operators (MIN for AND, MAX for OR).
*   Simplified centroid defuzzification (not true Center of Gravity).
*   `evaluate()` calculates only one output variable per call.
*   `autoTune()` is a basic heuristic tuner, not a guaranteed optimizer, and uses dynamic memory.
*   String operations (name lookups) are less efficient than using indices.

---

## Troubleshooting & Best Practices

*   **Check Return Codes:** The most common source of issues is ignoring errors during setup.
*   **Verify Logic:** Does `No Rules Fired` occur? Ensure your rules cover all expected input combinations. Check MF overlap – inputs should ideally always have *some* degree of membership in at least one MF.
*   **Input Ranges:** Ensure crisp input values passed to `evaluate` are within the `min`/`max` range defined for the variables (or handle clamping yourself). `evaluate` clamps the *output*.
*   **`autoTune` Issues:** If tuning doesn't improve things or makes them worse: check training data quality, adjust `mutationRate`/`mutationRange` (smaller values make smaller steps), increase `iterations`, or reconsider if the MF structure itself is suitable.
*   **Memory:** If running out of RAM, reduce the `FUZZY_MAX_...` limits or simplify your fuzzy system (fewer variables/MFs/rules). Avoid running `autoTune` on very low-memory boards if possible.
*   **Start Simple:** Build your fuzzy system incrementally. Test SISO parts before combining them into MIMO.

## Contributing

Contributions are welcome! Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file for detailed guidelines.

## License

This library is released under the MIT License. See the [LICENSE](LICENSE) file for details.
