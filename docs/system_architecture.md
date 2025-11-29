# AutoFuzzy System Architecture

This document describes the architectural design and implementation details of the AutoFuzzy library, providing insight into its structure, components, and design decisions.

## Overview

AutoFuzzy is a fuzzy logic inference engine designed specifically for microcontroller environments, particularly Arduino platforms. The library implements a Mamdani-style fuzzy inference system with optimizations for memory-constrained embedded systems.

## Architectural Principles

### Design Philosophy

1. **Memory Efficiency**: Conservative use of RAM with configurable limits
2. **Execution Speed**: Optimized algorithms for real-time control applications
3. **Error Resilience**: Comprehensive error handling and validation
4. **Modularity**: Clean separation of concerns with well-defined interfaces
5. **Extensibility**: Support for future enhancements without breaking changes

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                    AutoFuzzy Library                        │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │             Public Interface Layer                 │    │
│  │  ┌─────────────────────────────────────────────────┐ │    │
│  │  │ Variable Management    │ Rule Management       │ │    │
│  │  │ Membership Functions   │ Evaluation Engine     │ │    │
│  │  │ Configuration          │ Auto-Tuning           │ │    │
│  │  └─────────────────────────────────────────────────┘ │    │
│  └─────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │            Core Processing Engine                  │    │
│  │  ┌─────────────────────────────────────────────────┐ │    │
│  │  │ Fuzzification        │ Rule Evaluation         │ │    │
│  │  │ Aggregation          │ Defuzzification         │ │    │
│  │  └─────────────────────────────────────────────────┘ │    │
│  └─────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │             Data Management Layer                  │    │
│  │  ┌─────────────────────────────────────────────────┐ │    │
│  │  │ Variable Storage     │ Rule Storage           │ │    │
│  │  │ MF Parameters        │ Configuration           │ │    │
│  │  └─────────────────────────────────────────────────┘ │    │
│  └─────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐    │
│  │            Platform Abstraction                     │    │
│  │  Arduino Compatibility │ Memory Management         │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Component Architecture

### 1. Public Interface Layer

#### Variable Management
```cpp
class AutoFuzzy {
public:
    // Input/Output variable lifecycle
    int addInput(const char* name, float min, float max);
    int addOutput(const char* name, float min, float max);
    int findVariable(const char* name) const;
};
```

**Responsibilities:**
- Variable registration and validation
- Name-to-index mapping for efficient lookups
- Range validation and bounds checking
- Memory allocation management

#### Membership Function Management
```cpp
class AutoFuzzy {
public:
    // MF creation and management
    FuzzyResult addTriangularMF(int varIndex, const char* mfName, float a, float b, float c);
    FuzzyResult addTrapezoidalMF(int varIndex, const char* mfName, float a, float b, float c, float d);
    int findMF(int varIndex, const char* mfName) const;
};
```

**Responsibilities:**
- Membership function parameter validation
- Shape-specific calculations (triangular, trapezoidal)
- Parameter ordering enforcement
- MF indexing and lookup

#### Rule Management
```cpp
class AutoFuzzy {
public:
    // Rule definition and storage
    FuzzyResult addRule(const Antecedent antecedents[], int numAntecedents, FuzzyOperator op, const Consequent& consequent);
    FuzzyResult addRule(const Antecedent& antecedent, const Consequent& consequent);
    FuzzyResult addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF);
};
```

**Responsibilities:**
- Rule structure validation
- Antecedent/consequent verification
- Rule storage and indexing
- Multi-antecedent rule support

### 2. Core Processing Engine

#### Fuzzification Engine
```cpp
private:
    float calculateMembership(const MembershipFunction& mf, float value) const;
```

**Algorithm:**
```
For Triangular MF (a, b, c):
    if value ≤ a or value ≥ c: return 0
    if a < value < b: return (value - a) / (b - a)
    if b ≤ value < c: return (c - value) / (c - b)

For Trapezoidal MF (a, b, c, d):
    if value ≤ a or value ≥ d: return 0
    if b ≤ value ≤ c: return 1
    if a < value < b: return (value - a) / (b - a)
    if c < value < d: return (d - value) / (d - c)
```

