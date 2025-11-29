# AutoFuzzy Library: Technical Reference

This document provides comprehensive insight into the internal architecture, algorithms, and implementation details of the AutoFuzzy library.

## Fuzzy Logic System Overview

AutoFuzzy implements a Type-1 Mamdani-style fuzzy inference system optimized for microcontroller environments. The system follows standard fuzzy logic principles while incorporating practical simplifications for embedded systems.

### Core Components

#### Linguistic Variables
Represented by the `Variable` struct containing:
- **Name**: C-string identifier (max FUZZY_MAX_NAME_LEN characters)
- **Type**: Boolean flag indicating input (true) or output (false) classification
- **Range**: Floating-point minimum and maximum values defining valid input/output range
- **Membership Functions**: Array of associated fuzzy sets with count tracking

#### Membership Functions
Implemented through the `MembershipFunction` struct:
- **Name**: C-string identifier within the parent variable scope
- **Type**: Enumeration specifying shape (MF_TRIANGULAR, MF_TRAPEZOIDAL)
- **Parameters**: Four-element float array storing shape-defining values

#### Fuzzy Rules
Structured using the `Rule` data type:
- **Antecedent Indices**: Arrays mapping variable and membership function indices
- **Antecedent Count**: Number of conditions in the rule premise
- **Logical Operator**: Fuzzy combination method (FUZZY_AND, FUZZY_OR)
- **Consequent Indices**: Single output variable and membership function mapping

### Inference Process Architecture

The fuzzy inference engine operates through four distinct phases:

#### 1. Fuzzification Stage
Converts crisp input values to fuzzy membership degrees using `calculateMembership()`:

**Triangular Membership Function Algorithm:**
```
μ(x) = 0,                                        if x ≤ a or x ≥ c
μ(x) = (x - a) / (b - a),                       if a < x < b
μ(x) = (c - x) / (c - b),                       if b ≤ x < c
```
Where parameters represent (a, b, c) for left foot, peak, and right foot respectively.

**Trapezoidal Membership Function Algorithm:**
```
μ(x) = 0,                                        if x ≤ a or x ≥ d
μ(x) = (x - a) / (b - a),                       if a < x < b
μ(x) = 1,                                        if b ≤ x ≤ c
μ(x) = (d - x) / (d - c),                       if c < x < d
```
Where parameters represent (a, b, c, d) for left foot, left shoulder, right shoulder, and right foot.

#### 2. Rule Evaluation Stage
Determines rule activation strength through `calculateRuleActivation()`:

**Multi-Antecedent Rule Processing:**
```
activation = membership(first_antecedent)

for each subsequent antecedent:
    current_membership = calculateMembership(antecedent)
    activation = applyOperator(operator, activation, current_membership)

return activation ∈ [0,1]
```

#### 3. Aggregation Stage
Combines activated rules implicitly through weighted accumulation in the defuzzification process.

#### 4. Defuzzification Stage
Converts fuzzy output sets to crisp values using Center of Sums method in `evaluate()`:

**Center of Sums Algorithm:**
```
weighted_sum = 0
weight_sum = 0

for each rule targeting output_variable:
    rule_strength = calculateRuleActivation(rule, inputs)
    if rule_strength > 0:
        centroid_value = getMfCentroid(rule.consequent_mf)
        weighted_sum += rule_strength × centroid_value
        weight_sum += rule_strength

if weight_sum > 0:
    crisp_output = weighted_sum / weight_sum
    clamp crisp_output to [output_min, output_max]
    return FUZZY_OK
else:
    return FUZZY_ERROR_NO_RULES_FIRED
```

## System Architecture

### Data Structure Design

#### Variable Structure
```cpp
struct Variable {
    char name[FUZZY_MAX_NAME_LEN];                    // Variable identifier
    bool isInput;                                     // Input/output classification
    float min, max;                                   // Valid range bounds
    MembershipFunction mfs[FUZZY_MAX_MF_PER_VAR];     // Associated fuzzy sets
    uint8_t mfCount;                                   // Active membership function count
};
```
**Memory Footprint**: ~25 + (FUZZY_MAX_MF_PER_VAR × 25) bytes

#### Membership Function Structure
```cpp
struct MembershipFunction {
    char name[FUZZY_MAX_NAME_LEN];    // Function identifier within variable
    MfType type;                      // Shape enumeration (triangular/trapezoidal)
    float params[4];                  // Shape parameters (a,b,c,d)
};
```
**Memory Footprint**: ~25 bytes per function

#### Rule Structure
```cpp
struct Rule {
    uint8_t antecedentVarIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
    uint8_t antecedentMfIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
    uint8_t numAntecedents;           // Active antecedent count
    FuzzyOperator op;                 // Combination operator
    uint8_t consequentVarIndex;       // Output variable mapping
    uint8_t consequentMfIndex;        // Output membership function mapping
};
```
**Memory Footprint**: ~15 + (FUZZY_MAX_ANTECEDENTS_PER_RULE × 2) bytes

