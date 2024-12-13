// Simple Example: LED Brightness Control based on Light Sensor
// Controls LED brightness based on ambient light level
// Simple Example: LED Brightness Control
// - Single input (light sensor) and single output (LED)
// - Three basic membership functions for each variable
// - Simple rules for brightness control
// - Ideal for beginners learning fuzzy logic
// - Perfect for mood lighting or automatic night lights

#include <AutoFuzzy.h>

AutoFuzzy lightController;

void setup() {
  // Setup input/output
  lightController.addInput("ambient_light", 0, 1023);  // Analog input range
  lightController.addOutput("led_brightness", 0, 255);  // PWM output range

  // Define membership functions for ambient light
  lightController.addTriangularMF("ambient_light", "dark", 0, 0, 400);
  lightController.addTriangularMF("ambient_light", "medium", 300, 500, 700);
  lightController.addTriangularMF("ambient_light", "bright", 600, 1023, 1023);

  // Define membership functions for LED brightness
  lightController.addTriangularMF("led_brightness", "high", 170, 255, 255);
  lightController.addTriangularMF("led_brightness", "medium", 85, 127, 170);
  lightController.addTriangularMF("led_brightness", "low", 0, 0, 85);

  // Define rules
  lightController.addRule("ambient_light", "dark", "led_brightness", "high");
  lightController.addRule("ambient_light", "medium", "led_brightness", "medium");
  lightController.addRule("ambient_light", "bright", "led_brightness", "low");
}

void loop() {
  float lightLevel = analogRead(A0);
  float ledBrightness = lightController.evaluate(&lightLevel);
  analogWrite(9, ledBrightness);
  delay(100);
}
