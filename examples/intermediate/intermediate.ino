// Intermediate Example: Plant Watering System
// Controls water pump based on soil moisture and temperature
// Intermediate Example: Plant Watering System
// - Two inputs (moisture and temperature) and one output (pump duration)
// - Uses both triangular and trapezoidal membership functions
// - More complex rule set considering multiple conditions
// - Includes system optimization
// - Practical for home automation and indoor gardens
// - Features safety thresholds and timing controls

#include <AutoFuzzy.h>

AutoFuzzy waterController;
const int PUMP_PIN = 3;
const int MOISTURE_PIN = A0;
const int TEMP_PIN = A1;

void setup() {
  // Setup inputs and output
  waterController.addInput("moisture", 0, 100);      // Moisture percentage
  waterController.addInput("temperature", 0, 50);    // Temperature in Celsius
  waterController.addOutput("pump_duration", 0, 30); // Watering duration in seconds

  // Moisture membership functions
  waterController.addTrapezoidalMF("moisture", "dry", 0, 0, 20, 30);
  waterController.addTrapezoidalMF("moisture", "moist", 25, 40, 60, 75);
  waterController.addTrapezoidalMF("moisture", "wet", 70, 80, 100, 100);

  // Temperature membership functions
  waterController.addTrapezoidalMF("temperature", "cool", 0, 0, 15, 20);
  waterController.addTrapezoidalMF("temperature", "moderate", 18, 22, 28, 32);
  waterController.addTrapezoidalMF("temperature", "hot", 30, 35, 50, 50);

  // Pump duration membership functions
  waterController.addTriangularMF("pump_duration", "short", 0, 0, 10);
  waterController.addTriangularMF("pump_duration", "medium", 8, 15, 22);
  waterController.addTriangularMF("pump_duration", "long", 20, 30, 30);

  // Rules
  waterController.addRule("moisture", "dry", "pump_duration", "long");
  waterController.addRule("moisture", "moist", "pump_duration", "medium");
  waterController.addRule("moisture", "wet", "pump_duration", "short");
  waterController.addRule("temperature", "hot", "pump_duration", "long");
  waterController.addRule("temperature", "moderate", "pump_duration", "medium");
  waterController.addRule("temperature", "cool", "pump_duration", "short");

  // Optimize the system based on historical data
  waterController.autoOptimize(200);
}

void loop() {
  float moistureLevel = map(analogRead(MOISTURE_PIN), 0, 1023, 0, 100);
  float temperature = map(analogRead(TEMP_PIN), 0, 1023, 0, 50);
  float inputs[] = {moistureLevel, temperature};

  float pumpDuration = waterController.evaluate(inputs);

  if (pumpDuration > 5) {  // Minimum threshold to avoid frequent short bursts
    digitalWrite(PUMP_PIN, HIGH);
    delay(pumpDuration * 1000);
    digitalWrite(PUMP_PIN, LOW);
  }

  delay(300000);  // Check every 5 minutes
}
