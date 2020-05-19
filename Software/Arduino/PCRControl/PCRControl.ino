#include <math.h>
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

double peltierPWM = 0; // the PWM signal * curent direction to be sent to curent drivers for peltier
int limitPWMH = 255;
int limitPWMC = 255;

float avgPTemp = 0; // last average for peltier temperature
int avgPTempSampleSize = 10; // sample size for peltier temperature moving average

float avgPPWM = 0; // last average for peltier temperature
int avgPPWMSampleSize = 4; // sample size for peltier temperature moving average

double targetPeltierTemp = 40; // the tempature the system will try to move to, in degrees C
double currentPeltierTemp; // the tempature curently read from the thermoristor connected to thermP, in degrees C

double currentLidTemp; 
double LastLidTemp;

// class for creating a pid system
class pid {
  private:
  double kp; // higher moves faster
  double ki; // higher fixes ofset and faster
  double kd; // higher settes faster but creastes ofset
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
  double iErrorLimit = 255;
  
  public:
  pid (double proportionalGain = 1, double integralGain = 0, double derivativeGain = 0) {
    kp = proportionalGain; // higher moves faster
    ki = integralGain; // higher fixes ofset and faster
    kd = derivativeGain; // higher settes faster but creastes ofset and amplifies noise
  }

  void reset(){
    iError = 0;
  }
  
  double calculate(double currentTemp, double targetTemp) { // performs pid calculation returns error
    lastTime = currentTime;
    currentTime = millis();
    lError = pError;
    pError = targetTemp - currentTemp;
    iError = min(max(pError * (double)(currentTime - lastTime) / 1000 + iError, -iErrorLimit), iErrorLimit);
    dError = (pError - lError) / (double)(currentTime - lastTime) / 1000;
    return kp * pError + ki * iError + kd * dError;
  }
  
  void setKp(float value) {
    kp = value;
  }
  
  void setKi(float value) {
    ki = value;
  }
  
  void setKd(float value) {
    kd = value;
  }
};

class rampPid {
  private:
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
  double iErrorLimit = 127;
  
  public:
  void reset(){
    iError = 0;
  }
  
  double calculate(double currentTemp, double targetTemp) { // performs pid calculation
    lastTime = currentTime;
    currentTime = millis();
    lError = pError;
    pError = targetTemp - currentTemp;
    iError = min(max(pError * (double)(currentTime - lastTime) / 1000 + iError, -iErrorLimit), iErrorLimit);
    dError = (pError - lError) / (double)(currentTime - lastTime) / 1000;
    //return (289 / 6 - (5 * targetTemp) / 12) * pError + (-0.383333 + 0.00833333 * targetTemp) * iError + 10000000 * dError;
    if (70 > targetTemp) {
      return 19 * pError + 0.2 * iError + 10000000 * dError;
    } else if (80 > targetTemp) {
      return 9 * pError + 0.17 * iError + 10000000 * dError;
    } else {
      return 9 * pError + 0.40 * iError + 10000000 * dError;
    }
  }
};

// for creating a tempature sensor
// includes noise reduction
// call resetTemp() before a series of getTemp() calls
// see elegooThermalResistorSch.png for wireing
class TempSensor {
  private:
  int pin;
  
  public:
  TempSensor(int iPin) { 
    pin = iPin; // the pin that conected to the tempature network
  }

  double getTemp() { // returns the tempature from thermoristor connected to thermP in degrees C, includes noise reduction
    int tempReading = analogRead(pin);
    double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK ); // kelvin
    return (tempK - 273.15); // convert kelvin to celcius
  }
};

// setup pieltier tempature sensor
TempSensor peltierT(thermP);
TempSensor LidT(LidP); // JD setup for thermo resistor temp 

// setup pieltier PID
rampPid peltierPID;

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

void loop() {
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();
    if (incomingCommand == "d\n") {
      // requast system data
      Serial.print(avgPTemp);
      Serial.print(" ");
      Serial.print(avgPPWM);
      Serial.print(" ");
      Serial.print(currentLidTemp);
      Serial.print("\n");
    }
    if (incomingCommand == "state\n") {
      Serial.print(pPower);
      Serial.print("\n");
    }
    if (incomingCommand == "offl\n") {
      lPower = false;
    } else if (incomingCommand == "offp\n") {
      pPower = false;
    } else if (incomingCommand == "off\n") {
      pPower = false;
      lPower = false;
    }
    if (incomingCommand == "onl\n") {
      lPower = true;
    } else if (incomingCommand == "onp\n") {
      pPower = true;
    } else if (incomingCommand == "on\n") {
      peltierPID.reset();
      pPower = true;
      lPower = true;
    }
    if (incomingCommand.substring(0,2) == "pt") {
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,3) == "pia") {
      avgPTempSampleSize = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "poa") {
      avgPPWMSampleSize = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "plc") {
      limitPWMC = incomingCommand.substring(3).toInt();
    }
    if (incomingCommand.substring(0,3) == "plh") {
      limitPWMH = incomingCommand.substring(3).toInt();
    }
  }

  currentLidTemp = LidT.getTemp(); // read lid temp
  currentPeltierTemp = peltierT.getTemp(); // read pieltier temp
  if (isnan(currentPeltierTemp) || isinf(currentPeltierTemp)) {
    currentPeltierTemp = avgPTemp;
  }

  currentPeltierTemp = 0.9090590064070043 * currentPeltierTemp + 3.725848396176527; // estimate vial temperature
  currentPeltierTemp = 0.6075525829531135 * currentPeltierTemp + 15.615801552818361; // seccond estimate
  
  avgPTemp = ((avgPTempSampleSize - 1) * avgPTemp + currentPeltierTemp) / avgPTempSampleSize; // average input with the last 9 inputs
  peltierPWM = peltierPID.calculate(avgPTemp, targetPeltierTemp); // calculate pid and set to output
  peltierPWM = min(limitPWMH, max(-limitPWMC, peltierPWM)); // clamp output between -255 and 255
  if (isnan(peltierPWM ) || isinf(peltierPWM )) {
    peltierPWM = avgPPWM;
  }
  avgPPWM = ((avgPPWMSampleSize - 1) * avgPPWM + peltierPWM) / avgPPWMSampleSize; // average input with the last 9 inputs
  
  // Lid control
  if (lPower) {
    if(currentLidTemp < 90){ 
      digitalWrite(ssr, HIGH);
    } else {
      digitalWrite(ssr, LOW);
    }
  } else {
    digitalWrite(ssr, LOW);
  }
  
  if (!pPower || currentPeltierTemp > 150) {// shut off system if over 150 degrees for safety
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(fpwm, 1024);
    analogWrite(ppwm, 0);
    //digitalWrite(ssr, LOW);

    peltierPID.reset();
    // fixes nan error
    avgPTemp = currentPeltierTemp;
    avgPPWM = peltierPWM; 
    return;
  }

  // pieltier controll
  // convert pieltierDelta to pwm, inA, inB, and fan signals
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
