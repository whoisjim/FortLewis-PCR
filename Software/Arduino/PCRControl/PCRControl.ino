#include "TemperatureSensor.hpp"
#include "PID.hpp"
#include <math.h>

#define SOFTWARE_VERSION "2025.7.7"

// Pin definitions
const int inA = 13;       // Current driver direction pin A
const int inB = 12;       // Current driver direction pin B
const int ppwm = 11;      // Peltier PWM output 
const int fpwm = 10;      // Fan PWM output
const int thermP = A0;    // Thermistor input for Peltier block
const int LidP = A1;      // Thermistor input for heated lid
const int ssr = 9;        // Solid State Relay for lid heater

// System state flags
bool pPower = false;      
bool lPower = false;     
bool verboseState = false;
bool verbosePID = false;  

// PWM and temperature control
double peltierPWM = 0;    
int limitPWMH = 255;   
int limitPWMC = 255;     
int currentFanPWM = 255;  

// Temperature filtering
float avgPTemp = 0;       
float avgPPWM = 0;      
double targetPeltierTemp = 29; 
double currentPeltierTemp;     
double currentLidTemp;        
float alpha = 0.7;        

// Sensors and PID
TemperatureSensor peltierT(thermP);
TemperatureSensor LidT(LidP);
PID peltierPID(0, 0, 0); 

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }  

  // Initialize digital and PWM pins
  pinMode(inA, OUTPUT);
  pinMode(inB, OUTPUT);
  pinMode(ppwm, OUTPUT);
  pinMode(fpwm, OUTPUT);
  pinMode(thermP, INPUT);
  pinMode(LidP, INPUT);
  pinMode(ssr, OUTPUT);

  // Set initial safe state 
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);
  analogWrite(ppwm, 0);
  analogWrite(fpwm, 0);
  digitalWrite(ssr, LOW);

  // Setup PID constraints
  peltierPID.setOutputLimits(-200, 200); // Protects Pelteirs from excess use
  peltierPID.setIntegratorLimit(50);     
}

void handleSerialInput() {
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();

    // Identify device/version
    if (incomingCommand == "whoami\n") {
      Serial.print("FLC-PCR software version: ");
      Serial.print(SOFTWARE_VERSION);
      Serial.print("\n");
    }
    // Toggle telemetry modes
    else if (incomingCommand == "verbose\n") {
      verboseState = !verboseState;
    }
    else if (incomingCommand == "pid\n") {
      verbosePID = !verbosePID;
    }
    
    else if (incomingCommand == "d\n") {
      Serial.print(avgPTemp);
      Serial.print(" ");
      Serial.print(avgPPWM);
      Serial.print(" ");
      Serial.print(currentLidTemp);
      Serial.print("\n");
    }
    // Report power status
    else if (incomingCommand == "state\n") {
      Serial.print(pPower ? "ON" : "OFF");
      Serial.print("\n");
    }
    // Power control commands
    else if (incomingCommand == "offl\n") {
      lPower = false;
    }
    else if (incomingCommand == "offp\n") {
      pPower = false;
    }
    else if (incomingCommand == "off\n") {
      pPower = false;
      lPower = false;
    }
    else if (incomingCommand == "onl\n") {
      lPower = true;
    }
    else if (incomingCommand == "onp\n") {
      pPower = true;
      peltierPID.reset(); 
    }
    else if (incomingCommand == "on\n") {
      pPower = true;
      lPower = true;
      peltierPID.reset();
    }
    // Commands coming from SBC
    else if (incomingCommand.substring(0, 2) == "kp") {
      peltierPID.setKp(incomingCommand.substring(2).toFloat());
    }
    else if (incomingCommand.substring(0, 2) == "ki") {
      peltierPID.setKi(incomingCommand.substring(2).toFloat());
    }
    else if (incomingCommand.substring(0, 2) == "kd") {
      peltierPID.setKd(incomingCommand.substring(2).toFloat());
    }
    else if (incomingCommand.substring(0, 2) == "pt") {
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
      peltierPID.reset(); 
    }
    else if (incomingCommand.substring(0, 3) == "plc") {
      limitPWMC = incomingCommand.substring(3).toInt();
    }
    else if (incomingCommand.substring(0, 3) == "plh") {
      limitPWMH = incomingCommand.substring(3).toInt();
    }
  }
}