#### Rule Evaluation Engine
```cpp
private:
    float calculateRuleActivation(const Rule& rule, const FuzzyInput inputs[], int numInputs) const;
    float applyOperator(FuzzyOperator op, float value1, float value2) const;
```

**Algorithm:**
```
Initialize activation = membership of first antecedent

For each subsequent antecedent:
    Calculate current_membership for antecedent
    activation = applyOperator(op, activation, current_membership)

Return final activation (0.0 to 1.0)
```

**Operators:**
- `FUZZY_AND`: `min(value1, value2)`
- `FUZZY_OR`: `max(value1, value2)`

#### Defuzzification Engine
```cpp
public:
    FuzzyResult evaluate(const FuzzyInput inputs[], int numInputs, float& resultValue, int outputVarIndex);
```

**Center of Sums Algorithm:**
```
Initialize weighted_sum = 0, weight_sum = 0

For each rule affecting target output:
    Calculate rule_activation (w_i)
    If w_i > 0:
        Get MF centroid (c_i)
        weighted_sum += w_i * c_i
        weight_sum += w_i

If weight_sum > 0:
    result = weighted_sum / weight_sum
Else:
    Return error (no rules fired)

Clamp result to output variable range
Return result
```

### 3. Data Management Layer

#### Storage Architecture
```cpp
private:
    // Static arrays for memory efficiency
    Variable vars[FUZZY_MAX_VARS];
    uint8_t varCount;

    Rule rules[FUZZY_MAX_RULES];
    uint8_t ruleCount;
```

**Memory Layout:**
```
Variable Structure:
├── char name[FUZZY_MAX_NAME_LEN]     // Variable identifier
├── bool isInput                      // Input/output flag
├── float min, max                    // Valid range
├── MembershipFunction mfs[...]       // MF array
└── uint8_t mfCount                   // Active MF count

Rule Structure:
├── uint8_t antecedentVarIndices[...] // Input variable indices
├── uint8_t antecedentMfIndices[...]  // MF indices
├── uint8_t numAntecedents           // Antecedent count
├── FuzzyOperator op                 // Combination operator
├── uint8_t consequentVarIndex       // Output variable index
└── uint8_t consequentMfIndex        // Output MF index
```

#### Configuration Management
```cpp
// Preprocessor-based configuration
#ifndef FUZZY_MAX_VARS
#define FUZZY_MAX_VARS 3
#endif

#ifndef FUZZY_MAX_MF_PER_VAR
#define FUZZY_MAX_MF_PER_VAR 3
#endif

#ifndef FUZZY_MAX_RULES
#define FUZZY_MAX_RULES 8
#endif
```

### 4. Auto-Tuning System

#### Heuristic Optimization Engine
```cpp
public:
    FuzzyResult autoTune(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex, int iterations, float mutationRate, float mutationRange);
```

**Stochastic Hill Climbing Algorithm:**
```
1. Backup current MF parameters
2. Calculate baseline fitness (MSE)
3. For each iteration:
   a. Mutate all MF parameters randomly
   b. Constrain parameters to valid ranges
   c. Calculate new fitness
   d. If improved: keep changes, update best fitness
      Else: restore from backup
4. Return best parameters found
```

**Fitness Function:**
```
MSE = (1/numSets) * Σ (target_output - actual_output)²
```

## Error Handling Architecture

### Error Classification
```cpp
enum FuzzyResult {
    FUZZY_OK = 0,
    FUZZY_ERROR_TOO_MANY_VARS,        // Configuration limits exceeded
    FUZZY_ERROR_TOO_MANY_MFS,
    FUZZY_ERROR_TOO_MANY_RULES,
    FUZZY_ERROR_VAR_NOT_FOUND,        // Lookup failures
    FUZZY_ERROR_MF_NOT_FOUND,
    FUZZY_ERROR_INVALID_PARAMS,       // Parameter validation
    FUZZY_ERROR_INVALID_RULE,
    FUZZY_ERROR_NULL_POINTER,         // Memory safety
    FUZZY_ERROR_INDEX_OUT_OF_BOUNDS,
    FUZZY_ERROR_NO_RULES_FIRED,       // Runtime evaluation
    FUZZY_ERROR_DIVIDE_BY_ZERO
};
```

### Error Propagation Strategy

