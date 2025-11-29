# AutoFuzzy Usage Guide

This comprehensive guide provides detailed instructions for implementing fuzzy logic controllers using the AutoFuzzy library in Arduino projects.

## Prerequisites

### System Requirements
- Arduino IDE 1.8.x or Arduino CLI
- Arduino board with minimum 2KB RAM (Uno, Nano, Mega recommended)
- AutoFuzzy library installed

### Library Installation
1. Download from Arduino Library Manager: `Sketch > Include Library > Manage Libraries`
2. Search for "AutoFuzzy" and install
3. Or manually copy to `~/Arduino/libraries/` directory

### Header Inclusion
```cpp
// Include the library header
#include <AutoFuzzy.h>

// Optional: Configure memory limits before inclusion
#define FUZZY_MAX_VARS 4
#define FUZZY_MAX_RULES 10
#include <AutoFuzzy.h>
```

## Implementation Workflow

The AutoFuzzy implementation follows a structured approach:

### 1. System Initialization

Create an AutoFuzzy instance to manage the fuzzy logic system:

```cpp
// Create fuzzy logic controller instance
AutoFuzzy fuzzyController;

// Alternative: Use pointer for dynamic allocation if needed
// AutoFuzzy* fuzzyPtr = new AutoFuzzy();
```

**Design Consideration**: Use a single instance for related control variables to maintain system coherence.

### 2. Variable Definition

Define linguistic variables representing system inputs and outputs:

#### Input Variables
```cpp
// Define input variables with descriptive names and valid ranges
int temperatureVar = fuzzyController.addInput("Temperature", 0.0f, 50.0f);
int humidityVar = fuzzyController.addInput("Humidity", 0.0f, 100.0f);

// Validate successful creation
if (temperatureVar < 0 || humidityVar < 0) {
    Serial.println("Failed to create input variables");
    // Handle error - system cannot function without inputs
}
```

#### Output Variables
```cpp
// Define output variables for actuator control
int fanSpeedVar = fuzzyController.addOutput("FanSpeed", 0.0f, 255.0f);
int heaterPowerVar = fuzzyController.addOutput("HeaterPower", 0.0f, 100.0f);

// Validate successful creation
if (fanSpeedVar < 0 || heaterPowerVar < 0) {
    Serial.println("Failed to create output variables");
    // Handle error - system cannot function without outputs
}
```

**Critical Implementation Notes:**
- Return values must be validated (≥0 indicates success, -1 indicates failure)
- Variable ranges should match expected sensor/actuator capabilities
- Store returned indices for efficient subsequent operations
- Variable names must be unique within the system

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

### 3. Membership Function Configuration

Define fuzzy sets for each linguistic variable using membership functions:

#### Triangular Membership Functions
```cpp
// Temperature fuzzy sets using triangular functions
FuzzyResult result;

// Cold: Sharp peak, gradual decrease
result = fuzzyController.addTriangularMF(temperatureVar, "Cold", 0.0f, 10.0f, 20.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add Cold MF: ");
    Serial.println(fuzzyController.getResultString(result));
}

// Comfortable: Centered distribution
result = fuzzyController.addTriangularMF(temperatureVar, "Comfortable", 18.0f, 25.0f, 32.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add Comfortable MF: ");
    Serial.println(fuzzyController.getResultString(result));
}

// Hot: Sharp increase, gradual decrease
result = fuzzyController.addTriangularMF(temperatureVar, "Hot", 28.0f, 38.0f, 50.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add Hot MF: ");
    Serial.println(fuzzyController.getResultString(result));
}
```

#### Trapezoidal Membership Functions
```cpp
// Fan speed fuzzy sets using trapezoidal functions for smoother control
result = fuzzyController.addTrapezoidalMF(fanSpeedVar, "Off", 0.0f, 0.0f, 10.0f, 30.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add Off MF: ");
    Serial.println(fuzzyController.getResultString(result));
}

result = fuzzyController.addTrapezoidalMF(fanSpeedVar, "Low", 20.0f, 60.0f, 80.0f, 120.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add Low MF: ");
    Serial.println(fuzzyController.getResultString(result));
}

result = fuzzyController.addTrapezoidalMF(fanSpeedVar, "High", 100.0f, 180.0f, 220.0f, 255.0f);
if (result != FUZZY_OK) {
    Serial.print("Failed to add High MF: ");
    Serial.println(fuzzyController.getResultString(result));
}
```