void loop() {
  handleSerialInput();

  currentLidTemp = LidT.getTemp(115);
  currentPeltierTemp = peltierT.getTemp(targetPeltierTemp);

  if (isnan(currentPeltierTemp) || isinf(currentPeltierTemp)) {
    currentPeltierTemp = avgPTemp;
  }

  // Filter out signal noise
  avgPTemp = alpha * currentPeltierTemp + (1 - alpha) * avgPTemp;

  // PID Computation Loop
  static unsigned long lastPIDTime = 0;
  if (millis() - lastPIDTime >= 150) {
    double error = targetPeltierTemp - avgPTemp;

    // Gain Scheduling: Adjust PID constants based on target and error
    if (error < -2.0) {  // Cooling mode
      peltierPID.setKp(8.0);
      peltierPID.setKi(0.001);
      peltierPID.setKd(35.0);
    }
    else if (error < 0) {  // Fine tuning cooling 
      peltierPID.setKp(12.0);
      peltierPID.setKi(0.6);
      peltierPID.setKd(35.0);
    }
    else { 
      if (targetPeltierTemp >= 90) { 
        peltierPID.setKp(38);
        peltierPID.setKi(1.5);
        peltierPID.setKd(20);
        peltierPID.setIntegratorLimit(40);
      }
      else if (targetPeltierTemp >= 70) { 
        peltierPID.setKp(27);
        peltierPID.setKi(0.9);
        peltierPID.setKd(37);
        peltierPID.setIntegratorLimit(50);
      }
      else {
        peltierPID.setKp(21);
        peltierPID.setKi(0.6);
        peltierPID.setKd(41);
        peltierPID.setIntegratorLimit(50);
      }
    }
    peltierPWM = peltierPID.calculate(avgPTemp, targetPeltierTemp);
    avgPPWM = peltierPWM;
    lastPIDTime = millis();
  }

  peltierPWM = constrain(peltierPWM, -limitPWMC, limitPWMH);
  if (isnan(peltierPWM) || isinf(peltierPWM)) {
    peltierPWM = avgPPWM;
  }

  // Hardware control
  if (pPower && currentPeltierTemp <= 150) { 
    
    // Lower fan speed at high heat 
    if (avgPTemp >= 90 || targetPeltierTemp >= 90) {
      currentFanPWM = 100;  
    }
    else {
      currentFanPWM = 255; 
    }

    analogWrite(fpwm, currentFanPWM);
    analogWrite(ppwm, abs((int)peltierPWM)); 

    // Current driver Control
    if (peltierPWM > 0) { 
      digitalWrite(inA, HIGH);
      digitalWrite(inB, LOW);
    } else {              
      digitalWrite(inA, LOW);
      digitalWrite(inB, HIGH);
    }
  }
  else {
    // Safety shutdown
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(ppwm, 0);
    analogWrite(fpwm, 0);
    if (!pPower) {
      peltierPID.reset();
    }
  }

  // Solid state relay for heated lid
  if (lPower) {
    digitalWrite(ssr, (currentLidTemp < 115) ? HIGH : LOW); // Setting Lid Temperature
  } else {
    digitalWrite(ssr, LOW);
  }

  // UI/Serial update loop
  static unsigned long lastUIUpdateTime = 0;
  if (millis() - lastUIUpdateTime >= 100) {
    if (verboseState) {
      Serial.print(currentPeltierTemp);
      Serial.print(" ");
      Serial.print(peltierPWM);
      Serial.print(" ");
      Serial.print(currentLidTemp);
      Serial.print(" ");
      Serial.print(currentFanPWM);
      Serial.print("\n");
    }
    if (verbosePID) {
      Serial.print(avgPTemp);
      Serial.print(" ");
      Serial.print(targetPeltierTemp);
      Serial.print(" ");
      Serial.print(peltierPWM);
      Serial.print("\n");
    }
    lastUIUpdateTime = millis();
  }
}
