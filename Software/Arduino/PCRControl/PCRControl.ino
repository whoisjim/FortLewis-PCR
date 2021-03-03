
#include "TemperatureSensor.hpp"
#include "PID.hpp"
#include "RampPID.hpp"
#include "HardcodedThreeStepPID.hpp"

#include <math.h>

// version is year.month.date.revision
#define SOFTWARE_VERSION "2021.3.2.1"

const int inA = 13; // pin connected to INA on VHN5019
const int inB = 12; // pin connected to INB on VHN5019
const int ppwm = 11; // pin connected to PWM on VHN5019
const int fpwm = 10; // pin connected to PWM on fan
const int thermP = A0; // pin connected to block thermal resistor neetwork. see elegooThermalResistorSch.png
const int LidP = A1; // pin for thermal resistor conneccted to lid
const int cPin = A2; // for curent recording
const int ssr = 9; // solid state relay signal

bool pPower = false; // software pielter on/off
bool lPower = false; // software lid on/off

bool verboseState = false; // spam serial with state every loop?
bool verbosePID = false;  // spam serial with target and curent temperature?

double peltierPWM = 0; // the PWM signal * curent direction to be sent to curent drivers for peltier
int limitPWMH = 255;
int limitPWMC = 255;

float avgPTemp = 0; // last average for peltier temperature
int avgPTempSampleSize = 100; // sample size for peltier temperature moving average

float avgPPWM = 0; // last average for peltier temperature
int avgPPWMSampleSize = 2; // sample size for peltier temperature moving average was 4

double targetPeltierTemp = 29; // the tempature the system will try to move to, in degrees C
double currentPeltierTemp; // the tempature curently read from the thermoristor connected to thermP, in degrees C

double currentLidTemp; 
double LastLidTemp;

// setup pieltier tempature sensor
TemperatureSensor peltierT(thermP);
TemperatureSensor LidT(LidP); // JD setup for thermo resistor temp 

// setup pieltier PID
PID peltierPID(15, 0.1, 10000);

void setup() {
  // setup serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // setup pins
  pinMode(inA, OUTPUT);
  pinMode(inB, OUTPUT);
  pinMode(ppwm, OUTPUT);
  pinMode(fpwm, OUTPUT);
  pinMode(thermP, INPUT);
  pinMode(LidP, INPUT);
  pinMode(ssr, OUTPUT);
  // set initial pin state to off
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);
  analogWrite(ppwm, 0);
  analogWrite(fpwm, 0);
  digitalWrite(ssr,LOW);
}

// checks UART serial for any commands and executes them
void handleSerialInput() {
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();
    if (incomingCommand == "whoami\n") { // print out software ID
      Serial.print("FLC-PCR software version: ");
      Serial.print(SOFTWARE_VERSION);
      Serial.print("\n");
    }
    if (incomingCommand == "verbose\n") { // toggle sending curent temp, pwm and current lid temp every loop
      if (verboseState) {
        verboseState = false;
      } else {
        verboseState = true;
      }
    }
    if (incomingCommand == "pid\n") { // toggle sending target temp, curent temp and pwm every loop
      if (verbosePID) {
        verbosePID = false;
      } else {
        verbosePID = true;
      }
    }
    if (incomingCommand == "d\n") { // request a single sample of the curent temp, pwm and lid temp
      // request system data
      Serial.print(avgPTemp);
      Serial.print(" ");
      Serial.print(avgPPWM);
      Serial.print(" ");
      Serial.print(currentLidTemp);
      Serial.print("\n");
    }
    if (incomingCommand == "state\n") { // check weather or not the pieltiers are on or off
      Serial.print(pPower);
      Serial.print("\n");
    }
    if (incomingCommand == "offl\n") { // turn off lid
      lPower = false;
    } else if (incomingCommand == "offp\n") { // turn off pieltiers
      pPower = false;
    } else if (incomingCommand == "off\n") { // turn off both lid and pieltiers
      pPower = false;
      lPower = false;
    }
    if (incomingCommand == "onl\n") { // turn on the lid
      lPower = true;
    } else if (incomingCommand == "onp\n") { // turn on the pieltiers
      pPower = true;
      peltierPID.reset();
    } else if (incomingCommand == "on\n") { // turn on both lid and pieltiers
      peltierPID.reset();
      pPower = true;
      lPower = true;
    }
    if (incomingCommand.substring(0,2) == "kp") { // set proportional gain
      peltierPID.setKp(incomingCommand.substring(2).toFloat());
    }
    if (incomingCommand.substring(0,2) == "ki") { // set integral gain
      peltierPID.setKi(incomingCommand.substring(2).toFloat());
    }
    if (incomingCommand.substring(0,2) == "kd") { // set derivitive gain
      peltierPID.setKd(incomingCommand.substring(2).toFloat());
    }
    if (incomingCommand.substring(0,2) == "pt") { // set pieltier temperature
      peltierPID.reset();
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,3) == "pia") { // set size of pieltier temperature low pass filter
      avgPTempSampleSize = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "poa") { // set size of pwm low pass filter
      avgPPWMSampleSize = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "plc") { // set the max pwm for coling
      limitPWMC = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "plh") { // set the max pwm for heating
      limitPWMH = incomingCommand.substring(3).toInt();
    }
  }
}