**Parameter Validation Requirements:**
- Triangular: `a ≤ b ≤ c` (left foot ≤ peak ≤ right foot)
- Trapezoidal: `a ≤ b ≤ c ≤ d` (left foot ≤ left shoulder ≤ right shoulder ≤ right foot)
- Parameters must be within variable's defined range
- Function names must be unique within each variable

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

### 4. Fuzzy Rule Definition

Establish the logical relationships between inputs and outputs through fuzzy rules:

#### Single Input Single Output (SISO) Rules

**Using Variable Names (Convenient for development):**
```cpp
FuzzyResult result;

// Basic rules for temperature control
result = fuzzyController.addRule("Temperature", "Cold", "FanSpeed", "Off");
if (result != FUZZY_OK) {
    Serial.print("Rule 1 failed: ");
    Serial.println(fuzzyController.getResultString(result));
}

result = fuzzyController.addRule("Temperature", "Comfortable", "FanSpeed", "Low");
if (result != FUZZY_OK) {
    Serial.print("Rule 2 failed: ");
    Serial.println(fuzzyController.getResultString(result));
}

result = fuzzyController.addRule("Temperature", "Hot", "FanSpeed", "High");
if (result != FUZZY_OK) {
    Serial.print("Rule 3 failed: ");
    Serial.println(fuzzyController.getResultString(result));
}
```

**Using Indices (Recommended for performance):**
```cpp
// Pre-calculate indices for efficiency
int tempColdIdx = fuzzyController.findMF(temperatureVar, "Cold");
int tempComfortableIdx = fuzzyController.findMF(temperatureVar, "Comfortable");
int tempHotIdx = fuzzyController.findMF(temperatureVar, "Hot");
int fanOffIdx = fuzzyController.findMF(fanSpeedVar, "Off");
int fanLowIdx = fuzzyController.findMF(fanSpeedVar, "Low");
int fanHighIdx = fuzzyController.findMF(fanSpeedVar, "High");

// Validate all indices found
if (tempColdIdx < 0 || tempComfortableIdx < 0 || tempHotIdx < 0 ||
    fanOffIdx < 0 || fanLowIdx < 0 || fanHighIdx < 0) {
    Serial.println("One or more membership functions not found");
    return;
}

// Define and add rules
AutoFuzzy::Antecedent coldCondition = {temperatureVar, tempColdIdx};
AutoFuzzy::Consequent offAction = {fanSpeedVar, fanOffIdx};
result = fuzzyController.addRule(coldCondition, offAction);

AutoFuzzy::Antecedent comfortableCondition = {temperatureVar, tempComfortableIdx};
AutoFuzzy::Consequent lowAction = {fanSpeedVar, fanLowIdx};
result = fuzzyController.addRule(comfortableCondition, lowAction);

AutoFuzzy::Antecedent hotCondition = {temperatureVar, tempHotIdx};
AutoFuzzy::Consequent highAction = {fanSpeedVar, fanHighIdx};
result = fuzzyController.addRule(hotCondition, highAction);
```

#### Multiple Input Rules (MIMO)

**AND Combination Rules:**
```cpp
// Rule: IF Temperature IS Hot AND Humidity IS High THEN FanSpeed IS High
int humidityHighIdx = fuzzyController.findMF(humidityVar, "High");

AutoFuzzy::Antecedent hotTempCondition = {temperatureVar, tempHotIdx};
AutoFuzzy::Antecedent highHumidityCondition = {humidityVar, humidityHighIdx};
AutoFuzzy::Antecedent hotAndHumidConditions[] = {hotTempCondition, highHumidityCondition};

AutoFuzzy::Consequent highFanAction = {fanSpeedVar, fanHighIdx};

result = fuzzyController.addRule(hotAndHumidConditions, 2, FUZZY_AND, highFanAction);
if (result != FUZZY_OK) {
    Serial.print("Multi-condition rule failed: ");
    Serial.println(fuzzyController.getResultString(result));
}
```

**OR Combination Rules:**
```cpp
// Rule: IF Temperature IS Cold OR Humidity IS Low THEN HeaterPower IS High
int humidityLowIdx = fuzzyController.findMF(humidityVar, "Low");
int heaterHighIdx = fuzzyController.findMF(heaterPowerVar, "High");

AutoFuzzy::Antecedent coldTempCondition = {temperatureVar, tempColdIdx};
AutoFuzzy::Antecedent lowHumidityCondition = {humidityVar, humidityLowIdx};
AutoFuzzy::Antecedent coldOrDryConditions[] = {coldTempCondition, lowHumidityCondition};

AutoFuzzy::Consequent highHeaterAction = {heaterPowerVar, heaterHighIdx};

result = fuzzyController.addRule(coldOrDryConditions, 2, FUZZY_OR, highHeaterAction);
```

