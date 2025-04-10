# AutoFuzzy Library: Internal Explanation

This document provides insight into the internal workings, algorithms, and data structures used within the AutoFuzzy library.

## Core Fuzzy Logic Concepts Implemented

AutoFuzzy implements a Type-1 Mamdani-style fuzzy inference system with common simplifications for microcontroller environments.

*   **Linguistic Variables:** Represented by the `Variable` struct, holding name, range (min/max), input/output status, and associated Membership Functions.
*   **Membership Functions (MFs):** Represented by the `MembershipFunction` struct, storing name, type (`MfType` enum: `MF_TRIANGULAR`, `MF_TRAPEZOIDAL`), and parameters (`params` array). Currently, Triangular and Trapezoidal shapes are supported.
*   **Fuzzy Rules:** Represented by the `Rule` struct. Each rule stores indices for its antecedent variables and MFs, the combining operator (`FuzzyOperator` enum: `FUZZY_AND`, `FUZZY_OR`), and indices for its single consequent variable and MF.
*   **Inference Steps:**
    1.  **Fuzzification:** Input values are mapped to degrees of membership using `calculateMembership()`.
    2.  **Rule Evaluation:** Antecedent memberships are combined using `applyOperator()` (MIN for AND, MAX for OR) within `calculateRuleActivation()` to find each rule's strength.
    3.  **Defuzzification:** A crisp output is calculated using a weighted average (Center of Sums) approach within `evaluate()`, using MF centroids calculated by `getMfCentroid()`.

## Library Architecture & Data Structures

### Key Structs

*   **`Variable`**:
    ```cpp
    struct Variable {
        char name[FUZZY_MAX_NAME_LEN];
        bool isInput; // True if input, false if output
        float min;
        float max;
        MembershipFunction mfs[FUZZY_MAX_MF_PER_VAR]; // Array of MFs for this var
        uint8_t mfCount; // Number of active MFs in the array
        // ... helper methods
    };
    ```
*   **`MembershipFunction`**:
    ```cpp
    struct MembershipFunction {
        char name[FUZZY_MAX_NAME_LEN];
        MfType type; // MF_TRIANGULAR or MF_TRAPEZOIDAL
        float params[4]; // Stores a,b,c for Tri; a,b,c,d for Trap
    };
    ```
*   **`Rule`**:
    ```cpp
    struct Rule {
        uint8_t antecedentVarIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
        uint8_t antecedentMfIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
        uint8_t numAntecedents;
        FuzzyOperator op; // FUZZY_AND or FUZZY_OR
        uint8_t consequentVarIndex; // Single consequent variable
        uint8_t consequentMfIndex;  // Single consequent MF
    };
    ```
*   **`FuzzyInput`**: Used to pass crisp input values to `evaluate()`.
    ```cpp
    typedef struct {
        uint8_t varIndex;
        float value;
    } FuzzyInput;
    ```
*   **`Antecedent` / `Consequent`**: Helper structs for defining rules using indices.
    ```cpp
    struct AutoFuzzy::Antecedent { int varIndex; int mfIndex; };
    struct AutoFuzzy::Consequent { int varIndex; int mfIndex; };
    ```

### Main Storage

The `AutoFuzzy` class holds arrays of these core structs:

```cpp
class AutoFuzzy {
    // ... public methods ...
private:
    Variable vars[FUZZY_MAX_VARS];
    uint8_t varCount;
    Rule rules[FUZZY_MAX_RULES];
    uint8_t ruleCount;
    // ... private methods ...
};
```
The sizes of these arrays are determined by the `FUZZY_MAX_...` configuration defines.

## Inference Engine Details

### Fuzzification (`calculateMembership`)

This function takes a `MembershipFunction` and a crisp `value` and returns the degree of membership (0.0 to 1.0).

*   **Triangular (`a`, `b`, `c`):**
    *   If `value <= a` or `value >= c`, result is 0.
    *   If `a < value < b`, result is `(value - a) / (b - a)`.
    *   If `b <= value < c`, result is `(c - value) / (c - b)`.
    *   Handles cases where `b-a` or `c-b` are zero (vertical slopes) by returning 1.0 if `value` is exactly `b`.
*   **Trapezoidal (`a`, `b`, `c`, `d`):**
    *   If `value <= a` or `value >= d`, result is 0.
    *   If `b <= value <= c`, result is 1.0.
    *   If `a < value < b`, result is `(value - a) / (b - a)`.
    *   If `c < value < d`, result is `(d - value) / (d - c)`.
    *   Handles cases where `b-a` or `d-c` are zero.

### Rule Activation (`calculateRuleActivation`)

This function determines the "strength" or "firing level" of a single `Rule` based on the current `FuzzyInput` values.

1.  Initializes `activation` (e.g., to the membership of the first antecedent).
2.  Iterates through the rule's antecedents (`i = 1` to `numAntecedents - 1`).
3.  For each antecedent:
    *   Finds the corresponding `FuzzyInput` value provided.
    *   Calls `calculateMembership()` for that variable, MF, and value to get `currentMembership`.
    *   Combines the `activation` calculated so far with `currentMembership` using the rule's `op` (`FUZZY_AND` or `FUZZY_OR`) via `applyOperator()`.
    *   Updates `activation` with the result.
4.  Returns the final combined `activation` strength (0.0 to 1.0).