#### Input/Output Structures
```cpp
typedef struct {
    uint8_t varIndex;    // Target variable identifier
    float value;         // Crisp input value
} FuzzyInput;

struct Antecedent {
    int varIndex;        // Input variable index
    int mfIndex;         // Associated membership function index
};

struct Consequent {
    int varIndex;        // Output variable index
    int mfIndex;         // Associated membership function index
};
```

### Memory Management Architecture

#### Static Memory Model
The library employs static memory allocation for deterministic performance:

```cpp
class AutoFuzzy {
private:
    Variable vars[FUZZY_MAX_VARS];           // Variable storage array
    uint8_t varCount;                         // Active variable count
    Rule rules[FUZZY_MAX_RULES];             // Rule storage array
    uint8_t ruleCount;                        // Active rule count
};
```

#### Memory Configuration System
Preprocessor-based configuration enables tailoring to target platforms:

```cpp
#ifndef FUZZY_MAX_VARS
#define FUZZY_MAX_VARS 3              // Arduino Uno optimized
#endif

#ifndef FUZZY_MAX_MF_PER_VAR
#define FUZZY_MAX_MF_PER_VAR 3        // Conservative MF count
#endif

#ifndef FUZZY_MAX_RULES
#define FUZZY_MAX_RULES 8             // Limited rule capacity
#endif

#ifndef FUZZY_MAX_ANTECEDENTS_PER_RULE
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 2    // Multi-condition support
#endif

#ifndef FUZZY_MAX_NAME_LEN
#define FUZZY_MAX_NAME_LEN 10         // Compact naming
#endif
```

**Default Memory Profile (Arduino Uno):**
- Total Static RAM: ~1.8KB
- Variable Array: 3 × 175 bytes = 525 bytes
- Rule Array: 8 × 19 bytes = 152 bytes
- Overhead: ~250 bytes

## Inference Engine Implementation

### Fuzzification Process

The `calculateMembership()` function implements degree of membership calculation:

**Triangular Membership Function:**
```cpp
float calculateMembership(const MembershipFunction& mf, float value) {
    float a = mf.params[0], b = mf.params[1], c = mf.params[2];

    if (value <= a || value >= c) return 0.0f;
    if (value <= b) return (value - a) / (b - a);  // Left slope
    return (c - value) / (c - b);                   // Right slope
}
```

**Trapezoidal Membership Function:**
```cpp
float calculateMembership(const MembershipFunction& mf, float value) {
    float a = mf.params[0], b = mf.params[1], c = mf.params[2], d = mf.params[3];

    if (value <= a || value >= d) return 0.0f;
    if (value >= b && value <= c) return 1.0f;      // Plateau region
    if (value < b) return (value - a) / (b - a);   // Left slope
    return (d - value) / (d - c);                   // Right slope
}
```

### Rule Evaluation Algorithm

The `calculateRuleActivation()` method processes rule firing strength:

```cpp
float calculateRuleActivation(const Rule& rule, const FuzzyInput inputs[], int numInputs) {
    // Initialize with first antecedent
    float activation = 0.0f;
    bool foundFirst = false;

    for (uint8_t i = 0; i < rule.numAntecedents; i++) {
        // Find corresponding input value
        float inputValue = 0.0f;
        bool inputFound = false;

        for (int j = 0; j < numInputs; j++) {
            if (inputs[j].varIndex == rule.antecedentVarIndices[i]) {
                inputValue = inputs[j].value;
                inputFound = true;
                break;
            }
        }

        if (!inputFound) return 0.0f; // Required input missing

        // Calculate membership for this antecedent
        const MembershipFunction* mf = getMF(rule.antecedentVarIndices[i],
                                           rule.antecedentMfIndices[i]);
        float membership = calculateMembership(*mf, inputValue);

        // Combine with running activation
        if (!foundFirst) {
            activation = membership;
            foundFirst = true;
        } else {
            activation = applyOperator(rule.op, activation, membership);
        }
    }

    return activation;
}
```

### Fuzzy Logic Operators

The `applyOperator()` function implements standard fuzzy logic operations:

```cpp
float applyOperator(FuzzyOperator op, float value1, float value2) {
    switch (op) {
        case FUZZY_AND:
            return fmin(value1, value2);  // Minimum (T-norm)
        case FUZZY_OR:
            return fmax(value1, value2);  // Maximum (S-norm)
        default:
            return 0.0f;
    }
}
```

### Defuzzification Process

The `evaluate()` method implements Center of Sums defuzzification:

```cpp
FuzzyResult evaluate(const FuzzyInput inputs[], int numInputs,
                    float& resultValue, int outputVarIndex) {

    float weightedSum = 0.0f;
    float weightSum = 0.0f;
    bool rulesFired = false;

    // Process each rule
    for (uint8_t ruleIdx = 0; ruleIdx < ruleCount; ruleIdx++) {
        const Rule& rule = rules[ruleIdx];

        // Check if rule affects target output
        if (rule.consequentVarIndex != outputVarIndex) continue;

        // Calculate rule activation
        float activation = calculateRuleActivation(rule, inputs, numInputs);

        if (activation > 0.0f) {
            rulesFired = true;

            // Get consequent MF centroid
            const MembershipFunction* mf = getMF(rule.consequentVarIndex,
                                               rule.consequentMfIndex);
            float centroid = getMfCentroid(*mf);

            // Accumulate weighted sum
            weightedSum += activation * centroid;
            weightSum += activation;
        }
    }

    if (!rulesFired) {
        resultValue = getVariable(outputVarIndex)->min;
        return FUZZY_ERROR_NO_RULES_FIRED;
    }

    if (weightSum < FLT_EPSILON) {
        resultValue = getVariable(outputVarIndex)->min;
        return FUZZY_ERROR_DIVIDE_BY_ZERO;
    }

    // Calculate final crisp output
    resultValue = weightedSum / weightSum;

    // Clamp to valid range
    const Variable* outputVar = getVariable(outputVarIndex);
    resultValue = constrain(resultValue, outputVar->min, outputVar->max);

    return FUZZY_OK;
}
```

### Centroid Calculation

The `getMfCentroid()` function provides representative values for defuzzification:

```cpp
float getMfCentroid(const MembershipFunction& mf) {
    switch (mf.type) {
        case MF_TRIANGULAR:
            return mf.params[1];  // Peak value (b parameter)

        case MF_TRAPEZOIDAL:
            // Midpoint of plateau region
            return (mf.params[1] + mf.params[2]) / 2.0f;

        default:
            return 0.0f;
    }
}
```

## Auto-Tuning System

### Heuristic Optimization Algorithm

The `autoTune()` method implements stochastic hill climbing for parameter optimization:

#### Algorithm Overview
```cpp
FuzzyResult autoTune(float** trainingInputs, float* trainingOutputs,
                    int numSets, int outputVarIndex, int iterations,
                    float mutationRate, float mutationRange) {

    // Backup original parameters
    backupParameters();

    // Calculate baseline fitness
    float bestFitness = evaluateFitness(trainingInputs, trainingOutputs,
                                      numSets, outputVarIndex);

    for (int iter = 0; iter < iterations; iter++) {
        // Mutate parameters
        mutateParameters(mutationRate, mutationRange);

        // Evaluate new fitness
        float currentFitness = evaluateFitness(trainingInputs, trainingOutputs,
                                             numSets, outputVarIndex);

        if (currentFitness < bestFitness) {
            // Improvement found - keep changes
            bestFitness = currentFitness;
            backupParameters();  // Save improved parameters
        } else {
            // No improvement - restore previous best
            restoreParameters();
        }
    }

    return FUZZY_OK;
}
```

#### Fitness Evaluation Function
```cpp
float evaluateFitness(float** inputs, float* targets, int numSets, int outputVar) {
    float totalError = 0.0f;

    for (int i = 0; i < numSets; i++) {
        // Prepare input structure
        FuzzyInput fuzzyInputs[FUZZY_MAX_VARS];
        int inputCount = 0;

        // Map training inputs to fuzzy system inputs
        for (int varIdx = 0; varIdx < varCount; varIdx++) {
            if (vars[varIdx].isInput) {
                fuzzyInputs[inputCount].varIndex = varIdx;
                fuzzyInputs[inputCount].value = inputs[i][inputCount];
                inputCount++;
            }
        }

        // Evaluate fuzzy system
        float output;
        evaluate(fuzzyInputs, inputCount, output, outputVar);

        // Calculate squared error
        float error = targets[i] - output;
        totalError += error * error;
    }

    return totalError / numSets;  // Mean Squared Error
}
```

#### Parameter Mutation Strategy
```cpp
void mutateParameters(float mutationRate, float mutationRange) {
    for (uint8_t varIdx = 0; varIdx < varCount; varIdx++) {
        const Variable& var = vars[varIdx];

        for (uint8_t mfIdx = 0; mfIdx < var.mfCount; mfIdx++) {
            MembershipFunction& mf = vars[varIdx].mfs[mfIdx];

            // Mutate each parameter
            for (uint8_t paramIdx = 0; paramIdx < 4; paramIdx++) {
                if (random(0, 100) < (mutationRate * 100)) {
                    // Apply random mutation within range
                    float range = (var.max - var.min) * mutationRange;
                    float mutation = random(-range * 100, range * 100) / 100.0f;
                    mf.params[paramIdx] += mutation;

                    // Constrain to variable bounds
                    mf.params[paramIdx] = constrain(mf.params[paramIdx], var.min, var.max);
                }
            }

            // Ensure parameter ordering
            checkMfParamOrder(mf);
        }
    }
}
```