**Rule Validation Requirements:**
- Antecedents must reference input variables only
- Consequents must reference output variables only
- All referenced variables and membership functions must exist
- Rule count must not exceed FUZZY_MAX_RULES limit

### 5. Input Data Preparation

Prepare sensor data for fuzzy evaluation in the control loop:

```cpp
void loop() {
    // Read sensor values with appropriate scaling
    float currentTemperature = readTemperatureSensor();     // e.g., 0.0-50.0°C
    float currentHumidity = readHumiditySensor();          // e.g., 0.0-100.0%

    // Validate input ranges (optional but recommended)
    currentTemperature = constrain(currentTemperature, 0.0f, 50.0f);
    currentHumidity = constrain(currentHumidity, 0.0f, 100.0f);

    // Prepare fuzzy input array - must include ALL input variables
    FuzzyInput sensorInputs[] = {
        {(uint8_t)temperatureVar, currentTemperature},
        {(uint8_t)humidityVar, currentHumidity}
    };
    const int numInputs = 2;

    // Optional: Log input values for debugging
    Serial.print("Inputs - Temp: ");
    Serial.print(currentTemperature);
    Serial.print("°C, Humidity: ");
    Serial.print(currentHumidity);
    Serial.println("%");
}
```

**Input Validation Best Practices:**
- Ensure all input variables used by rules are provided
- Clamp sensor values to defined variable ranges
- Handle sensor read failures gracefully
- Consider input filtering for noisy sensors

### 6. Fuzzy System Evaluation

Execute the fuzzy inference process to generate control outputs:

#### Single Output Evaluation
```cpp
void loop() {
    // ... prepare sensorInputs array ...

    float fanSpeedOutput;

    // Evaluate fuzzy system for fan speed control
    FuzzyResult evaluationResult = fuzzyController.evaluate(
        sensorInputs,           // Input data array
        numInputs,              // Number of inputs provided
        fanSpeedOutput,         // Output result storage
        fanSpeedVar             // Target output variable index
    );

    // Process evaluation result
    switch (evaluationResult) {
        case FUZZY_OK:
            // Successful evaluation - apply control output
            analogWrite(FAN_PIN, (int)fanSpeedOutput);
            Serial.print("Fan Speed: ");
            Serial.println(fanSpeedOutput);
            break;

        case FUZZY_ERROR_NO_RULES_FIRED:
            // No rules activated - use safe default
            analogWrite(FAN_PIN, 0);  // Safe stop
            Serial.println("Warning: No rules fired - using safe default");
            break;

        default:
            // System error - implement failsafe
            analogWrite(FAN_PIN, 0);
            Serial.print("Fuzzy evaluation error: ");
            Serial.println(fuzzyController.getResultString(evaluationResult));
            // Consider system reset or error state
            break;
    }

    delay(500); // Control loop timing
}
```

#### Multiple Output Evaluation (MIMO)
```cpp
void loop() {
    // ... prepare sensorInputs array ...

    float fanSpeedOutput, heaterPowerOutput;

    // Evaluate multiple outputs using same input data
    FuzzyResult fanResult = fuzzyController.evaluate(
        sensorInputs, numInputs, fanSpeedOutput, fanSpeedVar);

    FuzzyResult heaterResult = fuzzyController.evaluate(
        sensorInputs, numInputs, heaterPowerOutput, heaterPowerVar);

    // Apply control outputs with error handling
    if (fanResult == FUZZY_OK) {
        int pwmValue = map(fanSpeedOutput, 0, 255, 0, 255);
        analogWrite(FAN_PIN, pwmValue);
    } else {
        analogWrite(FAN_PIN, 0); // Safe default
    }

    if (heaterResult == FUZZY_OK) {
        int powerValue = map(heaterPowerOutput, 0, 100, 0, 255);
        analogWrite(HEATER_PIN, powerValue);
    } else {
        analogWrite(HEATER_PIN, 0); // Safe default
    }

    // Optional: Combined error reporting
    if (fanResult != FUZZY_OK || heaterResult != FUZZY_OK) {
        Serial.println("One or more outputs failed evaluation");
    }
}
```

**Evaluation Performance Considerations:**
- Reuse input arrays across multiple output evaluations
- Implement appropriate control loop timing (100-1000ms typical)
- Monitor for consistent rule activation
- Consider output smoothing for stable control

### 7. Parameter Auto-Tuning (Advanced)

Optimize membership function parameters using training data:

