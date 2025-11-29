# AutoFuzzy Troubleshooting Guide

This guide provides solutions to common issues encountered when using the AutoFuzzy library, along with diagnostic procedures and preventive measures.

## Quick Diagnosis Checklist

Before diving into specific issues, run through this checklist:

### Basic Setup Verification
- [ ] Library installed correctly in Arduino/libraries folder
- [ ] `#include <AutoFuzzy.h>` statement present
- [ ] Library compiles without errors
- [ ] Board selection matches target hardware
- [ ] Serial monitor configured (if using debug output)

### Configuration Check
- [ ] Memory limits defined before header inclusion (if customized)
- [ ] Variable ranges match expected input values
- [ ] Membership function parameters are valid
- [ ] Rules reference existing variables and membership functions

### Runtime Verification
- [ ] Input values within defined variable ranges
- [ ] FuzzyInput array contains all required variables
- [ ] Output evaluation targets valid output variables
- [ ] Error codes checked and handled appropriately

## Compilation Issues

### Problem: "AutoFuzzy.h: No such file or directory"

**Symptoms:**
```
fatal error: AutoFuzzy.h: No such file or directory
```

**Solutions:**

1. **Library Installation**
   ```bash
   # Option 1: Arduino IDE
   Sketch > Include Library > Manage Libraries
   Search for "AutoFuzzy" and install

   # Option 2: Manual installation
   Copy AutoFuzzy folder to ~/Arduino/libraries/
   Restart Arduino IDE
   ```

2. **Include Path Issues**
   - Ensure library is in correct Arduino/libraries directory
   - Check for case sensitivity in include statement
   - Verify library folder name matches exactly

3. **Arduino CLI Issues**
   ```bash
   # Check library installation
   arduino-cli lib list | grep AutoFuzzy

   # Install if missing
   arduino-cli lib install AutoFuzzy
   ```

### Problem: "array types have different bounds"

**Symptoms:**
```
error: array types have different bounds
class AutoFuzzy {
    Variable vars[FUZZY_MAX_VARS];
```

**Cause:**
Memory configuration macros defined after header inclusion.

**Solution:**
```cpp
// ❌ Wrong - defines after include
#include <AutoFuzzy.h>
#define FUZZY_MAX_VARS 5

// ✅ Correct - defines before include
#define FUZZY_MAX_VARS 5
#include <AutoFuzzy.h>
```

### Problem: "Not enough memory" or "data section exceeds"

**Symptoms:**
```
Global variables use 101% of dynamic memory
Not enough memory; see memory tips
```

**Solutions:**

1. **Reduce Memory Configuration**
   ```cpp
   #define FUZZY_MAX_VARS 3
   #define FUZZY_MAX_RULES 6
   #define FUZZY_MAX_MF_PER_VAR 3
   #include <AutoFuzzy.h>
   ```

2. **Use Larger Board**
   - Arduino Uno: 2KB RAM → Arduino Mega: 8KB RAM
   - ESP32: 520KB RAM (recommended for complex systems)

3. **Optimize Fuzzy System**
   - Reduce number of membership functions
   - Combine similar rules
   - Use triangular instead of trapezoidal functions

## Runtime Issues

### Problem: FUZZY_ERROR_NO_RULES_FIRED

**Symptoms:**
```
Evaluation returns FUZZY_ERROR_NO_RULES_FIRED
Output values are default/minimum
```

**Causes and Solutions:**

1. **Input Values Outside MF Ranges**
   ```cpp
   // Check input coverage
   float testValue = 25.0;
   FuzzyInput inputs[] = {{tempVar, testValue}};

   // Verify MFs cover this range
   // Add MFs if needed
   fuzzy.addTriangularMF(tempVar, "Normal", 20, 25, 30);
   ```

2. **Insufficient MF Overlap**
   ```cpp
   // Ensure smooth transitions between MFs
   // Bad: Gap between MFs
   fuzzy.addTriangularMF(var, "Low", 0, 10, 20);
   fuzzy.addTriangularMF(var, "High", 30, 40, 50); // Gap at 20-30

   // Good: Overlapping MFs
   fuzzy.addTriangularMF(var, "Low", 0, 10, 25);
   fuzzy.addTriangularMF(var, "Medium", 15, 25, 35);
   fuzzy.addTriangularMF(var, "High", 25, 35, 50);
   ```

