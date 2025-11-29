# AutoFuzzy Integration Guide

This guide provides detailed instructions for integrating the AutoFuzzy library into existing Arduino projects and migrating from other fuzzy logic implementations.

## Prerequisites

### Hardware Requirements
- Arduino board with at least 2KB RAM (Uno, Nano, Mega recommended)
- Standard Arduino development environment

### Software Requirements
- Arduino IDE 1.8.x or Arduino CLI
- AutoFuzzy library installed

### Knowledge Requirements
- Basic Arduino programming concepts
- Understanding of fuzzy logic principles (recommended but not required)

## Integration Approaches

### Method 1: New Project Integration

For new projects incorporating fuzzy logic:

1. **Install the Library**
   ```bash
   # Using Arduino CLI
   arduino-cli lib install AutoFuzzy

   # Or manually copy to Arduino/libraries/ directory
   ```

2. **Basic Project Structure**
   ```cpp
   #include <AutoFuzzy.h>

   // Configuration (optional, adjust for your needs)
   #define FUZZY_MAX_VARS 4
   #define FUZZY_MAX_RULES 10

   #include <AutoFuzzy.h>

   AutoFuzzy fuzzy;

   void setup() {
       Serial.begin(9600);

       // Configure fuzzy system
       setupFuzzySystem();

       Serial.println("AutoFuzzy system initialized");
   }

   void loop() {
       // Read inputs
       float sensorValue = readSensor();

       // Prepare fuzzy inputs
       FuzzyInput inputs[] = {{0, sensorValue}}; // variable index 0

       // Evaluate
       float output;
       FuzzyResult result = fuzzy.evaluate(inputs, 1, output, 0); // output variable index 0

       if (result == FUZZY_OK) {
           applyOutput(output);
       }

       delay(100);
   }
   ```

### Method 2: Existing Project Migration

For adding fuzzy logic to existing projects:

1. **Identify Integration Points**
   - Determine which control variables need fuzzy logic
   - Identify existing sensor inputs and actuator outputs
   - Assess memory constraints

2. **Gradual Integration**
   ```cpp
   // Before: Direct sensor to output mapping
   int outputValue = map(sensorValue, 0, 1023, 0, 255);
   analogWrite(pin, outputValue);

   // After: Fuzzy logic integration
   #include <AutoFuzzy.h>
   AutoFuzzy fuzzy;

   // Setup fuzzy system in setup()
   // ...

   // Replace direct mapping with fuzzy evaluation
   FuzzyInput inputs[] = {{inputVarIndex, sensorValue}};
   float fuzzyOutput;
   if (fuzzy.evaluate(inputs, 1, fuzzyOutput, outputVarIndex) == FUZZY_OK) {
       int outputValue = (int)fuzzyOutput;
       analogWrite(pin, outputValue);
   }
   ```

## System Configuration

### Memory Management

Configure library limits based on your application requirements:

```cpp
// Basic configuration for simple applications
#define FUZZY_MAX_VARS 3           // Input + output variables
#define FUZZY_MAX_MF_PER_VAR 3     // Membership functions per variable
#define FUZZY_MAX_RULES 8          // Total rules
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 2
#define FUZZY_MAX_NAME_LEN 10

#include <AutoFuzzy.h>
```

### Error Handling Integration

Integrate AutoFuzzy error handling with your project's error management:

```cpp
class FuzzyController {
private:
    AutoFuzzy fuzzy;
    bool initialized;

public:
    FuzzyController() : initialized(false) {}

    bool initialize() {
        // Configure fuzzy system
        FuzzyResult result;

        // Add variables
        int tempVar = fuzzy.addInput("Temp", 0, 50);
        if (tempVar < 0) return false;

        int outputVar = fuzzy.addOutput("Output", 0, 255);
        if (outputVar < 0) return false;

        // Add membership functions
        result = fuzzy.addTriangularMF(tempVar, "Low", 0, 10, 20);
        if (result != FUZZY_OK) return false;

        result = fuzzy.addTriangularMF(tempVar, "High", 15, 25, 35);
        if (result != FUZZY_OK) return false;

        result = fuzzy.addTriangularMF(outputVar, "Low", 0, 50, 100);
        if (result != FUZZY_OK) return false;

        result = fuzzy.addTriangularMF(outputVar, "High", 80, 150, 255);
        if (result != FUZZY_OK) return false;

        // Add rules
        result = fuzzy.addRule("Temp", "Low", "Output", "Low");
        if (result != FUZZY_OK) return false;

        result = fuzzy.addRule("Temp", "High", "Output", "High");
        if (result != FUZZY_OK) return false;

        initialized = true;
        return true;
    }

    FuzzyResult evaluate(float input, float& output) {
        if (!initialized) return FUZZY_ERROR_INVALID_PARAMS;

        FuzzyInput inputs[] = {{0, input}}; // Assuming tempVar is index 0
        return fuzzy.evaluate(inputs, 1, output, 1); // Assuming outputVar is index 1
    }
};

// Usage
FuzzyController controller;

void setup() {
    Serial.begin(9600);
    if (!controller.initialize()) {
        Serial.println("Fuzzy system initialization failed");
        while(1); // Halt on critical error
    }
}

void loop() {
    float sensorValue = analogRead(A0) * (50.0 / 1023.0);
    float outputValue;

    FuzzyResult result = controller.evaluate(sensorValue, outputValue);
    if (result == FUZZY_OK) {
        analogWrite(9, (int)outputValue);
    } else {
        // Handle error - use safe default
        analogWrite(9, 127);
    }
}
```