#### Training Data Preparation
```cpp
// Define representative operating scenarios
const int TRAINING_SAMPLES = 8;

// Input scenarios (must match variable addition order)
float trainingInputs[TRAINING_SAMPLES][2] = {
    // {Temperature, Humidity}
    {5.0,  30.0},   // Cold, Dry - expect low fan speed
    {5.0,  70.0},   // Cold, Humid - expect medium fan speed
    {15.0, 30.0},   // Cool, Dry - expect low fan speed
    {15.0, 70.0},   // Cool, Humid - expect medium fan speed
    {25.0, 30.0},   // Warm, Dry - expect medium fan speed
    {25.0, 70.0},   // Warm, Humid - expect high fan speed
    {35.0, 30.0},   // Hot, Dry - expect high fan speed
    {35.0, 70.0}    // Hot, Humid - expect maximum fan speed
};

// Expected outputs for each scenario
float trainingOutputs[TRAINING_SAMPLES] = {
    20.0,   // Low speed for cold/dry
    40.0,   // Medium-low for cold/humid
    30.0,   // Low-medium for cool/dry
    50.0,   // Medium for cool/humid
    60.0,   // Medium for warm/dry
    80.0,   // Medium-high for warm/humid
    90.0,   // High for hot/dry
    100.0   // Maximum for hot/humid
};
```

#### Auto-Tuning Execution
```cpp
void setup() {
    // ... initialize variables, MFs, and rules first ...

    // Prepare training data pointers
    float* inputPointers[TRAINING_SAMPLES];
    for (int i = 0; i < TRAINING_SAMPLES; i++) {
        inputPointers[i] = trainingInputs[i];
    }

    Serial.println("Starting parameter auto-tuning...");

    // Execute heuristic optimization
    FuzzyResult tuningResult = fuzzyController.autoTune(
        inputPointers,              // Training input data
        trainingOutputs,            // Expected output data
        TRAINING_SAMPLES,           // Number of training samples
        fanSpeedVar,                // Output variable to optimize for
        200,                        // Optimization iterations
        0.15f,                      // Mutation rate (15% chance per parameter)
        0.10f                       // Mutation range (10% of variable range)
    );

    if (tuningResult == FUZZY_OK) {
        Serial.println("Auto-tuning completed successfully");
        Serial.println("Membership functions have been optimized");
    } else {
        Serial.print("Auto-tuning failed: ");
        Serial.println(fuzzyController.getResultString(tuningResult));
        Serial.println("Using original parameters");
    }
}
```

**Tuning Parameter Guidelines:**
- **Iterations**: 100-500 (higher values may improve results but increase time)
- **Mutation Rate**: 0.05-0.20 (probability of parameter modification per iteration)
- **Mutation Range**: 0.05-0.15 (fraction of variable range for random changes)
- **Training Data**: Must cover full expected operating range comprehensively

**Important Considerations:**
- Auto-tuning is heuristic and may not find optimal parameters
- Requires sufficient representative training data
- Computationally intensive - execute during setup, not loop
- Uses dynamic memory allocation temporarily
- Results may vary between runs due to random nature

## System Configuration

### Memory Limit Customization

Override default limits for specific application requirements:

```cpp
// Define limits BEFORE including header (recommended approach)
#define FUZZY_MAX_VARS 8              // Custom variable count for MIMO systems
#define FUZZY_MAX_MF_PER_VAR 6        // Custom MF count per variable
#define FUZZY_MAX_RULES 20            // Custom rule count for complex systems
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 4
#define FUZZY_MAX_NAME_LEN 15

#include <AutoFuzzy.h>

AutoFuzzy fuzzyController;
```

**Note:** Examples use Arduino Mega defaults (6 variables, 15 rules). Define custom limits before including the header to avoid One Definition Rule violations.

### Platform-Specific Optimization

**Arduino Uno/Memory-Constrained Boards:**
```cpp
// Conservative settings for 2KB RAM boards
#define FUZZY_MAX_VARS 3
#define FUZZY_MAX_RULES 6
#define FUZZY_MAX_MF_PER_VAR 3
```

**Arduino Mega/High-Memory Boards (Default Target):**
```cpp
// Optimized settings for 8KB RAM boards (current default)
#define FUZZY_MAX_VARS 6
#define FUZZY_MAX_MF_PER_VAR 5
#define FUZZY_MAX_RULES 15
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 3
```

**ESP32/High-Performance Boards:**
```cpp
// Maximum settings for advanced applications
#define FUZZY_MAX_VARS 12
#define FUZZY_MAX_MF_PER_VAR 8
#define FUZZY_MAX_RULES 50
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 5
```

