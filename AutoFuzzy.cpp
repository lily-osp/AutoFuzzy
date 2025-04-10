#include "AutoFuzzy.h"
#include <float.h> // For FLT_EPSILON, FLT_MAX
#include <math.h> // For fabs
#include <string.h> // For strncpy, strcmp

// Helper macro for safe string copy
#define SAFE_STRNCPY(dest, src, n)   \
    do {                             \
        strncpy(dest, src, (n) - 1); \
        dest[(n) - 1] = '\0';        \
    } while (0)

// --- Constructor ---
AutoFuzzy::AutoFuzzy()
    : varCount(0)
    , ruleCount(0)
{
    // Optional: Initialize random seed if needed for autoTune consistency
    // randomSeed(analogRead(0)); // Uncomment if needed and using Arduino random
}

// --- Variable Management ---

int AutoFuzzy::addInput(const char* name, float min, float max)
{
    if (!name)
        return -1; // Null pointer check
    if (varCount >= FUZZY_MAX_VARS)
        return -1; // Error: Too many vars

    // Check for duplicate name
    if (findVariable(name) != -1)
        return -1; // Error: Duplicate name

    Variable& var = vars[varCount];
    SAFE_STRNCPY(var.name, name, FUZZY_MAX_NAME_LEN);
    var.isInput = true;
    var.min = min;
    var.max = max;
    var.mfCount = 0;
    return varCount++; // Return index of the new variable
}

int AutoFuzzy::addOutput(const char* name, float min, float max)
{
    if (!name)
        return -1; // Null pointer check
    if (varCount >= FUZZY_MAX_VARS)
        return -1; // Error: Too many vars

    // Check for duplicate name
    if (findVariable(name) != -1)
        return -1; // Error: Duplicate name

    Variable& var = vars[varCount];
    SAFE_STRNCPY(var.name, name, FUZZY_MAX_NAME_LEN);
    var.isInput = false; // Output variable
    var.min = min;
    var.max = max;
    var.mfCount = 0;
    return varCount++; // Return index of the new variable
}

