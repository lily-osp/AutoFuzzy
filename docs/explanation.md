# Fuzzy Logic Explained

Fuzzy logic is a computational approach based on the principles of fuzzy set theory, which extends classical boolean logic to handle the concept of partial truth. Unlike traditional binary logic, where variables take only two values (true or false), fuzzy logic allows variables to have degrees of truth, represented by values between 0 and 1. This makes it particularly useful for dealing with uncertainty and vagueness in real-world problems.

---

## Key Concepts of Fuzzy Logic

### 1. **Fuzzy Sets**
A fuzzy set is a collection of elements with a continuum of grades of membership. Each element in a fuzzy set has a membership value (also called a degree of membership) ranging from 0 (not a member) to 1 (fully a member).

#### Example:
A fuzzy set representing the concept of "warm temperature" might include values like:
- 20°C: Membership = 0.2
- 25°C: Membership = 0.5
- 30°C: Membership = 1.0
- 35°C: Membership = 0.8

Fuzzy sets help in modeling uncertainty in the real world, such as "How warm is it outside?" rather than a binary yes/no answer.

### 2. **Membership Functions**
A membership function (MF) defines how each element in the input space maps to a membership value. Common types include:

- **Triangular:** Defined by a triangular shape, characterized by three parameters: left, center, and right.
- **Trapezoidal:** Defined by a trapezoid shape, characterized by four parameters: left, left-center, right-center, and right.
- **Gaussian:** A bell-shaped curve commonly used for smooth transitions.
- **Custom:** User-defined shapes for specialized applications.

#### Membership Function Applications:
Membership functions translate real-world values (e.g., temperature, speed) into degrees of truth that the fuzzy logic system can process. These degrees of truth act as the backbone of fuzzy logic, determining how inputs interact with the rules.

#### Triangular MF Example:
A triangular MF for the fuzzy set "cold" can be defined as:

```text
     1
     |      /\
     |     /  \
0.5  |    /    \
     |   /      \
     |  /        \
     -----------------
        0   20  40
```
Here, the membership value is 1 at 20°C, decreases linearly to 0 at 0°C and 40°C.

#### Trapezoidal MF Example:
A trapezoidal MF extends the triangle by adding a "flat" top region, which allows for full membership over a range of values. For instance, "medium speed" might be fully true from 30 to 50 units:

```text
1
|       _______
|      /       \
|     /         \
|    /           \
-----------------------
    10   30   50   70
```

### 3. **Fuzzy Rules**
Fuzzy rules are "if-then" statements that describe how to map input fuzzy sets to output fuzzy sets. They allow the system to make decisions or infer outputs based on the inputs.

#### Syntax:
```
IF <condition> THEN <conclusion>
```
Rules can involve multiple conditions combined using logical operators like AND, OR, or NOT.

#### Example:
```
IF temperature IS cold THEN fan_speed IS slow
IF temperature IS hot THEN fan_speed IS fast
```
Here, "temperature IS cold" and "fan_speed IS slow" are fuzzy sets defined by their respective membership functions.

### 4. **Fuzzification**
Fuzzification converts crisp inputs into fuzzy values. For instance, if the temperature is 25°C, it might belong partially to both the "cold" and "warm" sets. The degree of membership is calculated using the corresponding membership functions.

#### Example:
- 25°C:
  - Cold: Membership = 0.5
  - Warm: Membership = 0.5

This allows the system to consider input ambiguity rather than assigning a strict classification.

### 5. **Inference Engine**
The inference engine evaluates the fuzzy rules based on the fuzzified inputs and determines the output fuzzy sets. It involves:
- Aggregating the conditions of each rule.
- Determining the output fuzzy sets' membership values based on rule weights.

#### Example:
Given the rule:
```
IF temperature IS cold THEN fan_speed IS slow
```
If the input temperature has a membership of 0.5 in "cold," the output membership for "slow fan_speed" will also be 0.5.

### 6. **Defuzzification**
Defuzzification converts fuzzy output sets back into crisp values that can be used for decision-making or control purposes. Common methods include:
- **Centroid:** Computes the center of gravity of the output fuzzy set.
- **Max Membership:** Chooses the output value with the highest membership.

#### Example:
If the output fuzzy set for "fan_speed" spans from 50 to 127 with a peak at 90, the defuzzified crisp value might be calculated as the weighted average.

---

## General Advantages of Fuzzy Logic

1. **Human-Like Reasoning:** Mimics human decision-making by handling uncertainty and vagueness.
2. **Versatile:** Can be applied to a wide range of domains, including control systems, AI, and robotics.
3. **Easy to Understand:** Rules and membership functions are intuitive and easy to design.
4. **Robust:** Handles noisy and imprecise data effectively.

---

## What Fuzzy Logic Means in the `AutoFuzzy` Library

The `AutoFuzzy` library simplifies fuzzy logic implementation on Arduino devices, providing tools for handling fuzzification, inference, and defuzzification. Here's what each concept from fuzzy logic means in terms of its library implementation:

### 1. **Fuzzy Sets and Membership Functions**
- **Implementation:** Membership functions are defined for variables using `addTriangularMF` or `addTrapezoidalMF`. Each membership function represents a fuzzy set by defining the relationship between an input/output variable and its degree of truth.
- **Purpose:** These functions act as the foundation for input processing, translating real-world values into fuzzy values that the system can work with.
- **Example in Code:**
  ```cpp
  fuzzy.addTriangularMF("temperature", "cold", 0, 20, 40);
  fuzzy.addTrapezoidalMF("fan_speed", "slow", 0, 0, 50, 127);
  ```
  These methods assign fuzzy sets to the variables "temperature" and "fan_speed."

### 2. **Fuzzy Rules**
- **Implementation:** Rules are added using the `addRule` method, which links fuzzy sets (membership functions) of input variables to those of output variables.
- **Purpose:** Rules govern how inputs are translated to outputs, mimicking human decision-making processes.
- **Example in Code:**
  ```cpp
  fuzzy.addRule("temperature", "cold", "fan_speed", "slow");
  ```
  This defines the rule: IF "temperature" IS "cold" THEN "fan_speed" IS "slow."

### 3. **Fuzzification**
- **Implementation:** The `evaluate` function automatically converts crisp input values into fuzzy sets by calculating their degree of membership using the parameters of the defined membership functions.
- **Purpose:** Enables the system to process real-world inputs with inherent uncertainties.
- **Example in Code:**
  ```cpp
  float inputs[] = {25.0}; // Example input: 25°C
  float result = fuzzy.evaluate(inputs);
  ```
  This determines the degree to which 25°C belongs to each fuzzy set (e.g., "cold," "warm").

### 4. **Inference Engine**
- **Implementation:** The inference process combines fuzzy rules and degrees of membership to compute the fuzzy output sets. The engine supports methods like Max-Min to aggregate the rules' effects.
- **Purpose:** Bridges input fuzziness with actionable outputs by applying logical reasoning to rules.
- **Library Functionality:** This step is handled internally during evaluation.

### 5. **Defuzzification**
- **Implementation:** The `evaluate` function also converts the fuzzy output set into a crisp value by computing the centroid or using another defuzzification method.
- **Purpose:** Produces actionable output values from fuzzy results.
- **Example:**
  The crisp output from `evaluate` can directly control hardware (e.g., adjust a motor speed based on the computed "fan_speed").

### 6. **Optimization**
- **Implementation:** The `autoOptimize` method adjusts membership function parameters to improve the system's performance for

