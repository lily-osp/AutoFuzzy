#include <AutoFuzzy.h>

AutoFuzzy fuzzy;

// Pin definitions
const int potPin = A0; // Potentiometer connected to A0
const int ledPin = 9; // LED connected to PWM pin 9

void setup()
{
    Serial.begin(9600);

    // Simple Example: LED Brightness Control based on Potentiometer
    // Controls LED brightness based on potentiometer input
    // - Single input (potentiometer) and single output (LED brightness)
    // - Three basic membership functions for each variable
    // - Simple rules for brightness control
    // - Ideal for beginners learning fuzzy logic
    // - Perfect for dimmable lighting or user-controlled brightness

    // Add input (potentiometer value) and output (LED brightness)
    fuzzy.addInput("potValue", 0, 1023); // Potentiometer range
    fuzzy.addOutput("ledBrightness", 0, 255); // PWM range for LED

    // Add membership functions for input
    fuzzy.addTriangularMF("potValue", "low", 0, 255, 511);
    fuzzy.addTriangularMF("potValue", "medium", 255, 511, 767);
    fuzzy.addTriangularMF("potValue", "high", 511, 767, 1023);

    // Add membership functions for output
    fuzzy.addTriangularMF("ledBrightness", "dim", 0, 64, 128);
    fuzzy.addTriangularMF("ledBrightness", "medium", 64, 128, 192);
    fuzzy.addTriangularMF("ledBrightness", "bright", 128, 192, 255);

    // Add rules
    fuzzy.addRule("potValue", "low", "ledBrightness", "dim");
    fuzzy.addRule("potValue", "medium", "ledBrightness", "medium");
    fuzzy.addRule("potValue", "high", "ledBrightness", "bright");
}

void loop()
{
    // Read potentiometer value
    int potValue = analogRead(potPin);

    // Evaluate fuzzy logic
    float ledBrightness = fuzzy.evaluate((float*)&potValue);

    // Set LED brightness
    analogWrite(ledPin, (int)ledBrightness);

    // Print values for debugging
    Serial.print("Potentiometer: ");
    Serial.print(potValue);
    Serial.print(" LED Brightness: ");
    Serial.println((int)ledBrightness);

    delay(100); // Small delay for stability
}
