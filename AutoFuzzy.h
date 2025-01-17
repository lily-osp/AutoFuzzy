// AutoFuzzy.h
#ifndef AutoFuzzy_h
#define AutoFuzzy_h

#include "Arduino.h"

#define MAX_VARS 10
#define MAX_MEMBERSHIP_FUNCTIONS 5
#define MAX_RULES 50

class AutoFuzzy {
public:
    AutoFuzzy();

    // Input/Output variable management
    void addInput(const char* name, float min, float max);
    void addOutput(const char* name, float min, float max);

    // Membership functions
    void addTriangularMF(const char* varName, const char* mfName, float a, float b, float c);
    void addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d);

    // Rule management
    void addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF);

    // Automated optimization
    void autoOptimize(int iterations = 100);
    float evaluate(float* inputs);

private:
    struct MembershipFunction {
        char name[20];
        uint8_t type; // 0 for triangular, 1 for trapezoidal
        float params[4];
    };

    struct Variable {
        char name[20];
        bool isInput;
        float min;
        float max;
        MembershipFunction mfs[MAX_MEMBERSHIP_FUNCTIONS];
        uint8_t mfCount;
    };

    struct Rule {
        uint8_t ifVar;
        uint8_t ifMF;
        uint8_t thenVar;
        uint8_t thenMF;
    };

    Variable vars[MAX_VARS];
    uint8_t varCount;
    Rule rules[MAX_RULES];
    uint8_t ruleCount;

    float calculateMembership(MembershipFunction& mf, float value);
    void optimizeParameters(float* trainingInputs, float* trainingOutputs, int dataSize);
};

#endif
