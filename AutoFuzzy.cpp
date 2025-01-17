// AutoFuzzy.cpp
#include "AutoFuzzy.h"

AutoFuzzy::AutoFuzzy()
{
    varCount = 0;
    ruleCount = 0;
}

void AutoFuzzy::addInput(const char* name, float min, float max)
{
    if (varCount >= MAX_VARS)
        return;

    strncpy(vars[varCount].name, name, 19);
    vars[varCount].name[19] = '\0';
    vars[varCount].isInput = true;
    vars[varCount].min = min;
    vars[varCount].max = max;
    vars[varCount].mfCount = 0;
    varCount++;
}

void AutoFuzzy::addOutput(const char* name, float min, float max)
{
    if (varCount >= MAX_VARS)
        return;

    strncpy(vars[varCount].name, name, 19);
    vars[varCount].name[19] = '\0';
    vars[varCount].isInput = false;
    vars[varCount].min = min;
    vars[varCount].max = max;
    vars[varCount].mfCount = 0;
    varCount++;
}

void AutoFuzzy::addTriangularMF(const char* varName, const char* mfName, float a, float b, float c)
{
    // Find variable
    int varIndex = -1;
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i].name, varName) == 0) {
            varIndex = i;
            break;
        }
    }

    if (varIndex == -1 || vars[varIndex].mfCount >= MAX_MEMBERSHIP_FUNCTIONS)
        return;

    MembershipFunction& mf = vars[varIndex].mfs[vars[varIndex].mfCount];
    strncpy(mf.name, mfName, 19);
    mf.name[19] = '\0';
    mf.type = 0; // Triangular
    mf.params[0] = a;
    mf.params[1] = b;
    mf.params[2] = c;
    vars[varIndex].mfCount++;
}

void AutoFuzzy::addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d)
{
    // Find variable
    int varIndex = -1;
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i].name, varName) == 0) {
            varIndex = i;
            break;
        }
    }

    if (varIndex == -1 || vars[varIndex].mfCount >= MAX_MEMBERSHIP_FUNCTIONS)
        return;

    MembershipFunction& mf = vars[varIndex].mfs[vars[varIndex].mfCount];
    strncpy(mf.name, mfName, 19);
    mf.name[19] = '\0';
    mf.type = 1; // Trapezoidal
    mf.params[0] = a;
    mf.params[1] = b;
    mf.params[2] = c;
    mf.params[3] = d;
    vars[varIndex].mfCount++;
}

void AutoFuzzy::addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF)
{
    if (ruleCount >= MAX_RULES)
        return;

    // Find variables and membership functions
    int ifVarIndex = -1, thenVarIndex = -1;
    int ifMFIndex = -1, thenMFIndex = -1;

    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i].name, ifVar) == 0) {
            ifVarIndex = i;
            for (int j = 0; j < vars[i].mfCount; j++) {
                if (strcmp(vars[i].mfs[j].name, ifMF) == 0) {
                    ifMFIndex = j;
                    break;
                }
            }
        }
        if (strcmp(vars[i].name, thenVar) == 0) {
            thenVarIndex = i;
            for (int j = 0; j < vars[i].mfCount; j++) {
                if (strcmp(vars[i].mfs[j].name, thenMF) == 0) {
                    thenMFIndex = j;
                    break;
                }
            }
        }
    }

    if (ifVarIndex == -1 || thenVarIndex == -1 || ifMFIndex == -1 || thenMFIndex == -1)
        return;

    rules[ruleCount].ifVar = ifVarIndex;
    rules[ruleCount].ifMF = ifMFIndex;
    rules[ruleCount].thenVar = thenVarIndex;
    rules[ruleCount].thenMF = thenMFIndex;
    ruleCount++;
}

float AutoFuzzy::calculateMembership(MembershipFunction& mf, float value)
{
    if (mf.type == 0) { // Triangular
        if (value <= mf.params[0] || value >= mf.params[2])
            return 0;
        if (value <= mf.params[1]) {
            return (value - mf.params[0]) / (mf.params[1] - mf.params[0]);
        }
        return (mf.params[2] - value) / (mf.params[2] - mf.params[1]);
    } else { // Trapezoidal
        if (value <= mf.params[0] || value >= mf.params[3])
            return 0;
        if (value >= mf.params[1] && value <= mf.params[2])
            return 1;
        if (value < mf.params[1]) {
            return (value - mf.params[0]) / (mf.params[1] - mf.params[0]);
        }
        return (mf.params[3] - value) / (mf.params[3] - mf.params[2]);
    }
}

float AutoFuzzy::evaluate(float* inputs)
{
    float outputSum = 0;
    float weightSum = 0;

    for (int i = 0; i < ruleCount; i++) {
        float inputMembership = calculateMembership(
            vars[rules[i].ifVar].mfs[rules[i].ifMF],
            inputs[rules[i].ifVar]);

        if (inputMembership > 0) {
            // Use center of membership function as output value
            float outputValue;
            MembershipFunction& outMF = vars[rules[i].thenVar].mfs[rules[i].thenMF];

            if (outMF.type == 0) { // Triangular
                outputValue = outMF.params[1]; // Center of triangle
            } else { // Trapezoidal
                outputValue = (outMF.params[1] + outMF.params[2]) / 2; // Center of trapezoid
            }

            outputSum += outputValue * inputMembership;
            weightSum += inputMembership;
        }
    }

    return weightSum > 0 ? outputSum / weightSum : 0;
}

void AutoFuzzy::autoOptimize(int iterations)
{
    // Simple genetic algorithm to optimize membership function parameters
    const float mutationRate = 0.1;
    const float mutationRange = 0.1;

    for (int iter = 0; iter < iterations; iter++) {
        // For each membership function
        for (int i = 0; i < varCount; i++) {
            for (int j = 0; j < vars[i].mfCount; j++) {
                // Randomly mutate parameters
                if (random(100) / 100.0 < mutationRate) {
                    MembershipFunction& mf = vars[i].mfs[j];
                    int paramCount = mf.type == 0 ? 3 : 4;

                    for (int k = 0; k < paramCount; k++) {
                        float range = (vars[i].max - vars[i].min) * mutationRange;
                        mf.params[k] += random(-range * 1000, range * 1000) / 1000.0;

                        // Keep within variable bounds
                        mf.params[k] = constrain(mf.params[k], vars[i].min, vars[i].max);
                    }

                    // Ensure parameters remain ordered
                    for (int k = 1; k < paramCount; k++) {
                        if (mf.params[k] < mf.params[k - 1]) {
                            mf.params[k] = mf.params[k - 1];
                        }
                    }
                }
            }
        }
    }
}