## Development Best Practices

### Error Handling Strategy
```cpp
// Comprehensive error checking pattern
#define FUZZY_CHECK_RESULT(operation, context) \
    do { \
        FuzzyResult result = operation; \
        if (result != FUZZY_OK) { \
            Serial.print("Error in "); \
            Serial.print(context); \
            Serial.print(": "); \
            Serial.println(fuzzyController.getResultString(result)); \
            return false; \
        } \
    } while(0)

// Usage in setup
bool initializeFuzzySystem() {
    FUZZY_CHECK_RESULT(fuzzyController.addInput("Temp", 0, 50), "input creation");
    FUZZY_CHECK_RESULT(fuzzyController.addTriangularMF(0, "Cold", 0, 10, 20), "MF creation");
    // ... continue with other operations
    return true;
}
```

### Performance Optimization

1. **Index Caching:**
```cpp
// Cache frequently used indices
static int tempVarIdx, fanVarIdx;
static int tempColdIdx, fanLowIdx;

void setup() {
    // Initialize once
    tempVarIdx = fuzzyController.findVariable("Temperature");
    fanVarIdx = fuzzyController.findVariable("FanSpeed");
    tempColdIdx = fuzzyController.findMF(tempVarIdx, "Cold");
    fanLowIdx = fuzzyController.findMF(fanVarIdx, "Low");
}
```

2. **Efficient Evaluation:**
```cpp
// Minimize evaluation calls in time-critical code
static unsigned long lastEvaluation = 0;
const unsigned long EVALUATION_INTERVAL = 250; // ms

void loop() {
    if (millis() - lastEvaluation >= EVALUATION_INTERVAL) {
        // Perform fuzzy evaluation
        lastEvaluation = millis();
    }
}
```

### Testing and Validation

1. **Boundary Testing:**
```cpp
void testBoundaryConditions() {
    // Test extreme input values
    FuzzyInput testInputs[] = {{tempVarIdx, 0.0f}};   // Minimum
    float output;
    fuzzyController.evaluate(testInputs, 1, output, fanVarIdx);
    Serial.println("Min input result: " + String(output));

    testInputs[0].value = 50.0f;  // Maximum
    fuzzyController.evaluate(testInputs, 1, output, fanVarIdx);
    Serial.println("Max input result: " + String(output));
}
```

2. **Rule Coverage Verification:**
```cpp
void validateRuleCoverage() {
    // Test various input combinations
    float testCases[][2] = {
        {10, 30}, {10, 70}, {30, 30}, {30, 70}
    };

    for (auto& testCase : testCases) {
        FuzzyInput inputs[] = {
            {tempVarIdx, testCase[0]},
            {humidityVarIdx, testCase[1]}
        };

        float output;
        FuzzyResult result = fuzzyController.evaluate(inputs, 2, output, fanVarIdx);

        if (result == FUZZY_ERROR_NO_RULES_FIRED) {
            Serial.print("Missing rule coverage for input: ");
            Serial.print(testCase[0]); Serial.print(", ");
            Serial.println(testCase[1]);
        }
    }
}
```

### Documentation Standards

Maintain clear documentation for fuzzy system design:

```cpp
/*
 * Fuzzy Climate Control System - Fan Speed Controller
 *
 * Input Variables:
 * - Temperature: 0-50°C
 *   - Cold: Triangular(0, 10, 20)
 *   - Comfortable: Triangular(15, 25, 35)
 *   - Hot: Triangular(30, 40, 50)
 *
 * - Humidity: 0-100%
 *   - Dry: Triangular(0, 25, 50)
 *   - Humid: Triangular(50, 75, 100)
 *
 * Output Variables:
 * - FanSpeed: 0-255 PWM
 *   - Off: Triangular(0, 0, 50)
 *   - Low: Triangular(25, 75, 125)
 *   - High: Triangular(100, 175, 255)
 *
 * Rule Base:
 * 1. IF Temp=Cold THEN Fan=Off
 * 2. IF Temp=Comfortable AND Humidity=Dry THEN Fan=Low
 * 3. IF Temp=Comfortable AND Humidity=Humid THEN Fan=High
 * 4. IF Temp=Hot THEN Fan=High
 *
 * Memory Usage: ~1.8KB (Arduino Uno compatible)
 * Control Frequency: 4Hz (250ms intervals)
 */
```

This comprehensive usage guide provides the foundation for implementing robust fuzzy logic control systems with the AutoFuzzy library.
