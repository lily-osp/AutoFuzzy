# AutoFuzzy Library Usage Guide

The `AutoFuzzy` library is designed to simplify the implementation of fuzzy logic systems on Arduino. It allows you to define input and output variables, membership functions, and rules, and then evaluate the system to make decisions based on fuzzy logic.

## Table of Contents
1. [Installation](#installation)
2. [Getting Started](#getting-started)
3. [API Reference](#api-reference)
   - [Input/Output Variables](#inputoutput-variables)
   - [Membership Functions](#membership-functions)
   - [Rules](#rules)
   - [Evaluation](#evaluation)
   - [Optimization](#optimization)
4. [Examples](#examples)
   - [Simple Example: LED Brightness Control](#simple-example-led-brightness-control)
   - [Intermediate Example: Plant Watering System](#intermediate-example-plant-watering-system)
   - [Advanced Example: HVAC Control System](#advanced-example-hvac-control-system)
5. [Limitations](#limitations)
6. [Contributing](#contributing)

---

## Installation

1. Download the `AutoFuzzy` library as a `.zip` file or clone the repository.
2. Open the Arduino IDE.
3. Go to `Sketch > Include Library > Add .ZIP Library...`.
4. Select the downloaded `.zip` file or the cloned repository folder.
5. The library is now installed and ready to use.

---

## Getting Started

To use the `AutoFuzzy` library, include it in your sketch:

```cpp
#include <AutoFuzzy.h>
```

Create an instance of the `AutoFuzzy` class:

```cpp
AutoFuzzy fuzzy;
```

Now you can define your fuzzy logic system by adding inputs, outputs, membership functions, and rules.

---

## API Reference

### Input/Output Variables

#### `addInput(const char* name, float min, float max)`
- **Description**: Adds an input variable to the fuzzy system.
- **Parameters**:
  - `name`: The name of the input variable (e.g., "temperature").
  - `min`: The minimum value of the input range.
  - `max`: The maximum value of the input range.
- **Example**:
  ```cpp
  fuzzy.addInput("temperature", 0, 100);
  ```

#### `addOutput(const char* name, float min, float max)`
- **Description**: Adds an output variable to the fuzzy system.
- **Parameters**:
  - `name`: The name of the output variable (e.g., "fanSpeed").
  - `min`: The minimum value of the output range.
  - `max`: The maximum value of the output range.
- **Example**:
  ```cpp
  fuzzy.addOutput("fanSpeed", 0, 255);
  ```

---

### Membership Functions

#### `addTriangularMF(const char* varName, const char* mfName, float a, float b, float c)`
- **Description**: Adds a triangular membership function to a variable.
- **Parameters**:
  - `varName`: The name of the variable to which the membership function belongs.
  - `mfName`: The name of the membership function (e.g., "low").
  - `a`: The leftmost point of the triangle.
  - `b`: The peak of the triangle.
  - `c`: The rightmost point of the triangle.
- **Example**:
  ```cpp
  fuzzy.addTriangularMF("temperature", "low", 0, 20, 40);
  ```

#### `addTrapezoidalMF(const char* varName, const char* mfName, float a, float b, float c, float d)`
- **Description**: Adds a trapezoidal membership function to a variable.
- **Parameters**:
  - `varName`: The name of the variable to which the membership function belongs.
  - `mfName`: The name of the membership function (e.g., "medium").
  - `a`: The leftmost point of the trapezoid.
  - `b`: The start of the flat top.
  - `c`: The end of the flat top.
  - `d`: The rightmost point of the trapezoid.
- **Example**:
  ```cpp
  fuzzy.addTrapezoidalMF("temperature", "medium", 30, 40, 60, 70);
  ```

---

### Rules

#### `addRule(const char* ifVar, const char* ifMF, const char* thenVar, const char* thenMF)`
- **Description**: Adds a rule to the fuzzy system.
- **Parameters**:
  - `ifVar`: The name of the input variable in the "if" part of the rule.
  - `ifMF`: The name of the membership function in the "if" part of the rule.
  - `thenVar`: The name of the output variable in the "then" part of the rule.
  - `thenMF`: The name of the membership function in the "then" part of the rule.
- **Example**:
  ```cpp
  fuzzy.addRule("temperature", "high", "fanSpeed", "high");
  ```

---

### Evaluation

#### `evaluate(float* inputs)`
- **Description**: Evaluates the fuzzy system based on the provided input values.
- **Parameters**:
  - `inputs`: An array of input values corresponding to the input variables.
- **Returns**: The output value computed by the fuzzy system.
- **Example**:
  ```cpp
  float inputs[] = {25.0};  // Temperature = 25°C
  float output = fuzzy.evaluate(inputs);
  ```

---

### Optimization

#### `autoOptimize(int iterations = 100)`
- **Description**: Automatically optimizes the membership function parameters using a simple genetic algorithm.
- **Parameters**:
  - `iterations`: The number of optimization iterations (default: 100).
- **Example**:
  ```cpp
  fuzzy.autoOptimize(200);
  ```

---

## Examples

### Simple Example: LED Brightness Control
This example controls the brightness of an LED based on a potentiometer input.

```cpp
#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

void setup() {
  fuzzy.addInput("potValue", 0, 1023);
  fuzzy.addOutput("ledBrightness", 0, 255);

  fuzzy.addTriangularMF("potValue", "low", 0, 255, 511);
  fuzzy.addTriangularMF("potValue", "medium", 255, 511, 767);
  fuzzy.addTriangularMF("potValue", "high", 511, 767, 1023);

  fuzzy.addTriangularMF("ledBrightness", "dim", 0, 64, 128);
  fuzzy.addTriangularMF("ledBrightness", "medium", 64, 128, 192);
  fuzzy.addTriangularMF("ledBrightness", "bright", 128, 192, 255);

  fuzzy.addRule("potValue", "low", "ledBrightness", "dim");
  fuzzy.addRule("potValue", "medium", "ledBrightness", "medium");
  fuzzy.addRule("potValue", "high", "ledBrightness", "bright");
}

void loop() {
  int potValue = analogRead(A0);
  float ledBrightness = fuzzy.evaluate((float*)&potValue);
  analogWrite(9, (int)ledBrightness);
  delay(100);
}
```

---

### Intermediate Example: Plant Watering System
This example automates plant watering based on soil moisture and temperature.

```cpp
#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

void setup() {
  fuzzy.addInput("moisture", 0, 1023);
  fuzzy.addInput("temp", 0, 50);
  fuzzy.addOutput("pump", 0, 1);

  fuzzy.addTriangularMF("moisture", "dry", 0, 300, 500);
  fuzzy.addTriangularMF("moisture", "moist", 300, 500, 700);
  fuzzy.addTriangularMF("moisture", "wet", 500, 700, 1023);

  fuzzy.addTriangularMF("temp", "cold", 0, 10, 20);
  fuzzy.addTriangularMF("temp", "warm", 10, 20, 30);
  fuzzy.addTriangularMF("temp", "hot", 20, 30, 50);

  fuzzy.addTriangularMF("pump", "off", 0, 0, 0.5);
  fuzzy.addTriangularMF("pump", "on", 0.5, 1, 1);

  fuzzy.addRule("moisture", "dry", "pump", "on");
  fuzzy.addRule("moisture", "moist", "temp", "cold", "pump", "off");
  fuzzy.addRule("moisture", "moist", "temp", "warm", "pump", "on");
  fuzzy.addRule("moisture", "moist", "temp", "hot", "pump", "on");
  fuzzy.addRule("moisture", "wet", "pump", "off");
}

void loop() {
  int moisture = analogRead(A0);
  int temp = analogRead(A1) / 20.47;
  float inputs[] = {(float)moisture, (float)temp};
  float pumpState = fuzzy.evaluate(inputs);
  digitalWrite(8, pumpState > 0.5 ? HIGH : LOW);
  delay(1000);
}
```

---

### Advanced Example: HVAC Control System
This example controls heating and cooling based on temperature and humidity.

```cpp
#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

void setup() {
  fuzzy.addInput("temp", 0, 50);
  fuzzy.addInput("humidity", 0, 100);
  fuzzy.addOutput("heater", 0, 255);
  fuzzy.addOutput("cooler", 0, 255);

  fuzzy.addTriangularMF("temp", "cold", 0, 10, 20);
  fuzzy.addTriangularMF("temp", "comfort", 10, 20, 30);
  fuzzy.addTriangularMF("temp", "hot", 20, 30, 50);

  fuzzy.addTriangularMF("humidity", "low", 0, 30, 50);
  fuzzy.addTriangularMF("humidity", "comfort", 30, 50, 70);
  fuzzy.addTriangularMF("humidity", "high", 50, 70, 100);

  fuzzy.addTriangularMF("heater", "off", 0, 0, 128);
  fuzzy.addTriangularMF("heater", "low", 0, 128, 255);
  fuzzy.addTriangularMF("heater", "high", 128, 255, 255);

  fuzzy.addTriangularMF("cooler", "off", 0, 0, 128);
  fuzzy.addTriangularMF("cooler", "low", 0, 128, 255);
  fuzzy.addTriangularMF("cooler", "high", 128, 255, 255);

  fuzzy.addRule("temp", "cold", "heater", "high");
  fuzzy.addRule("temp", "cold", "cooler", "off");
  fuzzy.addRule("temp", "comfort", "humidity", "low", "heater", "low");
  fuzzy.addRule("temp", "comfort", "humidity", "comfort", "heater", "off");
  fuzzy.addRule("temp", "comfort", "humidity", "high", "cooler", "low");
  fuzzy.addRule("temp", "hot", "cooler", "high");
  fuzzy.addRule("temp", "hot", "heater", "off");
}

void loop() {
  int temp = analogRead(A0) / 20.47;
  int humidity = analogRead(A1) / 10.23;
  float inputs[] = {(float)temp, (float)humidity};
  float outputs[2];
  outputs[0] = fuzzy.evaluate(inputs);  // Heater
  outputs[1] = fuzzy.evaluate(inputs);  // Cooler
  analogWrite(9, (int)outputs[0]);
  analogWrite(10, (int)outputs[1]);
  delay(1000);
}
```

---

## Limitations
- The library supports a maximum of 10 variables, 5 membership functions per variable, and 50 rules.
- The optimization function (`autoOptimize`) is a simple genetic algorithm and may not be suitable for complex systems.

---

## Contributing
Contributions are welcome! Please open an issue or submit a pull request on the GitHub repository.

---