## Performance Considerations

### Memory Optimization

1. **Minimize Variable Count**
   - Only define variables actually used
   - Combine related inputs if possible

2. **Optimize Membership Functions**
   - Use triangular functions for memory efficiency
   - Limit overlap to necessary ranges
   - Use appropriate number of functions per variable

3. **Rule Optimization**
   - Minimize rule count
   - Use multi-antecedent rules to reduce total rules
   - Ensure rule coverage without redundancy

### Execution Time Optimization

1. **Pre-calculate Indices**
   ```cpp
   // Good: Pre-calculate in setup
   int tempVar = fuzzy.findVariable("Temperature");
   int tempLowMf = fuzzy.findMF(tempVar, "Low");

   // Avoid: Look up by name in loop
   // FuzzyResult result = fuzzy.evaluate(inputs, 1, output, fuzzy.findVariable("Output"));
   ```

2. **Minimize Evaluation Frequency**
   ```cpp
   // Good: Evaluate only when needed
   static unsigned long lastEvaluation = 0;
   if (millis() - lastEvaluation > 100) { // Evaluate every 100ms
       // Perform evaluation
       lastEvaluation = millis();
   }
   ```

## Migration from Other Libraries

### From Basic Fuzzy Logic Libraries

If migrating from simpler fuzzy logic implementations:

1. **Identify Existing Components**
   - Map existing membership functions to AutoFuzzy format
   - Convert rule definitions to AutoFuzzy rule format
   - Identify input/output variable ranges

2. **Gradual Migration**
   ```cpp
   // Before: Simple fuzzy library
   // fuzzy.setInput(sensorValue);
   // int output = fuzzy.getOutput();

   // After: AutoFuzzy
   FuzzyInput inputs[] = {{inputVarIndex, sensorValue}};
   float output;
   fuzzy.evaluate(inputs, 1, output, outputVarIndex);
   ```

### From Custom Fuzzy Implementations

For custom implementations:

1. **Extract Logic Components**
   - Document existing membership function parameters
   - Record rule conditions and actions
   - Note input/output ranges and scaling

2. **Component Mapping**
   ```cpp
   // Custom triangular MF: points a=10, b=20, c=30
   fuzzy.addTriangularMF(varIndex, "MF_Name", 10.0f, 20.0f, 30.0f);

   // Custom rule: if input > 15 then output = 100
   // Convert to fuzzy: if input is High then output is High
   fuzzy.addRule("Input", "High", "Output", "High");
   ```

## Testing and Validation

### Unit Testing Approach

```cpp
void testFuzzySystem() {
    // Test boundary conditions
    FuzzyInput testInputs[] = {{tempVar, 0.0f}}; // Minimum temperature
    float output;
    FuzzyResult result = fuzzy.evaluate(testInputs, 1, output, outputVar);

    Serial.print("Min temp test: ");
    if (result == FUZZY_OK) {
        Serial.println("PASS - Output: " + String(output));
    } else {
        Serial.println("FAIL - Error: " + String(fuzzy.getResultString(result)));
    }

    // Test normal operation
    testInputs[0].value = 25.0f; // Normal temperature
    result = fuzzy.evaluate(testInputs, 1, output, outputVar);

    Serial.print("Normal temp test: ");
    if (result == FUZZY_OK) {
        Serial.println("PASS - Output: " + String(output));
    } else {
        Serial.println("FAIL");
    }
}
```

### Integration Testing

1. **Sensor Integration Testing**
   ```cpp
   void testSensorIntegration() {
       for (int testValue = 0; testValue <= 50; testValue += 5) {
           FuzzyInput inputs[] = {{tempVar, (float)testValue}};
           float output;
           FuzzyResult result = fuzzy.evaluate(inputs, 1, output, outputVar);

           Serial.print("Input: ");
           Serial.print(testValue);
           Serial.print(" -> Output: ");
           Serial.println(result == FUZZY_OK ? String(output) : "ERROR");
       }
   }
   ```

