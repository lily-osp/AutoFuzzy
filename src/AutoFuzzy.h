/**
 * @file AutoFuzzy.h
 * @brief AutoFuzzy Fuzzy Logic Library Header
 *
 * A comprehensive fuzzy logic inference system for Arduino platforms,
 * featuring Mamdani-style inference, heuristic parameter tuning, and
 * optimized performance for microcontroller applications.
 */

#ifndef AutoFuzzy_h
#define AutoFuzzy_h

#include "Arduino.h"

/**
 * @defgroup Configuration Library Configuration
 * @brief Preprocessor defines for customizing memory allocation
 *
 * Define these macros BEFORE including AutoFuzzy.h to customize
 * memory limits for your specific Arduino platform.
 * @{
 */

/** Maximum number of linguistic variables (inputs + outputs) */
#ifndef FUZZY_MAX_VARS
#define FUZZY_MAX_VARS 6
#endif

/** Maximum membership functions per linguistic variable */
#ifndef FUZZY_MAX_MF_PER_VAR
#define FUZZY_MAX_MF_PER_VAR 5
#endif

/** Maximum fuzzy rules in the knowledge base */
#ifndef FUZZY_MAX_RULES
#define FUZZY_MAX_RULES 15
#endif

/** Maximum antecedent conditions per rule */
#ifndef FUZZY_MAX_ANTECEDENTS_PER_RULE
#define FUZZY_MAX_ANTECEDENTS_PER_RULE 3
#endif

/** Maximum length for variable and membership function names */
#ifndef FUZZY_MAX_NAME_LEN
#define FUZZY_MAX_NAME_LEN 12
#endif

/** @} */

// Forward Declarations
class AutoFuzzy;

// Core Data Types

/**
 * @brief Input value structure for fuzzy inference evaluation
 *
 * Pairs a variable index with its current crisp input value for
 * fuzzy inference processing.
 */
typedef struct {
    uint8_t varIndex;  /**< Index of the input linguistic variable */
    float value;       /**< Current crisp input value */
} FuzzyInput;

/**
 * @brief Operation result codes
 *
 * Comprehensive error reporting for all library operations,
 * enabling robust error handling and debugging.
 */
enum FuzzyResult {
    FUZZY_OK = 0,                         /**< Operation completed successfully */
    FUZZY_ERROR_TOO_MANY_VARS,            /**< Variable limit exceeded */
    FUZZY_ERROR_TOO_MANY_MFS,             /**< Membership function limit exceeded */
    FUZZY_ERROR_TOO_MANY_RULES,           /**< Rule limit exceeded */
    FUZZY_ERROR_TOO_MANY_ANTECEDENTS,     /**< Antecedent limit exceeded */
    FUZZY_ERROR_VAR_NOT_FOUND,            /**< Specified variable not found */
    FUZZY_ERROR_MF_NOT_FOUND,             /**< Specified membership function not found */
    FUZZY_ERROR_INVALID_PARAMS,           /**< Invalid parameters provided */
    FUZZY_ERROR_INVALID_RULE,             /**< Invalid rule structure */
    FUZZY_ERROR_NULL_POINTER,             /**< Null pointer parameter */
    FUZZY_ERROR_INDEX_OUT_OF_BOUNDS,      /**< Array index out of bounds */
    FUZZY_ERROR_NO_RULES_FIRED,           /**< No rules activated during inference */
    FUZZY_ERROR_DIVIDE_BY_ZERO           /**< Division by zero in calculations */
};

/**
 * @brief Supported membership function types
 */
enum MfType {
    MF_TRIANGULAR,   /**< Triangular membership function (3 parameters) */
    MF_TRAPEZOIDAL   /**< Trapezoidal membership function (4 parameters) */
};

/**
 * @brief Fuzzy logic operators for rule antecedent combination
 */
enum FuzzyOperator {
    FUZZY_AND = 0,   /**< Logical AND (typically MIN operator) */
    FUZZY_OR = 1     /**< Logical OR (typically MAX operator) */
};

/**
 * @class AutoFuzzy
 * @brief Main fuzzy logic controller class
 *
 * Provides a complete Mamdani-style fuzzy inference system with
 * variable management, membership functions, rule definition,
 * inference engine, and heuristic parameter tuning.
 */
class AutoFuzzy {
public:
    /**
     * @brief Constructor - Initialize fuzzy controller
     */
    AutoFuzzy();

    // Variable Management

    /**
     * @brief Add an input linguistic variable
     * @param name Variable name (max FUZZY_MAX_NAME_LEN-1 characters)
     * @param min Minimum valid range value
     * @param max Maximum valid range value
     * @return Variable index (≥0) on success, -1 on error
     */
    int addInput(const char* name, float min = 0.0f, float max = 1.0f);

