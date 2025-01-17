#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

// Pin definitions
const int tempPin = A0; // Temperature sensor
const int humidityPin = A1; // Humidity sensor
const int heaterPin = 9; // Heater control (PWM)
const int coolerPin = 10; // Cooler control (PWM)

void setup()
{
    Serial.begin(9600);

    // Advanced Example: HVAC Control System
    // Controls heating and cooling based on temperature and humidity
    // - Two inputs (temperature and humidity) and two outputs (heater and cooler)
    // - Three membership functions for each input and output
    // - Complex rules considering both temperature and humidity
    // - Ideal for advanced users exploring multi-input, multi-output fuzzy systems
    // - Perfect for smart climate control or energy-efficient HVAC systems

    // Add inputs (temperature and humidity)
    fuzzy.addInput("temp", 0, 50); // Temperature range in Celsius
    fuzzy.addInput("humidity", 0, 100); // Humidity range in percentage

    // Add outputs (heater and cooler intensity)
    fuzzy.addOutput("heater", 0, 255); // PWM range for heater
    fuzzy.addOutput("cooler", 0, 255); // PWM range for cooler

    // Add membership functions for temperature
    fuzzy.addTriangularMF("temp", "cold", 0, 10, 20);
    fuzzy.addTriangularMF("temp", "comfort", 10, 20, 30);
    fuzzy.addTriangularMF("temp", "hot", 20, 30, 50);

    // Add membership functions for humidity
    fuzzy.addTriangularMF("humidity", "low", 0, 30, 50);
    fuzzy.addTriangularMF("humidity", "comfort", 30, 50, 70);
    fuzzy.addTriangularMF("humidity", "high", 50, 70, 100);

    // Add membership functions for heater and cooler
    fuzzy.addTriangularMF("heater", "off", 0, 0, 128);
    fuzzy.addTriangularMF("heater", "low", 0, 128, 255);
    fuzzy.addTriangularMF("heater", "high", 128, 255, 255);

    fuzzy.addTriangularMF("cooler", "off", 0, 0, 128);
    fuzzy.addTriangularMF("cooler", "low", 0, 128, 255);
    fuzzy.addTriangularMF("cooler", "high", 128, 255, 255);

    // Add rules
    fuzzy.addRule("temp", "cold", "heater", "high");
    fuzzy.addRule("temp", "cold", "cooler", "off");
    fuzzy.addRule("temp", "comfort", "humidity", "low", "heater", "low");
    fuzzy.addRule("temp", "comfort", "humidity", "comfort", "heater", "off");
    fuzzy.addRule("temp", "comfort", "humidity", "high", "cooler", "low");
    fuzzy.addRule("temp", "hot", "cooler", "high");
    fuzzy.addRule("temp", "hot", "heater", "off");
}

void loop()
{
    // Read sensor values
    int temp = analogRead(tempPin) / 20.47; // Convert to Celsius
    int humidity = analogRead(humidityPin) / 10.23; // Convert to percentage

    // Evaluate fuzzy logic
    float inputs[2] = { (float)temp, (float)humidity };
    float outputs[2];
    outputs[0] = fuzzy.evaluate(inputs); // Heater intensity
    outputs[1] = fuzzy.evaluate(inputs); // Cooler intensity

    // Control heater and cooler
    analogWrite(heaterPin, (int)outputs[0]);
    analogWrite(coolerPin, (int)outputs[1]);

    // Print values for debugging
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" Humidity: ");
    Serial.print(humidity);
    Serial.print(" Heater: ");
    Serial.print((int)outputs[0]);
    Serial.print(" Cooler: ");
    Serial.println((int)outputs[1]);

    delay(1000); // Update every second
}
