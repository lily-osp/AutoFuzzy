#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

// Pin definitions
const int moisturePin = A0; // Soil moisture sensor
const int tempPin = A1; // Temperature sensor
const int pumpPin = 8; // Relay to control water pump

void setup()
{
    Serial.begin(9600);

    // Intermediate Example: Plant Watering System
    // Automates plant watering based on soil moisture and temperature
    // - Two inputs (soil moisture and temperature) and one output (pump control)
    // - Three membership functions for each input and output
    // - Rules consider both moisture and temperature for decision-making
    // - Ideal for intermediate users exploring multi-input fuzzy systems
    // - Perfect for smart gardening or automated plant care

    // Add inputs (soil moisture and temperature)
    fuzzy.addInput("moisture", 0, 1023); // Moisture sensor range
    fuzzy.addInput("temp", 0, 50); // Temperature range in Celsius

    // Add output (pump control: 0 = off, 1 = on)
    fuzzy.addOutput("pump", 0, 1);

    // Add membership functions for moisture
    fuzzy.addTriangularMF("moisture", "dry", 0, 300, 500);
    fuzzy.addTriangularMF("moisture", "moist", 300, 500, 700);
    fuzzy.addTriangularMF("moisture", "wet", 500, 700, 1023);

    // Add membership functions for temperature
    fuzzy.addTriangularMF("temp", "cold", 0, 10, 20);
    fuzzy.addTriangularMF("temp", "warm", 10, 20, 30);
    fuzzy.addTriangularMF("temp", "hot", 20, 30, 50);

    // Add membership functions for pump
    fuzzy.addTriangularMF("pump", "off", 0, 0, 0.5);
    fuzzy.addTriangularMF("pump", "on", 0.5, 1, 1);

    // Add rules
    fuzzy.addRule("moisture", "dry", "pump", "on");
    fuzzy.addRule("moisture", "moist", "temp", "cold", "pump", "off");
    fuzzy.addRule("moisture", "moist", "temp", "warm", "pump", "on");
    fuzzy.addRule("moisture", "moist", "temp", "hot", "pump", "on");
    fuzzy.addRule("moisture", "wet", "pump", "off");
}

void loop()
{
    // Read sensor values
    int moisture = analogRead(moisturePin);
    int temp = analogRead(tempPin) / 20.47; // Convert to Celsius (assuming 10-bit ADC)

    // Evaluate fuzzy logic
    float inputs[2] = { (float)moisture, (float)temp };
    float pumpState = fuzzy.evaluate(inputs);

    // Control the pump
    digitalWrite(pumpPin, pumpState > 0.5 ? HIGH : LOW);

    // Print values for debugging
    Serial.print("Moisture: ");
    Serial.print(moisture);
    Serial.print(" Temp: ");
    Serial.print(temp);
    Serial.print(" Pump: ");
    Serial.println(pumpState > 0.5 ? "ON" : "OFF");

    delay(1000); // Check every second
}