    /**
     * @brief Add an output linguistic variable
     * @param name Variable name (max FUZZY_MAX_NAME_LEN-1 characters)
     * @param min Minimum valid range value
     * @param max Maximum valid range value
     * @return Variable index (≥0) on success, -1 on error
     */
    int addOutput(const char* name, float min = 0.0f, float max = 1.0f);

    /**
     * @brief Find variable by name
     * @param name Variable name to search for
     * @return Variable index (≥0) if found, -1 if not found
     */
    int findVariable(const char* name) const;

    // Membership Function Management

    /**
     * @brief Add triangular membership function by variable index
     * @param varIndex Target variable index
     * @param mfName Membership function name
     * @param a Left foot point
     * @param b Peak point
     * @param c Right foot point
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addTriangularMF(int varIndex, const char* mfName, float a, float b, float c);

    /**
     * @brief Add triangular membership function by variable name
     * @param varName Target variable name
     * @param mfName Membership function name
     * @param a Left foot point
     * @param b Peak point
     * @param c Right foot point
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addTriangularMF(const char* varName, const char* mfName, float a, float b, float c);

    /**
     * @brief Add trapezoidal membership function by variable index
     * @param varIndex Target variable index
     * @param mfName Membership function name
     * @param a Left foot point
     * @param b Left shoulder point
     * @param c Right shoulder point
     * @param d Right foot point
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addTrapezoidalMF(int varIndex, const char* mfName, float a, float b, float c, float d);

    /**
     * @brief Add trapezoidal membership function by variable name
     * @param varName Target variable name
     * @param mfName Membership function name
     * @param a Left foot point
     * @param b Left shoulder point
     * @param c Right shoulder point
     * @param d Right foot point
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d);

    /**
     * @brief Find membership function by variable and name
     * @param varIndex Variable index
     * @param mfName Membership function name
     * @return MF index (≥0) if found, -1 if not found
     */
    int findMF(int varIndex, const char* mfName) const;

    // Rule Management

    /**
     * @brief Antecedent condition structure for rule definition
     */
    struct Antecedent {
        int varIndex;  /**< Input variable index */
        int mfIndex;   /**< Membership function index for this variable */
    };

    /**
     * @brief Consequent action structure for rule definition
     */
    struct Consequent {
        int varIndex;  /**< Output variable index */
        int mfIndex;   /**< Membership function index for this variable */
    };

    /**
     * @brief Add complex rule with multiple antecedents
     * @param antecedents Array of antecedent conditions
     * @param numAntecedents Number of conditions in array
     * @param op Logical operator for combining antecedents
     * @param consequent Result action
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addRule(const Antecedent antecedents[], int numAntecedents, FuzzyOperator op, const Consequent& consequent);

    /**
     * @brief Add simple single-antecedent rule
     * @param antecedent Single condition
     * @param consequent Result action
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addRule(const Antecedent& antecedent, const Consequent& consequent);

    /**
     * @brief Add rule using variable and MF names
     * @param ifVar Input variable name
     * @param ifMF Input membership function name
     * @param thenVar Output variable name
     * @param thenMF Output membership function name
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF);

    // Fuzzy Inference

    /**
     * @brief Execute fuzzy inference for specified output
     * @param inputs Array of current input values
     * @param numInputs Number of input values provided
     * @param resultValue Reference to store calculated output
     * @param outputVarIndex Target output variable index (default: 0)
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult evaluate(const FuzzyInput inputs[], int numInputs, float& resultValue, int outputVarIndex = 0);

    // Automated Tuning

    /**
     * @brief Perform heuristic parameter optimization
     * @param trainingInputs 2D array of training input scenarios
     * @param trainingOutputs Array of expected output values
     * @param numSets Number of training scenarios
     * @param outputVarIndex Target output variable for optimization
     * @param iterations Number of optimization cycles (default: 100)
     * @param mutationRate Parameter mutation probability (default: 0.1)
     * @param mutationRange Maximum parameter change fraction (default: 0.1)
     * @return FUZZY_OK on success, error code otherwise
     */
    FuzzyResult autoTune(float** trainingInputs, float* trainingOutputs, int numSets, int outputVarIndex,
        int iterations = 100, float mutationRate = 0.1f, float mutationRange = 0.1f);

    // Utility Functions

    /**
     * @brief Convert result code to human-readable string
     * @param result Result code to convert
     * @return Descriptive error message string
     */
    const char* getResultString(FuzzyResult result) const;

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