void loop() {
  handleSerialInput();

  currentLidTemp = LidT.getTemp(); // read lid temp
  currentPeltierTemp = peltierT.getTemp(); // read pieltier temp
  if (isnan(currentPeltierTemp) || isinf(currentPeltierTemp)) { // reset nan and inf values
    currentPeltierTemp = avgPTemp;
  }

  // old temperature calibration
  //currentPeltierTemp = 0.9090590064070043 * currentPeltierTemp + 3.725848396176527; // estimate vial temperature
  //currentPeltierTemp = 0.6075525829531135 * currentPeltierTemp + 15.615801552818361; // seccond estimate

  // 3/2/2021 temperature calabration
  currentPeltierTemp = 1.1201 * currentPeltierTemp - 3.32051;
  
  avgPTemp = ((avgPTempSampleSize - 1) * avgPTemp + currentPeltierTemp) / avgPTempSampleSize; // average
  peltierPWM = peltierPID.calculate(avgPTemp, targetPeltierTemp); // calculate pid and set to output
  peltierPWM = min(limitPWMH, max(-limitPWMC, peltierPWM)); // clamp output between -255 and 255
  if (isnan(peltierPWM ) || isinf(peltierPWM )) { // reset nan and inf values
    peltierPWM = avgPPWM;
  }
  avgPPWM = ((avgPPWMSampleSize - 1) * avgPPWM + peltierPWM) / avgPPWMSampleSize; // average

  // print out verbose data to serial if set
  if (verboseState) {
    Serial.print(avgPTemp);
    Serial.print(" ");
    Serial.print(avgPPWM);
    Serial.print(" ");
    Serial.print(currentLidTemp);
    Serial.print("\n");
  }
  if (verbosePID) {
    Serial.print(avgPTemp);
    Serial.print(" ");
    Serial.print(targetPeltierTemp);
    Serial.print(" ");
    Serial.print(avgPPWM);
    Serial.print("\n");
  }

  // lid controll
  if (lPower) {
    if(currentLidTemp < 90){ 
      digitalWrite(ssr, HIGH);
    } else {
      digitalWrite(ssr, LOW);
    }
  } else {
    digitalWrite(ssr, LOW);
  }
  
  // pieltier control
  if (!pPower || currentPeltierTemp > 150) { // pieltiers on, shut down if over 150C
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(fpwm, 225);
    analogWrite(ppwm, 0);

    peltierPID.reset();
    return;
  } else { // pieltiers on
    // convert pieltierDelta to pwm, inA, inB
    analogWrite(ppwm, abs(peltierPWM));
    analogWrite(fpwm, 255);
    if (peltierPWM > 0) {
      digitalWrite(inA, HIGH);
      digitalWrite(inB, LOW);
    } else {
      digitalWrite(inA, LOW);
      digitalWrite(inB, HIGH);
    }
  }
}