#### Parameter Order Validation
```cpp
bool checkMfParamOrder(MembershipFunction& mf) {
    switch (mf.type) {
        case MF_TRIANGULAR:
            // Ensure a ≤ b ≤ c
            if (mf.params[0] > mf.params[1]) mf.params[0] = mf.params[1];
            if (mf.params[1] > mf.params[2]) mf.params[2] = mf.params[1];
            return (mf.params[0] <= mf.params[1] && mf.params[1] <= mf.params[2]);

        case MF_TRAPEZOIDAL:
            // Ensure a ≤ b ≤ c ≤ d
            if (mf.params[0] > mf.params[1]) mf.params[0] = mf.params[1];
            if (mf.params[1] > mf.params[2]) mf.params[1] = mf.params[2];
            if (mf.params[2] > mf.params[3]) mf.params[3] = mf.params[2];
            return (mf.params[0] <= mf.params[1] && mf.params[1] <= mf.params[2] &&
                    mf.params[2] <= mf.params[3]);

        default:
            return false;
    }
}
```

### Optimization Characteristics

- **Heuristic Nature**: Does not guarantee global optimum
- **Local Optima Risk**: May converge to suboptimal solutions
- **Training Data Dependency**: Performance heavily influenced by data quality
- **Computational Cost**: O(iterations × training_sets × rule_complexity)
- **Memory Overhead**: Temporary dynamic allocation during optimization

## Memory Management Strategy

### Static Memory Architecture

**Fixed-Size Arrays:**
```cpp
Variable vars[FUZZY_MAX_VARS];        // Variable storage
Rule rules[FUZZY_MAX_RULES];          // Rule storage
```

**Memory Calculation:**
```
Total_Static_RAM = (FUZZY_MAX_VARS × Variable_Size) +
                   (FUZZY_MAX_RULES × Rule_Size) +
                   Overhead_Constants

Variable_Size ≈ 25 + (FUZZY_MAX_MF_PER_VAR × 25) bytes
Rule_Size ≈ 15 + (FUZZY_MAX_ANTECEDENTS_PER_RULE × 2) bytes
```

**Default Configuration Memory Profile:**
- Variables: 3 × 175 = 525 bytes
- Rules: 8 × 19 = 152 bytes
- Total Static: ~800 bytes
- Runtime Overhead: ~1,000 bytes
- Total RAM: ~1.8KB (Arduino Uno compatible)

### Dynamic Memory Usage

**Auto-Tuning Only:**
```cpp
// Temporary allocations during optimization
MembershipFunction** backupParams = new MembershipFunction*[totalMFs];
float* tempInputs = new float[numInputVars];

// Deallocated upon completion
delete[] backupParams;
delete[] tempInputs;
```

### Memory Safety Features

- **Bounds Checking**: All array accesses validated
- **Resource Limits**: Configuration prevents overflow
- **Leak Prevention**: Dynamic memory properly managed
- **Stack Protection**: Large operations avoided in interrupts

## Error Handling Framework

### Result Code Architecture

```cpp
enum FuzzyResult {
    FUZZY_OK = 0,

    // Configuration errors
    FUZZY_ERROR_TOO_MANY_VARS,
    FUZZY_ERROR_TOO_MANY_MFS,
    FUZZY_ERROR_TOO_MANY_RULES,
    FUZZY_ERROR_TOO_MANY_ANTECEDENTS,

    // Lookup errors
    FUZZY_ERROR_VAR_NOT_FOUND,
    FUZZY_ERROR_MF_NOT_FOUND,

    // Parameter errors
    FUZZY_ERROR_INVALID_PARAMS,
    FUZZY_ERROR_INVALID_RULE,

    // Runtime errors
    FUZZY_ERROR_NULL_POINTER,
    FUZZY_ERROR_INDEX_OUT_OF_BOUNDS,
    FUZZY_ERROR_NO_RULES_FIRED,
    FUZZY_ERROR_DIVIDE_BY_ZERO
};
```

### Error Propagation Strategy

1. **Configuration Phase**: Validation prevents invalid system states
2. **Runtime Phase**: Graceful degradation with safe defaults
3. **Diagnostic Support**: Descriptive error messages for debugging

### Error Recovery Mechanisms

- **Safe Defaults**: System continues with reasonable fallback values
- **State Preservation**: Valid components remain operational
- **Diagnostic Output**: Comprehensive error reporting for development

This technical reference provides complete insight into AutoFuzzy's internal implementation, enabling advanced users to understand, optimize, and extend the fuzzy logic system for their specific applications.
