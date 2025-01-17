# AutoFuzzy Library for Arduino

AutoFuzzy is a powerful and flexible Arduino library that provides automated fuzzy logic capabilities with self-optimizing features. The library is designed to simplify the implementation of fuzzy logic controllers while offering advanced features for optimization and automation.

---

## Features

- Easy-to-use API for fuzzy logic implementation
- Support for multiple input and output variables
- Triangular and trapezoidal membership functions
- Automated parameter optimization using genetic algorithms
- Up to 10 variables (inputs + outputs)
- Up to 5 membership functions per variable
- Up to 50 fuzzy rules
- Memory-efficient implementation
- Built-in constraint handling
- Real-time evaluation capabilities

---

## Membership Functions: Triangular and Trapezoidal

Membership functions are used in fuzzy logic to determine the degree of membership of an input to a fuzzy set. AutoFuzzy supports **triangular** and **trapezoidal** membership functions:

### Triangular Membership Function

#### Definition:

The triangular membership function is defined by three parameters: a, b, and c. These parameters represent the **lower bound**, the **peak**, and the **upper bound**, respectively.

#### Formula:

$\mu(x) = \begin{cases} 0 & \text{if } x \leq a \text{ or } x \geq c \\ \frac{x - a}{b - a} & \text{if } a \leq x < b \\ \frac{c - x}{c - b} & \text{if } b \leq x < c \end{cases}$

#### Graph:

A triangular membership function has a simple triangle shape:

- Rises linearly from a to b.
- Peaks at b with a membership value of 1.
- Declines linearly from b to c.

#### Use Case:

Triangular membership functions are commonly used for crisp transitions or when boundaries are well-defined.

### Trapezoidal Membership Function

#### Definition:

The trapezoidal membership function is defined by four parameters: a, b, c, and d. These parameters define the start, plateau start, plateau end, and end of the trapezoid.

#### Formula:

$\mu(x) = \begin{cases} 0 & \text{if } x \leq a \text{ or } x \geq d \\ \frac{x - a}{b - a} & \text{if } a \leq x < b \\ 1 & \text{if } b \leq x \leq c \\ \frac{d - x}{d - c} & \text{if } c < x \leq d \end{cases}$

#### Graph:

A trapezoidal membership function forms a trapezoid:

- Rises linearly from a to b.
- Maintains a plateau with a membership value of 1 between b and c.
- Declines linearly from c to d.

#### Use Case:

Trapezoidal membership functions are ideal when variables have a "neutral" or "stable" range where full membership applies.

### Comparison of Triangular and Trapezoidal Membership Functions:

| **Feature**     | **Triangular MF**             | **Trapezoidal MF**         |
| --------------- | ----------------------------- | -------------------------- |
| **Parameters**  | 3 (a, b, c)                   | 4 (a, b, c, d)             |
| **Shape**       | Triangle                      | Trapezoid (with plateau)   |
| **Complexity**  | Simpler                       | Slightly more complex      |
| **Application** | Crisp transitions, small sets | Stable ranges, larger sets |

---

## Installation

1. Download the library as a ZIP file
2. In the Arduino IDE, go to Sketch > Include Library > Add .ZIP Library
3. Select the downloaded ZIP file
4. Restart the Arduino IDE

---

## Basic Usage

### Include the Library

```cpp
#include <AutoFuzzy.h>
```

### Create an Instance

```cpp
AutoFuzzy fuzzy;
```

### Define Variables

```cpp
// Add input variable (name, min value, max value)
fuzzy.addInput("temperature", 0, 100);

// Add output variable (name, min value, max value)
fuzzy.addOutput("fan_speed", 0, 255);
```

### Define Membership Functions

