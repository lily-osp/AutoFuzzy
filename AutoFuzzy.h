#ifndef AutoFuzzy_h
#define AutoFuzzy_h

#include "Arduino.h"

// --- Configuration ---
// You can override these by defining them BEFORE including AutoFuzzy.h
#ifndef FUZZY_MAX_VARS
#define FUZZY_MAX_VARS 10 // Total number of input and output variables
#endif

#ifndef FUZZY_MAX_MF_PER_VAR
#define FUZZY_MAX_MF_PER_VAR 5 // Max membership functions per variable
#endif

#ifndef FUZZY_MAX_RULES
#define FUZZY_MAX_RULES 50 // Max total rules
#endif

#ifndef FUZZY_MAX_ANTECEDENTS_PER_RULE
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 5 // Max IF conditions per rule (e.g., IF x is A AND y is B)
#endif

#ifndef FUZZY_MAX_NAME_LEN
#define FUZZY_MAX_NAME_LEN 20 // Max length for variable and MF names (including null terminator)
#endif
// --- End Configuration ---

// Forward declaration
class AutoFuzzy;

// --- Type Definitions ---

// Represents a specific input value for evaluation
typedef struct {
    uint8_t varIndex; // Index of the input variable
    float value; // Current value of the input variable
} FuzzyInput;

// Represents the overall result of an operation
enum FuzzyResult {
    FUZZY_OK = 0,
    FUZZY_ERROR_TOO_MANY_VARS,
    FUZZY_ERROR_TOO_MANY_MFS,
    FUZZY_ERROR_TOO_MANY_RULES,
    FUZZY_ERROR_TOO_MANY_ANTECEDENTS,
    FUZZY_ERROR_VAR_NOT_FOUND,
    FUZZY_ERROR_MF_NOT_FOUND,
    FUZZY_ERROR_INVALID_PARAMS,
    FUZZY_ERROR_INVALID_RULE, // e.g., input as consequent, output as antecedent
    FUZZY_ERROR_NULL_POINTER,
    FUZZY_ERROR_INDEX_OUT_OF_BOUNDS,
    FUZZY_ERROR_NO_RULES_FIRED,
    FUZZY_ERROR_DIVIDE_BY_ZERO
};

// Type of Membership Function
enum MfType {
    MF_TRIANGULAR,
    MF_TRAPEZOIDAL
    // Add other types here if needed (e.g., Gaussian)
};

// Fuzzy Logic Operators for combining rule antecedents
enum FuzzyOperator {
    FUZZY_AND = 0, // Typically MIN operator
    FUZZY_OR = 1 // Typically MAX operator
    // FUZZY_AND_PROD, // Product AND
    // FUZZY_OR_PROBOR // Probabilistic OR
};

// --- Class Definition ---

class AutoFuzzy {
public:
    AutoFuzzy();

    // --- Variable Management ---
    // Adds an input variable. Returns variable index or -1 on error.
    int addInput(const char* name, float min = 0.0f, float max = 1.0f);
    // Adds an output variable. Returns variable index or -1 on error.
    int addOutput(const char* name, float min = 0.0f, float max = 1.0f);
    int findVariable(const char* name) const; // Returns index or -1