3. **Missing Rules for Input Combinations**
   ```cpp
   // Ensure rules cover all expected scenarios
   fuzzy.addRule("Temp", "Low", "Fan", "Off");
   fuzzy.addRule("Temp", "Medium", "Fan", "Low");
   fuzzy.addRule("Temp", "High", "Fan", "High");
   ```

### Problem: Unexpected Output Values

**Symptoms:**
```
Fuzzy output doesn't match expected behavior
Output values seem random or incorrect
```

**Diagnostic Steps:**

1. **Verify Input Processing**
   ```cpp
   void debugInputs(const FuzzyInput inputs[], int numInputs) {
       for (int i = 0; i < numInputs; i++) {
           Serial.print("Input ");
           Serial.print(inputs[i].varIndex);
           Serial.print(": ");
           Serial.println(inputs[i].value);
       }
   }
   ```

2. **Check Membership Function Calculations**
   ```cpp
   void debugMembership(int varIndex, float value) {
       int mfCount = fuzzy.getVariable(varIndex)->mfCount;
       for (int i = 0; i < mfCount; i++) {
           const MembershipFunction* mf = fuzzy.getMF(varIndex, i);
           float membership = fuzzy.calculateMembership(*mf, value);
           Serial.print("MF ");
           Serial.print(mf->name);
           Serial.print(": ");
           Serial.println(membership);
       }
   }
   ```

3. **Validate Rule Activation**
   ```cpp
   // Add debug output to check which rules fire
   FuzzyResult result = fuzzy.evaluate(inputs, numInputs, output, outputVar);
   if (result == FUZZY_ERROR_NO_RULES_FIRED) {
       Serial.println("No rules activated - check MF coverage");
   }
   ```

4. **Test Individual Components**
   ```cpp
   // Test MF calculation directly
   MembershipFunction testMF = {"Test", MF_TRIANGULAR, {0, 10, 20, 0}};
   float membership = fuzzy.calculateMembership(testMF, 10.0);
   Serial.println("Expected: 1.0, Got: " + String(membership));
   ```

### Problem: FUZZY_ERROR_INVALID_PARAMS

**Symptoms:**
```
Function returns FUZZY_ERROR_INVALID_PARAMS
Membership function or rule addition fails
```

**Common Causes:**

1. **Invalid MF Parameters**
   ```cpp
   // Triangular MF requires a ≤ b ≤ c
   // ❌ Invalid
   fuzzy.addTriangularMF(var, "Bad", 20, 10, 30); // b < a

   // ✅ Valid
   fuzzy.addTriangularMF(var, "Good", 10, 20, 30); // a ≤ b ≤ c
   ```

2. **Trapezoidal MF Parameter Order**
   ```cpp
   // Requires a ≤ b ≤ c ≤ d
   // ❌ Invalid
   fuzzy.addTrapezoidalMF(var, "Bad", 10, 30, 20, 40); // b > c

   // ✅ Valid
   fuzzy.addTrapezoidalMF(var, "Good", 10, 20, 30, 40); // a ≤ b ≤ c ≤ d
   ```

3. **Rule Validation Issues**
   ```cpp
   // Rules must connect inputs to outputs
   // ❌ Invalid: input to input
   fuzzy.addRule("Temp", "High", "Humidity", "Low");

   // ✅ Valid: input to output
   fuzzy.addRule("Temp", "High", "Fan", "High");
   ```

### Problem: Memory Corruption or Crashes

**Symptoms:**
```
System becomes unresponsive
Unexpected behavior after evaluation
Memory-related crashes
```

**Prevention and Solutions:**

1. **Array Bounds Checking**
   ```cpp
   // Always validate indices before use
   int varIndex = fuzzy.findVariable("Temp");
   if (varIndex < 0) {
       Serial.println("Variable not found");
       return;
   }
   ```

2. **Input Array Validation**
   ```cpp
   // Ensure input array matches system requirements
   if (numInputs != expectedInputCount) {
       Serial.println("Incorrect number of inputs");
       return;
   }
   ```

3. **Memory Limit Awareness**
   ```cpp
   // Monitor memory usage
   void printMemoryUsage() {
       Serial.print("Free RAM: ");
       Serial.println(freeMemory());
   }
   ```

## Auto-Tuning Issues

### Problem: Auto-Tuning Doesn't Improve Results