1. **Configuration Errors**: Detected during setup, prevent system initialization
2. **Runtime Errors**: Handled gracefully during evaluation, provide safe defaults
3. **Memory Errors**: Prevented through static allocation and bounds checking
4. **Validation Errors**: Comprehensive parameter checking at all interfaces

## Memory Management Strategy

### Static Allocation Design

**Advantages:**
- Deterministic memory usage
- No heap fragmentation
- Predictable performance
- Arduino-compatible (limited heap)

**Memory Calculation:**
```
Total RAM = (FUZZY_MAX_VARS × Variable_Size) +
            (FUZZY_MAX_RULES × Rule_Size) +
            Runtime_Overhead

Variable_Size ≈ 25 + (FUZZY_MAX_MF_PER_VAR × 25) bytes
Rule_Size ≈ 15 + (FUZZY_MAX_ANTECEDENTS_PER_RULE × 2) bytes
```

### Dynamic Memory Usage

**Auto-Tuning Only:**
```cpp
// Temporary allocations during tuning
float** backupParams = new float*[totalParameters];
float* tempInputs = new float[numInputVars];

// Deallocated after tuning completes
delete[] backupParams;
delete[] tempInputs;
```

## Performance Characteristics

### Time Complexity

| Operation | Complexity | Typical Time (Uno) |
|-----------|------------|-------------------|
| Variable Addition | O(1) | < 10μs |
| MF Addition | O(1) | < 15μs |
| Rule Addition | O(1) | < 20μs |
| Single Evaluation | O(R × A) | < 500μs |
| Auto-Tuning (100 iter) | O(I × R × A) | < 50ms |

Where:
- R = Rules affecting output
- A = Average antecedents per rule
- I = Tuning iterations

### Memory Efficiency

**Memory Usage Examples:**

| Configuration | Variables | RAM Usage | Status |
|---------------|-----------|-----------|---------|
| Basic (3 var, 3 MF, 8 rules) | 3 in, 1 out | ~1.8KB | ✅ Arduino Uno compatible |
| Advanced (6 var, 4 MF, 20 rules) | 4 in, 2 out | ~3.2KB | ✅ Arduino Mega compatible |
| Complex (10 var, 5 MF, 50 rules) | 8 in, 2 out | ~6.8KB | ✅ ESP32 compatible |

## Platform Compatibility

### Arduino Compatibility Layer

```cpp
// Arduino-specific adaptations
#include <Arduino.h>        // Standard Arduino functions
#include <float.h>          // FLT_EPSILON for floating point comparisons
#include <math.h>           // fabs, min, max functions
#include <string.h>         // strcpy, strcmp for string operations
```

### Board-Specific Optimizations

**AVR (Uno, Nano):**
- Conservative memory limits
- Integer-based calculations where possible
- Minimal floating-point operations

**ARM (Mega, ESP32):**
- Higher memory limits available
- Enhanced floating-point precision
- Additional optimization features

## Extensibility Design

### Modular Architecture Benefits

1. **Algorithm Extensibility**
   - Additional membership function types
   - Alternative defuzzification methods
   - Custom fuzzy operators

2. **Platform Extensibility**
   - Additional microcontroller support
   - Custom memory management strategies
   - Hardware acceleration integration

3. **Feature Extensibility**
   - Advanced auto-tuning algorithms
   - Fuzzy rule learning
   - Multi-objective optimization

### Interface Stability

**Stable Interfaces:**
- Public method signatures
- Error code definitions
- Configuration macros
- Data structure layouts

**Extension Points:**
- Private implementation methods
- Internal algorithms
- Memory management strategies

## Quality Assurance

### Validation Strategy

1. **Static Analysis**
   - Bounds checking on all array accesses
   - Parameter validation at all entry points
   - Memory safety verification

2. **Runtime Testing**
   - Boundary condition testing
   - Error condition handling
   - Performance benchmarking

3. **Integration Testing**
   - Arduino board compatibility
   - Library manager compliance
   - Example verification

### Error Prevention

**Defensive Programming:**
- Null pointer checks
- Range validation
- Resource limit enforcement
- Graceful error recovery

**Fail-Safe Design:**
- Safe default values on errors
- System state preservation
- Diagnostic information provision

This architectural design provides a robust, efficient, and maintainable fuzzy logic implementation suitable for resource-constrained embedded systems while maintaining extensibility for future enhancements.
