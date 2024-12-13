// Advanced Example: HVAC Control System
// Controls heating/cooling and humidity based on multiple sensors
// Advanced Example: HVAC Control System
// - Multiple inputs and outputs (temperature, humidity, CO2, energy rate)
// - Complex state management
// - Advanced features:
//  - Occupancy-based target adjustment
//  - Energy cost optimization
//  - CO2-based ventilation control
//  - Periodic system optimization
//  - Data logging for system improvement
//  - Soft start/stop for equipment protection
// - Suitable for commercial building automation

#include <AutoFuzzy.h>

AutoFuzzy hvacController;

struct HVACState {
  float targetTemp;
  float targetHumidity;
  float currentTemp;
  float currentHumidity;
  float co2Level;
  float energyRate;
  bool isOccupied;
} state;

const int HEAT_PIN = 3;
const int COOL_PIN = 4;
const int FAN_PIN = 5;
const int HUMID_PIN = 6;
const int DEHUMID_PIN = 7;

void setup() {
  // Setup inputs
  hvacController.addInput("temp_diff", -10, 10);     // Difference from target temp
  hvacController.addInput("humid_diff", -30, 30);    // Difference from target humidity
  hvacController.addInput("co2_level", 0, 2000);     // CO2 PPM
  hvacController.addInput("energy_rate", 0, 100);    // Energy cost rate

  // Setup outputs
  hvacController.addOutput("heat_power", 0, 100);    // Heating power %
  hvacController.addOutput("cool_power", 0, 100);    // Cooling power %
  hvacController.addOutput("fan_speed", 0, 100);     // Fan speed %
  hvacController.addOutput("humid_power", 0, 100);   // Humidifier power %

  // Temperature difference membership functions
  hvacController.addTriangularMF("temp_diff", "cold", -10, -5, 0);
  hvacController.addTriangularMF("temp_diff", "okay", -2, 0, 2);
  hvacController.addTriangularMF("temp_diff", "hot", 0, 5, 10);

  // Humidity difference membership functions
  hvacController.addTriangularMF("humid_diff", "dry", -30, -15, 0);
  hvacController.addTriangularMF("humid_diff", "okay", -10, 0, 10);
  hvacController.addTriangularMF("humid_diff", "humid", 0, 15, 30);

  // CO2 level membership functions
  hvacController.addTrapezoidalMF("co2_level", "good", 0, 0, 800, 1000);
  hvacController.addTrapezoidalMF("co2_level", "high", 800, 1000, 1500, 1700);
  hvacController.addTrapezoidalMF("co2_level", "critical", 1500, 1700, 2000, 2000);

  // Energy rate membership functions
  hvacController.addTrapezoidalMF("energy_rate", "low", 0, 0, 30, 40);
  hvacController.addTrapezoidalMF("energy_rate", "medium", 35, 45, 65, 75);
  hvacController.addTrapezoidalMF("energy_rate", "high", 70, 80, 100, 100);

  // Output membership functions
  // ... (Similar pattern for heat_power, cool_power, fan_speed, humid_power)

  // Complex rules for energy-efficient operation
  hvacController.addRule("temp_diff", "cold", "heat_power", "high");
  hvacController.addRule("temp_diff", "hot", "cool_power", "high");
  hvacController.addRule("humid_diff", "dry", "humid_power", "high");
  hvacController.addRule("co2_level", "high", "fan_speed", "high");
  // ... (Many more rules for different combinations)

  // Initial optimization
  hvacController.autoOptimize(500);
}

void updateSensors() {
  state.currentTemp = readTemperatureSensor();
  state.currentHumidity = readHumiditySensor();
  state.co2Level = readCO2Sensor();
  state.energyRate = getCurrentEnergyRate();
  state.isOccupied = readOccupancySensor();
}

void adjustTargets() {
  // Adjust target values based on occupancy and time of day
  if (!state.isOccupied) {
    state.targetTemp = getEcoTemp();
    state.targetHumidity = getEcoHumidity();
  } else {
    state.targetTemp = getComfortTemp();
    state.targetHumidity = getComfortHumidity();
  }
}

void loop() {
  updateSensors();
  adjustTargets();

  float tempDiff = state.currentTemp - state.targetTemp;
  float humidDiff = state.currentHumidity - state.targetHumidity;

  float inputs[] = {tempDiff, humidDiff, state.co2Level, state.energyRate};

  // Evaluate all outputs
  float heatPower = hvacController.evaluate(inputs);
  float coolPower = hvacController.evaluate(inputs);
  float fanSpeed = hvacController.evaluate(inputs);
  float humidPower = hvacController.evaluate(inputs);

  // Apply controls with soft start/stop
  adjustHeatPower(heatPower);
  adjustCoolPower(coolPower);
  adjustFanSpeed(fanSpeed);
  adjustHumidifier(humidPower);

  // Log data for optimization
  logSystemData(inputs, heatPower, coolPower, fanSpeed, humidPower);

  // Periodic optimization based on logged data
  static unsigned long lastOptimize = 0;
  if (millis() - lastOptimize > 86400000) {  // Daily optimization
    hvacController.autoOptimize(500);
    lastOptimize = millis();
  }

  delay(30000);  // Update every 30 seconds
}