**Symptoms:**
```
autoTune completes but results unchanged or worse
MSE doesn't decrease significantly
```

**Solutions:**

1. **Improve Training Data**
   ```cpp
   // Use more representative training data
   const int TRAINING_SETS = 20; // Increase from 3
   float inputs[TRAINING_SETS][numInputs] = {
       // Cover full operating range
       {0.0, 0.0},   // Min conditions
       {25.0, 50.0}, // Normal conditions
       {50.0, 100.0} // Max conditions
       // Add more intermediate points
   };
   ```

2. **Adjust Tuning Parameters**
   ```cpp
   // More aggressive tuning
   fuzzy.autoTune(trainInputs, trainOutputs, numSets,
                   outputVar, 200,    // More iterations
                   0.2f,   // Higher mutation rate
                   0.15f); // Higher mutation range
   ```

3. **Validate Training Data Quality**
   ```cpp
   // Check that training outputs are reasonable
   for (int i = 0; i < numSets; i++) {
       if (trainOutputs[i] < outputMin || trainOutputs[i] > outputMax) {
           Serial.println("Invalid training output at index " + String(i));
       }
   }
   ```

### Problem: Auto-Tuning Causes System Instability

**Symptoms:**
```
System becomes unstable after auto-tuning
Parameters become unreasonable
```

**Solutions:**

1. **Constrain Parameter Ranges**
   ```cpp
   // Ensure parameters stay within reasonable bounds
   // This is handled automatically by the library
   ```

2. **Use Conservative Tuning**
   ```cpp
   fuzzy.autoTune(trainInputs, trainOutputs, numSets,
                   outputVar, 50,     // Fewer iterations
                   0.05f,  // Lower mutation rate
                   0.05f); // Smaller mutations
   ```

3. **Backup Original Parameters**
   ```cpp
   // Save original state before tuning
   // (Library handles this internally)
   FuzzyResult result = fuzzy.autoTune(...);
   if (result != FUZZY_OK) {
       Serial.println("Tuning failed - parameters unchanged");
   }
   ```

## Performance Issues

### Problem: Evaluation Too Slow

**Symptoms:**
```
Control loop runs slower than expected
Real-time requirements not met
```

**Optimizations:**

1. **Reduce Evaluation Frequency**
   ```cpp
   static unsigned long lastEval = 0;
   if (millis() - lastEval >= 100) { // Evaluate every 100ms
       // Perform fuzzy evaluation
       lastEval = millis();
   }
   ```

2. **Optimize Rule Count**
   ```cpp
   // Use fewer, more general rules
   // Combine rules where possible
   fuzzy.addRule(antecedents, 2, FUZZY_AND, consequent); // Multi-antecedent
   ```

3. **Pre-calculate Indices**
   ```cpp
   // Store indices to avoid name lookups
   static int tempVar = fuzzy.findVariable("Temperature");
   static int outputVar = fuzzy.findVariable("FanSpeed");
   ```

### Problem: High Memory Usage

**Symptoms:**
```
Approaching memory limits
System unstable with additional features
```

**Memory Optimization:**

1. **Minimal Configuration**
   ```cpp
   #define FUZZY_MAX_VARS 2
   #define FUZZY_MAX_RULES 4
   #define FUZZY_MAX_MF_PER_VAR 2
   ```

2. **Efficient Data Types**
   ```cpp
   // Use appropriate precision
   float preciseValue = 25.567; // Full precision when needed
   int roundedValue = (int)preciseValue; // Integer when sufficient
   ```

3. **Avoid Dynamic Memory in Loops**
   ```cpp
   // Don't allocate in time-critical code
   // Use static arrays where possible
   ```

## Platform-Specific Issues

### Arduino Uno Specific

**Memory Constraints:**
```cpp
// Maximum safe configuration for Uno
#define FUZZY_MAX_VARS 3
#define FUZZY_MAX_RULES 6
#define FUZZY_MAX_MF_PER_VAR 3
```

**Floating Point Precision:**
```cpp
// Uno has limited float precision
// Use reasonable precision ranges
fuzzy.addInput("Temp", 0.0f, 50.0f); // 0.1°C resolution adequate
```

### ESP32 Specific