    // --- Membership Function Management ---
    FuzzyResult addTriangularMF(int varIndex, const char* mfName, float a, float b, float c);
    FuzzyResult addTriangularMF(const char* varName, const char* mfName, float a, float b, float c);
    FuzzyResult addTrapezoidalMF(int varIndex, const char* mfName, float a, float b, float c, float d);
    FuzzyResult addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d);
    int findMF(int varIndex, const char* mfName) const; // Returns MF index or -1

    // --- Rule Management ---
    // Structure to define one antecedent (IF condition)
    struct Antecedent {
        int varIndex; // Index of the input variable
        int mfIndex; // Index of the membership function for this variable
    };
    // Structure to define the consequent (THEN action)
    struct Consequent {
        int varIndex; // Index of the output variable
        int mfIndex; // Index of the membership function for this variable
    };

    // Adds a rule: IF (antecedents combined by op) THEN consequent
    FuzzyResult addRule(const Antecedent antecedents[], int numAntecedents, FuzzyOperator op, const Consequent& consequent);
    // Helper for simple SISO rules (IF antecedent THEN consequent)
    FuzzyResult addRule(const Antecedent& antecedent, const Consequent& consequent);
    // Helper using names (less efficient, more convenient)
    FuzzyResult addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF);

    // --- Fuzzy Inference ---
    // Calculates the fuzzy output based on current inputs
    // inputs: array of FuzzyInput structs
    // numInputs: number of elements in the inputs array
    // resultValue: pointer to store the calculated output
    // outputVarIndex: index of the output variable to calculate for
    FuzzyResult evaluate(const FuzzyInput inputs[], int numInputs, float& resultValue, int outputVarIndex = 0); // Defaults to first output var

    // --- Automated Tuning (Heuristic) ---
    // Tunes MF parameters based on provided training data using random mutations.
    // This is a *heuristic* tuner, not a guaranteed optimizer. It needs evaluation data.
    // trainingInputs: Array of input value arrays (numSets x numInputVars)
    // trainingOutputs: Array of expected output values (numSets)
    // numSets: Number of training data points
    // outputVarIndex: Index of the output variable being tuned
    // iterations: Number of tuning cycles
    // mutationRate: Probability (0-1) of mutating a parameter per iteration
    // mutationRange: Max fractional change (0-1) relative to variable range
    FuzzyResult autoTune(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex,
        int iterations = 100, float mutationRate = 0.1f, float mutationRange = 0.1f);

    // --- Utility ---
    const char* getResultString(FuzzyResult result) const; // Get human-readable error string

private:
    struct MembershipFunction {
        char name[FUZZY_MAX_NAME_LEN];
        MfType type;
        float params[4]; // Stores a,b,c for triangular; a,b,c,d for trapezoidal
        // Add other MF parameters if needed
    };

    struct Variable {
        char name[FUZZY_MAX_NAME_LEN];
        bool isInput;
        float min;
        float max;
        MembershipFunction mfs[FUZZY_MAX_MF_PER_VAR];
        uint8_t mfCount;
        bool isOutput() const { return !isInput; }
    };

    struct Rule {
        uint8_t antecedentVarIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
        uint8_t antecedentMfIndices[FUZZY_MAX_ANTECEDENTS_PER_RULE];
        uint8_t numAntecedents;
        FuzzyOperator op; // How to combine antecedents (AND/OR)
        uint8_t consequentVarIndex;
        uint8_t consequentMfIndex;
    };

    Variable vars[FUZZY_MAX_VARS];
    uint8_t varCount;
    Rule rules[FUZZY_MAX_RULES];
    uint8_t ruleCount;

    // Internal helper to get variable pointer safely
    Variable* getVariable(int varIndex);
    const Variable* getVariable(int varIndex) const;

    // Internal helper to get MF pointer safely
    MembershipFunction* getMF(int varIndex, int mfIndex);
    const MembershipFunction* getMF(int varIndex, int mfIndex) const;

    // Fuzzy Logic Core
    float calculateMembership(const MembershipFunction& mf, float value) const;
    float applyOperator(FuzzyOperator op, float value1, float value2) const; // Applies AND/OR
    float calculateRuleActivation(const Rule& rule, const FuzzyInput inputs[], int numInputs) const;
    float getMfCentroid(const MembershipFunction& mf) const; // Gets representative value for defuzzification

    // Tuning Helpers
    float evaluateFitness(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex);
    void mutateParameters(float mutationRate, float mutationRange);
    void backupParameters(MembershipFunction& dest, const MembershipFunction& src);
    void restoreParameters(MembershipFunction& dest, const MembershipFunction& src);
    bool checkMfParamOrder(const MembershipFunction& mf) const; // Ensure params are ordered correctly

    // Internal Finders (using indices for rules)
    int findInputVarIndex(const char* name) const;
    int findOutputVarIndex(const char* name) const;
};

#endif // AutoFuzzy_h