int AutoFuzzy::findVariable(const char* name) const
{
    if (!name)
        return -1;
    for (int i = 0; i < varCount; ++i) {
        if (strcmp(vars[i].name, name) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// Internal helper to get variable pointer safely
AutoFuzzy::Variable* AutoFuzzy::getVariable(int varIndex)
{
    if (varIndex < 0 || varIndex >= varCount)
        return nullptr;
    return &vars[varIndex];
}
const AutoFuzzy::Variable* AutoFuzzy::getVariable(int varIndex) const
{
    if (varIndex < 0 || varIndex >= varCount)
        return nullptr;
    return &vars[varIndex];
}

int AutoFuzzy::findInputVarIndex(const char* name) const
{
    int idx = findVariable(name);
    if (idx != -1 && vars[idx].isInput) {
        return idx;
    }
    return -1;
}

int AutoFuzzy::findOutputVarIndex(const char* name) const
{
    int idx = findVariable(name);
    if (idx != -1 && !vars[idx].isInput) {
        return idx;
    }
    return -1;
}

// --- Membership Function Management ---

// Helper to check MF parameter validity
bool AutoFuzzy::checkMfParamOrder(const MembershipFunction& mf) const
{
    switch (mf.type) {
    case MF_TRIANGULAR:
        // a <= b <= c
        return (mf.params[0] <= mf.params[1] && mf.params[1] <= mf.params[2]);
    case MF_TRAPEZOIDAL:
        // a <= b <= c <= d
        return (mf.params[0] <= mf.params[1] && mf.params[1] <= mf.params[2] && mf.params[2] <= mf.params[3]);
    default:
        return false; // Unknown type
    }
}

FuzzyResult AutoFuzzy::addTriangularMF(int varIndex, const char* mfName, float a, float b, float c)
{
    if (!mfName)
        return FUZZY_ERROR_NULL_POINTER;
    Variable* var = getVariable(varIndex);
    if (!var)
        return FUZZY_ERROR_VAR_NOT_FOUND;
    if (var->mfCount >= FUZZY_MAX_MF_PER_VAR)
        return FUZZY_ERROR_TOO_MANY_MFS;
    if (findMF(varIndex, mfName) != -1)
        return FUZZY_ERROR_INVALID_PARAMS; // Duplicate MF name for this var

    MembershipFunction& mf = var->mfs[var->mfCount];
    SAFE_STRNCPY(mf.name, mfName, FUZZY_MAX_NAME_LEN);
    mf.type = MF_TRIANGULAR;
    mf.params[0] = a;
    mf.params[1] = b;
    mf.params[2] = c;

    if (!checkMfParamOrder(mf))
        return FUZZY_ERROR_INVALID_PARAMS; // a <= b <= c check

    var->mfCount++;
    return FUZZY_OK;
}

// Overload using variable name
FuzzyResult AutoFuzzy::addTriangularMF(const char* varName, const char* mfName, float a, float b, float c)
{
    int varIndex = findVariable(varName);
    return addTriangularMF(varIndex, mfName, a, b, c);
}

FuzzyResult AutoFuzzy::addTrapezoidalMF(int varIndex, const char* mfName, float a, float b, float c, float d)
{
    if (!mfName)
        return FUZZY_ERROR_NULL_POINTER;
    Variable* var = getVariable(varIndex);
    if (!var)
        return FUZZY_ERROR_VAR_NOT_FOUND;
    if (var->mfCount >= FUZZY_MAX_MF_PER_VAR)
        return FUZZY_ERROR_TOO_MANY_MFS;
    if (findMF(varIndex, mfName) != -1)
        return FUZZY_ERROR_INVALID_PARAMS; // Duplicate MF name

    MembershipFunction& mf = var->mfs[var->mfCount];
    SAFE_STRNCPY(mf.name, mfName, FUZZY_MAX_NAME_LEN);
    mf.type = MF_TRAPEZOIDAL;
    mf.params[0] = a;
    mf.params[1] = b;
    mf.params[2] = c;
    mf.params[3] = d;

    if (!checkMfParamOrder(mf))
        return FUZZY_ERROR_INVALID_PARAMS; // a <= b <= c <= d check

    var->mfCount++;
    return FUZZY_OK;
}

// Overload using variable name
FuzzyResult AutoFuzzy::addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d)
{
    int varIndex = findVariable(varName);
    return addTrapezoidalMF(varIndex, mfName, a, b, c, d);
}

int AutoFuzzy::findMF(int varIndex, const char* mfName) const
{
    if (!mfName)
        return -1;
    const Variable* var = getVariable(varIndex);
    if (!var)
        return -1;

    for (int i = 0; i < var->mfCount; ++i) {
        if (strcmp(var->mfs[i].name, mfName) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

AutoFuzzy::MembershipFunction* AutoFuzzy::getMF(int varIndex, int mfIndex)
{
    Variable* var = getVariable(varIndex);
    if (!var || mfIndex < 0 || mfIndex >= var->mfCount)
        return nullptr;
    return &var->mfs[mfIndex];
}
const AutoFuzzy::MembershipFunction* AutoFuzzy::getMF(int varIndex, int mfIndex) const
{
    const Variable* var = getVariable(varIndex);
    if (!var || mfIndex < 0 || mfIndex >= var->mfCount)
        return nullptr;
    return &var->mfs[mfIndex];
}

// --- Rule Management ---

FuzzyResult AutoFuzzy::addRule(const Antecedent antecedents[], int numAntecedents, FuzzyOperator op, const Consequent& consequent)
{
    if (ruleCount >= FUZZY_MAX_RULES)
        return FUZZY_ERROR_TOO_MANY_RULES;
    if (numAntecedents <= 0 || numAntecedents > FUZZY_MAX_ANTECEDENTS_PER_RULE)
        return FUZZY_ERROR_TOO_MANY_ANTECEDENTS;
    if (!antecedents)
        return FUZZY_ERROR_NULL_POINTER;

    Rule& rule = rules[ruleCount];
    rule.numAntecedents = numAntecedents;
    rule.op = op;

    // Verify and store antecedents
    for (int i = 0; i < numAntecedents; ++i) {
        const Variable* ifVar = getVariable(antecedents[i].varIndex);
        if (!ifVar || !ifVar->isInput)
            return FUZZY_ERROR_INVALID_RULE; // Antecedent must be an input variable
        const MembershipFunction* ifMF = getMF(antecedents[i].varIndex, antecedents[i].mfIndex);
        if (!ifMF)
            return FUZZY_ERROR_MF_NOT_FOUND; // MF must exist

        rule.antecedentVarIndices[i] = antecedents[i].varIndex;
        rule.antecedentMfIndices[i] = antecedents[i].mfIndex;
    }

    // Verify and store consequent
    const Variable* thenVar = getVariable(consequent.varIndex);
    if (!thenVar || !thenVar->isOutput())
        return FUZZY_ERROR_INVALID_RULE; // Consequent must be an output variable
    const MembershipFunction* thenMF = getMF(consequent.varIndex, consequent.mfIndex);
    if (!thenMF)
        return FUZZY_ERROR_MF_NOT_FOUND; // MF must exist

    rule.consequentVarIndex = consequent.varIndex;
    rule.consequentMfIndex = consequent.mfIndex;

    ruleCount++;
    return FUZZY_OK;
}

// Helper for simple SISO rules
FuzzyResult AutoFuzzy::addRule(const Antecedent& antecedent, const Consequent& consequent)
{
    return addRule(&antecedent, 1, FUZZY_AND, consequent); // Default to AND for single antecedent
}

// Helper using names (less efficient)
FuzzyResult AutoFuzzy::addRule(const char* ifVarName, const char* ifMfName, const char* thenVarName, const char* thenMfName)
{
    if (!ifVarName || !ifMfName || !thenVarName || !thenMfName)
        return FUZZY_ERROR_NULL_POINTER;

    int ifVarIdx = findInputVarIndex(ifVarName); // Ensures it's an input var
    if (ifVarIdx == -1)
        return FUZZY_ERROR_VAR_NOT_FOUND;
    int ifMfIdx = findMF(ifVarIdx, ifMfName);
    if (ifMfIdx == -1)
        return FUZZY_ERROR_MF_NOT_FOUND;

    int thenVarIdx = findOutputVarIndex(thenVarName); // Ensures it's an output var
    if (thenVarIdx == -1)
        return FUZZY_ERROR_VAR_NOT_FOUND;
    int thenMfIdx = findMF(thenVarIdx, thenMfName);
    if (thenMfIdx == -1)
        return FUZZY_ERROR_MF_NOT_FOUND;

    Antecedent ant = { ifVarIdx, ifMfIdx };
    Consequent con = { thenVarIdx, thenMfIdx };

    return addRule(ant, con);
}

// --- Fuzzy Inference Core ---

float AutoFuzzy::calculateMembership(const MembershipFunction& mf, float value) const
{
    // Clamp value to variable bounds (optional but often helpful)
    // Optional: Add parent Variable reference to MF if needed for bounds. For now, assume value is reasonable.

    switch (mf.type) {
    case MF_TRIANGULAR: {
        float a = mf.params[0], b = mf.params[1], c = mf.params[2];
        if (value <= a || value >= c)
            return 0.0f;
        if (value == b)
            return 1.0f; // Handle peak directly
        if (value < b) {
            float denom = b - a;
            return (fabs(denom) < FLT_EPSILON) ? 1.0f : (value - a) / denom; // Avoid division by zero
        } else { // value > b
            float denom = c - b;
            return (fabs(denom) < FLT_EPSILON) ? 1.0f : (c - value) / denom; // Avoid division by zero
        }
    }
    case MF_TRAPEZOIDAL: {
        float a = mf.params[0], b = mf.params[1], c = mf.params[2], d = mf.params[3];
        if (value <= a || value >= d)
            return 0.0f;
        if (value >= b && value <= c)
            return 1.0f;
        if (value < b) {
            float denom = b - a;
            return (fabs(denom) < FLT_EPSILON) ? 1.0f : (value - a) / denom; // Avoid division by zero
        } else { // value > c
            float denom = d - c;
            return (fabs(denom) < FLT_EPSILON) ? 1.0f : (d - value) / denom; // Avoid division by zero
        }
    }
    default:
        return 0.0f; // Unknown type
    }
}

float AutoFuzzy::applyOperator(FuzzyOperator op, float value1, float value2) const
{
    switch (op) {
    case FUZZY_AND:
        return fmin(value1, value2); // MIN operator for AND
    case FUZZY_OR:
        return fmax(value1, value2); // MAX operator for OR
    // Add other operators like product if needed
    default:
        return fmin(value1, value2); // Default to AND
    }
}

float AutoFuzzy::calculateRuleActivation(const Rule& rule, const FuzzyInput inputs[], int numInputs) const
{
    if (rule.numAntecedents == 0)
        return 0.0f; // No antecedents, rule doesn't fire

    float activation = -1.0f; // Sentinel value, assumes memberships are >= 0

    for (int i = 0; i < rule.numAntecedents; ++i) {
        int varIdx = rule.antecedentVarIndices[i];
        int mfIdx = rule.antecedentMfIndices[i];

        // Find the input value for this antecedent variable
        float inputValue = NAN; // Not a Number
        for (int j = 0; j < numInputs; ++j) {
            if (inputs[j].varIndex == varIdx) {
                inputValue = inputs[j].value;
                break;
            }
        }

        if (isnan(inputValue))
            return 0.0f; // Input value for this antecedent not provided

        const MembershipFunction* mf = getMF(varIdx, mfIdx);
        if (!mf)
            return 0.0f; // Should not happen if rule added correctly

        float membership = calculateMembership(*mf, inputValue);

        if (i == 0) {
            activation = membership; // First antecedent
        } else {
            // Combine with previous activation using the rule's operator
            activation = applyOperator(rule.op, activation, membership);
        }

        // Optimization: If activation is already zero for AND rule, no need to check further
        if (rule.op == FUZZY_AND && activation < FLT_EPSILON) {
            return 0.0f;
        }
    }

    return (activation < 0.0f) ? 0.0f : activation; // Handle case where loop didn't run or initial value issue
}

// Gets the representative value (centroid) for a membership function
// Used in simplified Center of Gravity/Sum defuzzification
float AutoFuzzy::getMfCentroid(const MembershipFunction& mf) const
{
    switch (mf.type) {
    case MF_TRIANGULAR:
        // Centroid of a triangle is often approximated by its peak for simplicity here.
        // A more accurate centroid calculation is more complex.
        // return (mf.params[0] + mf.params[1] + mf.params[2]) / 3.0f; // Actual centroid
        return mf.params[1]; // Peak (simpler, common in Sugeno-like outputs)
    case MF_TRAPEZOIDAL:
        // Centroid of the flat top section for simplicity
        return (mf.params[1] + mf.params[2]) / 2.0f;
        // More accurate centroid requires calculating area moments
    default:
        return NAN; // Unknown type
    }
}

FuzzyResult AutoFuzzy::evaluate(const FuzzyInput inputs[], int numInputs, float& resultValue, int outputVarIndex)
{
    if (!inputs || numInputs <= 0)
        return FUZZY_ERROR_NULL_POINTER;

    const Variable* outputVar = getVariable(outputVarIndex);
    if (!outputVar || !outputVar->isOutput()) {
        return FUZZY_ERROR_VAR_NOT_FOUND; // Must specify a valid output variable
    }

    float weightedSum = 0.0f;
    float weightSum = 0.0f;
    bool ruleFired = false;

    for (int i = 0; i < ruleCount; ++i) {
        // Only consider rules that affect the desired output variable
        if (rules[i].consequentVarIndex != outputVarIndex) {
            continue;
        }

        float activation = calculateRuleActivation(rules[i], inputs, numInputs);

        if (activation > FLT_EPSILON) { // Rule has fired with non-zero strength
            ruleFired = true;
            const MembershipFunction* consequentMF = getMF(rules[i].consequentVarIndex, rules[i].consequentMfIndex);
            if (!consequentMF)
                return FUZZY_ERROR_MF_NOT_FOUND; // Should not happen

            float mfCenter = getMfCentroid(*consequentMF);
            if (isnan(mfCenter))
                return FUZZY_ERROR_INVALID_PARAMS; // Error getting centroid

            weightedSum += mfCenter * activation;
            weightSum += activation;
        }
    }

    if (!ruleFired) {
        resultValue = outputVar->min; // Or some default value (e.g., midpoint, 0)
        return FUZZY_ERROR_NO_RULES_FIRED;
    }

    if (fabs(weightSum) < FLT_EPSILON) {
        resultValue = outputVar->min; // Avoid division by zero, return default
        return FUZZY_ERROR_DIVIDE_BY_ZERO;
    }

    resultValue = weightedSum / weightSum;

    // Clamp result to output variable's defined range
    resultValue = constrain(resultValue, outputVar->min, outputVar->max);

    return FUZZY_OK;
}

// --- Automated Tuning (Heuristic) ---

// Backup/Restore helpers for tuning
void AutoFuzzy::backupParameters(MembershipFunction& dest, const MembershipFunction& src)
{
    memcpy(&dest, &src, sizeof(MembershipFunction));
}
void AutoFuzzy::restoreParameters(MembershipFunction& dest, const MembershipFunction& src)
{
    memcpy(&dest, &src, sizeof(MembershipFunction));
}

// Simple fitness: Mean Squared Error
float AutoFuzzy::evaluateFitness(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex)
{
    if (!trainingInputs || !trainingOutputs || numSets <= 0)
        return FLT_MAX; // Invalid data

    float totalSquaredError = 0.0f;
    int numInputVars = 0;
    // Count number of actual input variables for sizing FuzzyInput array
    for (int i = 0; i < varCount; ++i) {
        if (vars[i].isInput)
            numInputVars++;
    }
    if (numInputVars == 0)
        return FLT_MAX; // No inputs defined

    FuzzyInput* currentInputs = new FuzzyInput[numInputVars]; // Remove (std::nothrow)
    if (!currentInputs) { // Check for null pointer after allocation
        // Optional: Add a Serial print here if debugging memory issues
        // Serial.println("Error: Failed to allocate memory for currentInputs in evaluateFitness");
        return FLT_MAX; // Memory allocation failed
    }

    for (int i = 0; i < numSets; ++i) {
        // Prepare FuzzyInput structure for this training set
        int currentInputIndex = 0;
        for (int v = 0; v < varCount; ++v) {
            if (vars[v].isInput) {
                if (currentInputIndex >= numInputVars)
                    break; // Safety check
                currentInputs[currentInputIndex].varIndex = v;
                currentInputs[currentInputIndex].value = trainingInputs[i][currentInputIndex]; // Assumes trainingInputs columns match input var order
                currentInputIndex++;
            }
        }

        float predictedOutput;
        FuzzyResult evalResult = evaluate(currentInputs, numInputVars, predictedOutput, outputVarIndex);

        if (evalResult == FUZZY_OK || evalResult == FUZZY_ERROR_NO_RULES_FIRED || evalResult == FUZZY_ERROR_DIVIDE_BY_ZERO) {
            // Handle cases where evaluation completes but might not be ideal
            if (evalResult != FUZZY_OK) {
                // Penalize cases where no rules fired or division by zero occurred
                // Use the expected output for error calculation, but maybe add a penalty
                predictedOutput = (vars[outputVarIndex].min + vars[outputVarIndex].max) / 2.0f; // Assign default mid-value
                totalSquaredError += pow(trainingOutputs[i] - predictedOutput, 2) * 10.0; // Add penalty factor
            } else {
                float error = trainingOutputs[i] - predictedOutput;
                totalSquaredError += error * error;
            }
        } else {
            // Serious error during evaluation - assign high penalty
            totalSquaredError += 1e6; // Large penalty for invalid configurations
        }
    }

    delete[] currentInputs;

    return (numSets > 0) ? (totalSquaredError / numSets) : FLT_MAX;
}

void AutoFuzzy::mutateParameters(float mutationRate, float mutationRange)
{
    for (int i = 0; i < varCount; ++i) { // Iterate through all variables (inputs and outputs)
        for (int j = 0; j < vars[i].mfCount; ++j) {
            MembershipFunction& mf = vars[i].mfs[j];
            int paramCount = (mf.type == MF_TRIANGULAR) ? 3 : 4;

            for (int k = 0; k < paramCount; ++k) {
// Check mutation probability using Arduino's random or standard C rand
// Using Arduino random for simplicity if available
#ifdef ARDUINO
                if (random(1000) / 1000.0f < mutationRate) {
                    float range = (vars[i].max - vars[i].min) * mutationRange;
                    // Generate random value in [-range, range]
                    float change = (random(0, 2001) - 1000) / 1000.0f * range;
                    mf.params[k] += change;
                    // Constrain parameter within the variable's min/max bounds
                    mf.params[k] = constrain(mf.params[k], vars[i].min, vars[i].max);
                }
#else
                // Fallback for standard C/C++ if not Arduino
                if (static_cast<float>(rand()) / RAND_MAX < mutationRate) {
                    float range = (vars[i].max - vars[i].min) * mutationRange;
                    float change = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * range; // [-range, range]
                    mf.params[k] += change;
                    // C++ constrain equivalent
                    mf.params[k] = std::max(vars[i].min, std::min(mf.params[k], vars[i].max));
                }
#endif
            }

            // Ensure parameters remain ordered (e.g., a <= b <= c for triangular)
            // Sort the parameters after mutation
            for (int k = 1; k < paramCount; ++k) {
                if (mf.params[k] < mf.params[k - 1]) {
                    // Simple fix: set to previous value. More robust: sort params[0..paramCount-1]
                    mf.params[k] = mf.params[k - 1];
                }
            }
            // Re-check validity (could be combined with sorting)
            if (!checkMfParamOrder(mf)) {
                // If still invalid after simple fix (unlikely with this fix),
                // potentially revert this MF's mutation or implement a proper sort.
                // For now, we assume the constrain + simple ordering fix is sufficient.
            }
        }
    }
}

FuzzyResult AutoFuzzy::autoTune(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex,
    int iterations, float mutationRate, float mutationRange)
{
    if (numSets <= 0 || !trainingInputs || !trainingOutputs)
        return FUZZY_ERROR_INVALID_PARAMS;
    if (outputVarIndex < 0 || outputVarIndex >= varCount || vars[outputVarIndex].isInput)
        return FUZZY_ERROR_VAR_NOT_FOUND;

    // Backup original parameters for all MFs
    MembershipFunction* backupMFs = new MembershipFunction[FUZZY_MAX_VARS * FUZZY_MAX_MF_PER_VAR]; // Remove (std::nothrow)
    if (!backupMFs) { // Check for null pointer after allocation
        // Optional: Add a Serial print here if debugging memory issues
        // Serial.println("Error: Failed to allocate memory for backupMFs in autoTune");
        return FUZZY_ERROR_NULL_POINTER; // Allocation failed
    }

    int backupIdx = 0;
    for (int i = 0; i < varCount; ++i) {
        for (int j = 0; j < vars[i].mfCount; ++j) {
            if (backupIdx >= FUZZY_MAX_VARS * FUZZY_MAX_MF_PER_VAR)
                break; // Safety
            backupParameters(backupMFs[backupIdx++], vars[i].mfs[j]);
        }
    }

    float bestFitness = evaluateFitness(trainingInputs, trainingOutputs, numSets, outputVarIndex);

    for (int iter = 0; iter < iterations; ++iter) {
        // --- Create Candidate: Mutate current best ---
        // (Could also restore original and mutate from there, or implement population)

        // Mutate the current parameters
        mutateParameters(mutationRate, mutationRange);

        // --- Evaluate Candidate ---
        float currentFitness = evaluateFitness(trainingInputs, trainingOutputs, numSets, outputVarIndex);

        // --- Selection: Keep if better ---
        if (currentFitness < bestFitness) {
            // New best found, keep mutated parameters
            bestFitness = currentFitness;
            // Backup the new best parameters
            backupIdx = 0;
            for (int i = 0; i < varCount; ++i) {
                for (int j = 0; j < vars[i].mfCount; ++j) {
                    if (backupIdx >= FUZZY_MAX_VARS * FUZZY_MAX_MF_PER_VAR)
                        break; // Safety
                    backupParameters(backupMFs[backupIdx++], vars[i].mfs[j]);
                }
            }
        } else {
            // Worse or equal, restore the previous best parameters
            backupIdx = 0;
            for (int i = 0; i < varCount; ++i) {
                for (int j = 0; j < vars[i].mfCount; ++j) {
                    if (backupIdx >= FUZZY_MAX_VARS * FUZZY_MAX_MF_PER_VAR)
                        break; // Safety
                    restoreParameters(vars[i].mfs[j], backupMFs[backupIdx++]);
                }
            }
        }

        // Optional: Add some logging for progress (e.g., every 10 iterations)
        // if (iter % 10 == 0) { Serial.print("Iter: "); Serial.print(iter); Serial.print(" Best Fitness: "); Serial.println(bestFitness); }
    }

    // Clean up backup memory
    delete[] backupMFs;

    return FUZZY_OK;
}

// --- Utility ---
const char* AutoFuzzy::getResultString(FuzzyResult result) const
{
    switch (result) {
    case FUZZY_OK:
        return "OK";
    case FUZZY_ERROR_TOO_MANY_VARS:
        return "Error: Too many variables";
    case FUZZY_ERROR_TOO_MANY_MFS:
        return "Error: Too many membership functions for variable";
    case FUZZY_ERROR_TOO_MANY_RULES:
        return "Error: Too many rules";
    case FUZZY_ERROR_TOO_MANY_ANTECEDENTS:
        return "Error: Too many antecedents in rule";
    case FUZZY_ERROR_VAR_NOT_FOUND:
        return "Error: Variable not found or wrong type (input/output)";
    case FUZZY_ERROR_MF_NOT_FOUND:
        return "Error: Membership function not found";
    case FUZZY_ERROR_INVALID_PARAMS:
        return "Error: Invalid parameters (e.g., MF order, duplicate name)";
    case FUZZY_ERROR_INVALID_RULE:
        return "Error: Invalid rule structure (e.g., input as consequent)";
    case FUZZY_ERROR_NULL_POINTER:
        return "Error: Null pointer argument";
    case FUZZY_ERROR_INDEX_OUT_OF_BOUNDS:
        return "Error: Index out of bounds";
    case FUZZY_ERROR_NO_RULES_FIRED:
        return "Warning: No rules fired during evaluation";
    case FUZZY_ERROR_DIVIDE_BY_ZERO:
        return "Warning: Division by zero during defuzzification";
    default:
        return "Error: Unknown error";
    }
}