**Memory Abundance:**
```cpp
// ESP32 can handle larger systems
#define FUZZY_MAX_VARS 8
#define FUZZY_MAX_RULES 25
#define FUZZY_MAX_MF_PER_VAR 5
```

**Performance Optimization:**
```cpp
// ESP32 benefits from more complex systems
// Additional MFs for better precision
fuzzy.addTrapezoidalMF(var, "VeryLow", 0, 5, 10, 15);
fuzzy.addTrapezoidalMF(var, "Low", 10, 15, 20, 25);
```

## Diagnostic Tools

### Debug Output Functions

```cpp
void debugFuzzySystem() {
    Serial.println("=== Fuzzy System Debug ===");

    // System configuration
    Serial.print("Variables: ");
    Serial.println(fuzzy.varCount);
    Serial.print("Rules: ");
    Serial.println(fuzzy.ruleCount);

    // Variable details
    for (int i = 0; i < fuzzy.varCount; i++) {
        const Variable* var = fuzzy.getVariable(i);
        Serial.print("Var ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(var->name);
        Serial.print(" (");
        Serial.print(var->isInput ? "Input" : "Output");
        Serial.print(") Range: ");
        Serial.print(var->min);
        Serial.print("-");
        Serial.println(var->max);
    }
}

void debugEvaluation(const FuzzyInput inputs[], int numInputs, int outputVar) {
    Serial.println("=== Evaluation Debug ===");

    // Input values
    for (int i = 0; i < numInputs; i++) {
        Serial.print("Input ");
        Serial.print(inputs[i].varIndex);
        Serial.print(": ");
        Serial.println(inputs[i].value);
    }

    // Rule activations
    float output;
    FuzzyResult result = fuzzy.evaluate(inputs, numInputs, output, outputVar);

    Serial.print("Result: ");
    Serial.println(fuzzy.getResultString(result));
    Serial.print("Output: ");
    Serial.println(output);
}
```

### Memory Monitoring

```cpp
// For AVR boards
#ifdef __AVR__
int freeMemory() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

void monitorMemory() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 5000) { // Check every 5 seconds
        Serial.print("Free RAM: ");
        Serial.println(freeMemory());
        lastCheck = millis();
    }
}
```

## Preventive Measures

### Development Best Practices

1. **Incremental Development**
   ```cpp
   // Build system step by step
   void setup() {
       // Step 1: Basic variables
       addBasicVariables();

       // Step 2: Test basic functionality
       testBasicEvaluation();

       // Step 3: Add complexity
       addMembershipFunctions();
       addRules();
   }
   ```

2. **Comprehensive Testing**
   ```cpp
   void runSystemTests() {
       testBoundaryConditions();
       testNormalOperation();
       testErrorConditions();
       testMemoryUsage();
   }
   ```

3. **Version Control Integration**
   ```cpp
   // Document fuzzy system parameters
   /*
   FUZZY_SYSTEM_CONFIG v1.2
   - Temperature: Triangular MFs (0,15,30), (20,35,50)
   - Output: Triangular MFs (0,100,200), (150,225,255)
   - Rules: 3 active rules
   - Memory: ~1.8KB on Arduino Uno
   */
   ```

### Error Recovery Strategies

```cpp
class FuzzyController {
private:
    AutoFuzzy fuzzy;
    bool initialized;
    float safeOutputValue; // Default safe value

public:
    FuzzyResult evaluateSafe(const FuzzyInput inputs[], int numInputs, float& output, int outputVar) {
        if (!initialized) {
            output = safeOutputValue;
            return FUZZY_ERROR_INVALID_PARAMS;
        }

        FuzzyResult result = fuzzy.evaluate(inputs, numInputs, output, outputVar);

        switch (result) {
            case FUZZY_OK:
                // Clamp to safe range
                output = constrain(output, fuzzy.getVariable(outputVar)->min,
                                 fuzzy.getVariable(outputVar)->max);
                break;

            case FUZZY_ERROR_NO_RULES_FIRED:
                Serial.println("Warning: No rules fired, using safe value");
                output = safeOutputValue;
                break;

            default:
                Serial.print("Fuzzy evaluation error: ");
                Serial.println(fuzzy.getResultString(result));
                output = safeOutputValue;
                break;
        }

        return result;
    }
};
```

This comprehensive troubleshooting guide addresses the most common issues encountered when using AutoFuzzy, with practical solutions and preventive measures to ensure reliable operation.