```cpp
// Triangular membership function (variable, name, left, center, right)
fuzzy.addTriangularMF("temperature", "cold", 0, 20, 40);
fuzzy.addTriangularMF("temperature", "hot", 60, 80, 100);

// Trapezoidal membership function (variable, name, left, left-center, right-center, right)
fuzzy.addTrapezoidalMF("fan_speed", "slow", 0, 0, 50, 127);
fuzzy.addTrapezoidalMF("fan_speed", "fast", 127, 200, 255, 255);
```

### Define Rules

```cpp
fuzzy.addRule("temperature", "cold", "fan_speed", "slow");
fuzzy.addRule("temperature", "hot", "fan_speed", "fast");
```

### Optimize and Evaluate

```cpp
// Optional: Run optimization (iterations)
fuzzy.autoOptimize(100);

// Evaluate input and get output
float temp = analogRead(A0) * 100.0 / 1023.0;
float fanSpeed = fuzzy.evaluate(&temp);
```

---

## Example Projects

### 1. Simple LED Brightness Control

Controls LED brightness based on ambient light level.

- [example](examples/simple)

### 2. Plant Watering System

Automated watering system based on soil moisture and temperature.

- [example](examples/intermediate)

### 3. HVAC Control System

Advanced temperature and humidity control.

- [example](examples/advanced)

---

## Advanced Usage

### Multiple Inputs

```cpp
fuzzy.addInput("temperature", 0, 100);
fuzzy.addInput("humidity", 0, 100);
fuzzy.addOutput("fan_speed", 0, 255);

// Evaluate multiple inputs
float inputs[] = {temperature, humidity};
float fanSpeed = fuzzy.evaluate(inputs);
```

### Optimization Parameters

The `autoOptimize()` function uses a genetic algorithm to optimize membership function parameters:

```cpp
// Default optimization
fuzzy.autoOptimize(100); // 100 iterations

// The optimization process:
// 1. Randomly adjusts membership function parameters
// 2. Maintains valid ranges and relationships
// 3. Preserves system stability
// 4. Improves response based on built-in fitness function
```

---

## Other

### Memory Usage

- Each variable: ~32 bytes
- Each membership function: ~24 bytes
- Each rule: ~4 bytes
- Maximum memory usage with default settings: ~1KB

### Limitations

- Maximum 10 variables (combined inputs and outputs)
- Maximum 5 membership functions per variable
- Maximum 50 rules
- No support for custom membership function shapes
- Single output evaluation per call

### Performance Considerations

- Evaluation time increases linearly with number of rules
- Optimization process can take significant time
- Consider reducing iteration count for faster optimization
- Memory usage increases with number of variables and rules

---

## Detailed Explanation

- For more details about the algorithms used in the library, read [here](docs/explanation.md).
- For more details about the library usage, read [here](docs/usage.md).
- For more details about manual tuning, read [here](docs/manual.md).
- And For those who seek the Flowchart and the state diagram of the library u can see here:
  - [Flowchart](docs/flowchart.mermaid)
  - [State diagram](docs/state-diagram.mermaid)

---

## Troubleshooting

### Common Issues

1. System not responding:
   
   - Check if variables are within defined ranges
   - Verify rule definitions
   - Ensure proper connection between inputs and outputs

2. Unexpected behavior:
   
   - Verify membership function overlap
   - Check rule consistency
   - Validate input scaling

3. Memory issues:
   
   - Reduce number of variables/rules
   - Optimize membership function count
   - Consider using PROGMEM for constant data

---

## Best Practices

1. System Design:
   
   - Start with simple systems and gradually add complexity
   - Use appropriate ranges for variables
   - Ensure smooth overlap between membership functions
   - Design rules to cover all possible scenarios

2. Optimization:
   
   - Run optimization during setup if possible
   - Use appropriate iteration count for your application
   - Monitor system performance after optimization
   - Consider periodic re-optimization for dynamic systems

3. Memory Management:
   
   - Use minimum necessary membership functions
   - Optimize rule count
   - Clean up unused variables and rules

---

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a new Pull Request

## License

This library is released under the MIT License. See the LICENSE file
