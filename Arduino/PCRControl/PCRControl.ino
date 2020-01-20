#include <math.h>
const int inA = 13; // pin connected to INA on VHN5019
const int inB = 12; // pin connected to INB on VHN5019
const int ppwm = 11; // pin connected to PWM on VHN5019
const int fpwm = 10; // pin connected to PWM on fan
const int thermP = A0; // pin connected to block thermal resistor neetwork. see elegooThermalResistorSch.png
const int LidP = A1; // pin for thermal resistor conneccted to lid
const int cPin = A2; // for curent recording
const int ssr = 9; // solid state relay signal

bool power = false; // software on/off

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

class dualPid {
  private:
  double ikp; // higher moves faster
  double iki; // higher fixes ofset and faster
  double ikd; // higher settes faster but creastes ofset
  double dkp; // higher moves faster
  double dki; // higher fixes ofset and faster
  double dkd; // higher settes faster but creastes ofset
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
  double iErrorLimit = 127;
  
  public:
  dualPid (double proportionalGainI = 1, double integralGainI = 0, double derivativeGainI = 0, double proportionalGainD = 1, double integralGainD = 0, double derivativeGainD = 0) {
    ikp = proportionalGainI; // higher moves faster
    iki = integralGainI; // higher fixes ofset and faster
    ikd = derivativeGainI; // higher settes faster but creastes ofset and amplifies noise
    dkp = proportionalGainD; // higher moves faster
    dki = integralGainD; // higher fixes ofset and faster
    dkd = derivativeGainD; // higher settes faster but creastes ofset and amplifies noise
  }

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
    if (70 > targetTemp) { // good
      return 19 * pError + 0.2 * iError + 10000000 * dError;
    } else if (80 > targetTemp) {
      return 9 * pError + 0.17 * iError + 10000000 * dError;
    } else {
      return 9 * pError + 0.40 * iError + 10000000 * dError;
    }

    /*
    if (70 > targetTemp) { // low
      return 30 * pError + 0.75 * iError + 100000 * dError;
    } else if (80 > targetTemp) { // med
      return 5 * pError + 0.25 * iError + 10000000 * dError;
    } else { // high
      return 5 * pError + 0.5 * iError + 10000000 * dError;
    }


    //(10, 0.5, 10000000, 40, 0.75, 0)
    */
  }
  
  void setIkp(float value) {
    ikp = value;
  }
  
  void setIki(float value) {
    iki = value;
  }
  
  void setIkd(float value) {
    ikd = value;
  }

  void setDkp(float value) {
    dkp = value;
  }
  
  void setDki(float value) {
    dki = value;
  }
  
  void setDkd(float value) {
    dkd = value;
  }
};

// for creating a tempature sensor
// includes noise reduction
// call resetTemp() before a series of getTemp() calls
// see elegooThermalResistorSch.png for wireing
class TempSensor {
  private:
  int pin;
  double temp;
  double lastK;
  
  public:
  TempSensor(int iPin) { 
    pin = iPin; // the pin that conected to the tempature network
  }
  
  void resetTemp() { // resets saved privois tempature, nessasary if it has been a while since last getTemp()
    int tempReading = analogRead(pin);
    double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    lastK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );
  }

  double getTemp() { // returns the tempature from thermoristor connected to thermP in degrees C, includes noise reduction
    static double spike0u;
    int tempReading = analogRead(pin);
    double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK ); // kelvin

    //Serial.print(tempK - 273.15);
    //Serial.print(" ");
    
    // noise peak removal
    
    if (tempK - lastK > 0.5 + abs(lastK -299.15) * 0.1) {
      if (spike0u == 0) {
        spike0u = tempK - lastK;
      }
      tempK -= spike0u;
    } else {
      spike0u = 0;
    }

    if (tempK - lastK < -10) {
      tempK += spike0u;
      spike0u = 0;
    }
    
    lastK = tempK;
    
    return (tempK - 273.15); // convert kelvin to celcius
  }
};

