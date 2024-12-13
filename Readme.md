# AutoFuzzy Library for Arduino

## Overview
AutoFuzzy is a powerful and flexible Arduino library that provides automated fuzzy logic capabilities with self-optimizing features. The library is designed to simplify the implementation of fuzzy logic controllers while offering advanced features for optimization and automation.

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

## Installation

1. Download the library as a ZIP file
2. In the Arduino IDE, go to Sketch > Include Library > Add .ZIP Library
3. Select the downloaded ZIP file
4. Restart the Arduino IDE

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

## Example Projects

### 1. Simple LED Brightness Control
Controls LED brightness based on ambient light level.
```cpp
// See examples/SimpleLED/SimpleLED.ino
```

### 2. Plant Watering System
Automated watering system based on soil moisture and temperature.
```cpp
// See examples/PlantWatering/PlantWatering.ino
```

### 3. HVAC Control System
Advanced temperature and humidity control with energy optimization.
```cpp
// See examples/HVACControl/HVACControl.ino
```

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
fuzzy.autoOptimize(100);  // 100 iterations

// The optimization process:
// 1. Randomly adjusts membership function parameters
// 2. Maintains valid ranges and relationships
// 3. Preserves system stability
// 4. Improves response based on built-in fitness function
```

## Memory Usage
- Each variable: ~32 bytes
- Each membership function: ~24 bytes
- Each rule: ~4 bytes
- Maximum memory usage with default settings: ~1KB

## Limitations
- Maximum 10 variables (combined inputs and outputs)
- Maximum 5 membership functions per variable
- Maximum 50 rules
- No support for custom membership function shapes
- Single output evaluation per call

## Performance Considerations
- Evaluation time increases linearly with number of rules
- Optimization process can take significant time
- Consider reducing iteration count for faster optimization
- Memory usage increases with number of variables and rules

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

## Contributing
1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a new Pull Request

## License
This library is released under the MIT License. See the LICENSE file for details.

## Support
For bugs, feature requests, and discussions:
- Open an issue on GitHub
- Contact: https://azzar.netlify.app

## Version History
- 1.0.0 (2024-12-13)
  - Initial release
  - Basic fuzzy logic functionality
  - Automated optimization features
  - Example projects

## Future Development
- Support for custom membership function shapes
- Advanced optimization algorithms
- Multiple output evaluation
- Enhanced data logging and analysis tools