2. **Actuator Integration Testing**
   ```cpp
   void testActuatorResponse() {
       // Test that fuzzy outputs produce expected actuator responses
       float testOutputs[] = {0.0f, 50.0f, 100.0f, 150.0f, 200.0f, 255.0f};

       for (float testOutput : testOutputs) {
           analogWrite(testPin, (int)testOutput);
           delay(500); // Allow time to observe actuator response
           Serial.println("Testing output: " + String(testOutput));
       }
   }
   ```

## Best Practices

### Code Organization

1. **Separate Fuzzy Configuration**
   ```cpp
   // fuzzy_config.h
   #ifndef FUZZY_CONFIG_H
   #define FUZZY_CONFIG_H

   #include <AutoFuzzy.h>

   class FuzzyConfig {
   public:
       static bool setupFuzzySystem(AutoFuzzy& fuzzy);
   };

   #endif
   ```

2. **Error Handling Strategy**
   ```cpp
   // Define error handling policy
   #define FUZZY_ERROR_POLICY_DEFAULT  // Use default values on error
   // #define FUZZY_ERROR_POLICY_HALT     // Halt on critical errors

   FuzzyResult handleFuzzyError(FuzzyResult result, const char* operation) {
       if (result != FUZZY_OK) {
           Serial.print("Fuzzy error in ");
           Serial.print(operation);
           Serial.print(": ");
           Serial.println(fuzzy.getResultString(result));

   #ifdef FUZZY_ERROR_POLICY_HALT
           while(1); // Halt on error
   #else
           return result; // Continue with error handling
   #endif
       }
       return FUZZY_OK;
   }
   ```

### Documentation and Maintenance

1. **Document Fuzzy System Design**
   ```cpp
   /*
    * Temperature Control Fuzzy System Design
    *
    * Inputs:
    * - Temperature: 0-50°C
    *   - Cold: Triangular(0, 10, 20)
    *   - Warm: Triangular(15, 25, 35)
    *   - Hot: Triangular(30, 40, 50)
    *
    * Outputs:
    * - Fan Speed: 0-255 PWM
    *   - Low: Triangular(0, 50, 100)
    *   - High: Triangular(80, 150, 255)
    *
    * Rules:
    * 1. IF Temperature IS Cold THEN FanSpeed IS Low
    * 2. IF Temperature IS Warm THEN FanSpeed IS High
    * 3. IF Temperature IS Hot THEN FanSpeed IS High
    */
   ```

2. **Version Control Integration**
   - Document fuzzy system parameters in version control
   - Track changes to membership functions and rules
   - Maintain test cases for fuzzy system validation

## Common Integration Patterns

### Temperature Control System

```cpp
class TemperatureController {
private:
    AutoFuzzy fuzzy;
    int tempVar, humidityVar, fanVar, heaterVar;

public:
    bool initialize() {
        // Configure variables
        tempVar = fuzzy.addInput("Temp", 0, 50);
        humidityVar = fuzzy.addInput("Humidity", 0, 100);
        fanVar = fuzzy.addOutput("Fan", 0, 255);
        heaterVar = fuzzy.addOutput("Heater", 0, 255);

        // Configure membership functions and rules
        // ... implementation details

        return true;
    }

    void control(float temperature, float humidity) {
        FuzzyInput inputs[] = {
            {tempVar, temperature},
            {humidityVar, humidity}
        };

        float fanSpeed, heaterPower;

        fuzzy.evaluate(inputs, 2, fanSpeed, fanVar);
        fuzzy.evaluate(inputs, 2, heaterPower, heaterVar);

        // Apply to hardware
        analogWrite(FAN_PIN, (int)fanSpeed);
        analogWrite(HEATER_PIN, (int)heaterPower);
    }
};
```

### PID Controller Enhancement

```cpp
class EnhancedPIDController {
private:
    AutoFuzzy fuzzyTuner;
    float kp, ki, kd;
    bool fuzzyTuningEnabled;

public:
    void enableFuzzyTuning(float error, float errorRate) {
        // Use fuzzy logic to adjust PID parameters based on error characteristics
        FuzzyInput inputs[] = {{0, error}, {1, errorRate}};
        float kpAdjustment, kiAdjustment, kdAdjustment;

        fuzzyTuner.evaluate(inputs, 2, kpAdjustment, 0);
        fuzzyTuner.evaluate(inputs, 2, kiAdjustment, 1);
        fuzzyTuner.evaluate(inputs, 2, kdAdjustment, 2);

        // Apply adjustments to PID parameters
        kp += kpAdjustment;
        ki += kiAdjustment;
        kd += kdAdjustment;

        // Constrain to reasonable ranges
        kp = constrain(kp, 0.1, 10.0);
        ki = constrain(ki, 0.0, 5.0);
        kd = constrain(kd, 0.0, 2.0);
    }
};
```

This integration guide provides a comprehensive approach to incorporating AutoFuzzy into Arduino projects, with emphasis on proper error handling, performance optimization, and maintainable code structure.