// setup pieltier tempature sensor
TempSensor peltierT(thermP);
TempSensor LidT(LidP); // JD setup for thermo resistor temp 

// setup pieltier PID
dualPid peltierPID(10, 0.5, 10000000, 40, 0.75, 0); // 5, 2, 4000, 8, 2, 4000 data43

void setup() {
  // setup serial
  Serial.begin(9600);
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
  // commands
  // off
  //   power off
  // on
  //   power on
  // pt[floatValue]
  //   sets targetPeltierTemp to floatValue
  // pk[p,d,i][value]
  //   sets pid constants for peltier
  // pa[intValue]
  //   sets sample size for peltier moving average
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();
    if (incomingCommand == "off\n") {
      power = false;
    } else if (incomingCommand.substring(0,3) == "off") {
      power = false;
      targetPeltierTemp = incomingCommand.substring(3).toFloat();
    }
    if (incomingCommand == "on\n") {
      peltierPID.reset();
      power = true;
    } else if (incomingCommand.substring(0,2) == "on") {
      peltierPID.reset();
      power = true;
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,2) == "pt") {
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,4) == "pikp") {
      peltierPID.setIkp(incomingCommand.substring(4).toFloat());
    }
    if (incomingCommand.substring(0,4) == "piki") {
      peltierPID.setIki(incomingCommand.substring(4).toFloat());
    }
    if (incomingCommand.substring(0,4) == "pikd") {
      peltierPID.setIkd(incomingCommand.substring(4).toFloat());
    }
    if (incomingCommand.substring(0,4) == "pdkp") {
      peltierPID.setDkp(incomingCommand.substring(4).toFloat());
    }
    if (incomingCommand.substring(0,4) == "pdki") {
      peltierPID.setDki(incomingCommand.substring(4).toFloat());
    }
    if (incomingCommand.substring(0,4) == "pdkd") {
      peltierPID.setDkd(incomingCommand.substring(4).toFloat());
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

  LidT.resetTemp();
  currentLidTemp = LidT.getTemp(); // read lid temp
  peltierT.resetTemp();
  currentPeltierTemp = peltierT.getTemp(); // read pieltier temp

  currentPeltierTemp = 0.9090590064070043 * currentPeltierTemp + 3.725848396176527; // estimate vial temperature
  
  avgPTemp = ((avgPTempSampleSize - 1) * avgPTemp + currentPeltierTemp) / avgPTempSampleSize; // average input with the last 9 inputs
  peltierPWM = peltierPID.calculate(avgPTemp, targetPeltierTemp); // calculate pid and set to output
  peltierPWM = min(limitPWMH, max(-limitPWMC, peltierPWM)); // clamp output between -255 and 255

  // Lid control
  if(currentLidTemp < 90){
    digitalWrite(ssr, HIGH);
  } else {
    digitalWrite(ssr, LOW);
  }

  avgPPWM = ((avgPPWMSampleSize - 1) * avgPPWM + peltierPWM) / avgPPWMSampleSize; // average input with the last 9 inputs
  
  if (!power || currentPeltierTemp > 150) {// shut off system if over 150 degrees for safety
    // for graphing system state
    Serial.print(avgPTemp);
    Serial.print(" 0 ");
    Serial.print(analogRead(A1) * 0.065168);
    Serial.print(" ");
    Serial.print(currentLidTemp);
    Serial.print("\n");
    
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(fpwm, 1024);
    analogWrite(ppwm, 0);
    //digitalWrite(ssr, LOW);

    peltierPID.reset();
    peltierT.resetTemp();
    avgPPWM = peltierPWM; // fixes nan error
    return;
  }

  // for recording system state
  Serial.print(avgPTemp);
  Serial.print(" ");
  Serial.print(avgPPWM);
  Serial.print(" ");
  Serial.print(analogRead(cPin) * 0.065168);
  Serial.print(" ");
  Serial.print(currentLidTemp);
  Serial.print("\n");

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