### Fuzzy Operators (`applyOperator`)

This helper simply implements the chosen fuzzy logic for AND/OR:

*   `FUZZY_AND`: Returns `fmin(value1, value2)`
*   `FUZZY_OR`: Returns `fmax(value1, value2)`

### Defuzzification (`evaluate`)

This function calculates the final crisp output value for a specific `outputVarIndex`.

1.  Initializes `weightedSum = 0.0` and `weightSum = 0.0`.
2.  Iterates through all defined `rules`.
3.  If a rule's `consequentVarIndex` matches the target `outputVarIndex`:
    *   Calls `calculateRuleActivation()` to get the rule's strength `w_i`.
    *   If `w_i > 0`:
        *   Gets the consequent `MembershipFunction`.
        *   Calls `getMfCentroid()` to find the representative crisp value `c_i` for that MF.
        *   Updates `weightedSum += w_i * c_i`.
        *   Updates `weightSum += w_i`.
        *   Sets a flag indicating at least one rule fired.
4.  After checking all rules:
    *   If no rules fired, returns `FUZZY_ERROR_NO_RULES_FIRED` (output value might be set to min).
    *   If `weightSum` is close to zero, returns `FUZZY_ERROR_DIVIDE_BY_ZERO` (output value might be set to min).
    *   Otherwise, calculates the final result: `resultValue = weightedSum / weightSum`.
    *   Clamps `resultValue` to the output variable's defined `min`/`max` range.
    *   Returns `FUZZY_OK`.

### MF Centroid Calculation (`getMfCentroid`)

This provides a simplified representative value for output MFs, used in defuzzification.

*   **Triangular (`a`, `b`, `c`):** Returns the peak value `b`. (Approximation)
*   **Trapezoidal (`a`, `b`, `c`, `d`):** Returns the midpoint of the plateau `(b + c) / 2`. (Approximation)

*(Note: These are simplifications. Calculating the true geometric centroid of these shapes is more complex and often unnecessary for practical control applications on microcontrollers).*

## `autoTune` Algorithm Explanation

`autoTune` implements a randomized **heuristic optimization** (specifically, a form of **stochastic hill climbing**) to adjust MF parameters (`a`, `b`, `c`, `d` values for all MFs).

*   **Goal:** Minimize the Mean Squared Error (MSE) between the fuzzy system's output for a given output variable and a set of target outputs provided in the training data.
*   **Fitness Function (`evaluateFitness`):** Calculates MSE:
    ```
    MSE = (1 / numSets) * Σ [ (trainingOutputs[i] - evaluate(trainingInputs[i]))^2 ]
    ```
    The sum is over all training sets `i`. `evaluate()` is called internally using the current MF parameters. Lower MSE indicates better fitness.
*   **Process (`autoTune` main loop):**
    1.  **Backup:** Store the current parameters of all MFs. Calculate initial `bestFitness`.
    2.  **Iterate:** Loop `iterations` times.
    3.  **Mutate (`mutateParameters`):** Randomly modify parameters of all MFs:
        *   For each parameter (`a,b,c` or `a,b,c,d`):
            *   With probability `mutationRate`, add a random value within `[-range, +range]`, where `range = (var.max - var.min) * mutationRange`.
        *   **Constrain:** Ensure mutated parameters stay within `var.min` and `var.max`.
        *   **Order:** Enforce parameter order (e.g., `a <= b <= c`). A simple fix is applied (`params[k] = params[k-1]` if out of order), rather than a full sort.
    4.  **Evaluate:** Calculate `currentFitness` using the *mutated* parameters via `evaluateFitness()`.
    5.  **Select:**
        *   If `currentFitness < bestFitness`: The mutation was beneficial. Keep the mutated parameters. Update `bestFitness = currentFitness`. Backup the new best parameters.
        *   Else: The mutation was not helpful. Restore the parameters from the last saved backup.
    6.  **Repeat:** Continue iterating.
*   **Outcome:** After `iterations`, the MFs will hold the parameters corresponding to the lowest MSE found during the process.
*   **Heuristic Nature:** It explores the parameter space randomly. It might find a good solution but doesn't guarantee finding the absolute *best* possible parameters (global minimum). It can get stuck in local minima. Performance depends heavily on the starting point, training data, and tuning parameters (`iterations`, `mutationRate`, `mutationRange`).
*   **Dynamic Memory:** Uses `new`/`delete[]` to temporarily store backup parameters during the tuning process.

## Memory Considerations

*   **Static Memory (RAM):** The primary usage comes from the fixed-size arrays `vars[]` and `rules[]`. Their size depends directly on `FUZZY_MAX_VARS`, `FUZZY_MAX_MF_PER_VAR`, and `FUZZY_MAX_RULES`. Keep these limits reasonable for your target board.
*   **Dynamic Memory (Heap):** Only `autoTune()` uses the heap via `new`/`delete[]` for backing up MF parameters and creating temporary input arrays. If `autoTune` is not used, the library uses no dynamic memory allocation.

## Error Handling (`FuzzyResult`)

Most public methods return a value from the `FuzzyResult` enum. This allows the calling code to detect and react to issues during setup or evaluation (e.g., configuration errors, evaluation warnings like `NO_RULES_FIRED`). The `getResultString()` method provides a textual description for debugging.
