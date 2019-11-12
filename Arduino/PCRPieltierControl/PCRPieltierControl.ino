#include <math.h>
const int inA = 13; // pin connected to INA on VHN5019
const int inB = 12; // pin connected to INB on VHN5019
const int ppwm = 11; // pin connected to PWM on VHN5019
const int fpwm = 10; // pin connected to PWM on fan
const int thermP = A0; // pin connected to thermal resistor neetwork. see elegooThermalResistorSch.png

double pieltierDelta = 0; // the change in degrees celcius of pieltier wanted over the next time step

// for converting pieltierDelta to pwm
const double pwmDivPieltierDelta = 1023.0 / 60.0;

double targetPieltierTemp = 27; // the tempature the system will try to move to, in degrees C
double currentPieltierTemp; // the tempature curently read from the thermoristor connected to thermP, in degrees C

// class for creating a pid system
class pid {
  private:
  double kp; // higher moves faster
  double ki; // higher fixes ofset and faster
  double kd; // higher settes faster but creastes ofset
  double kAmbiant; // experimental, fixes ofset?, may remove. leave at zero if unshure
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
    
  public:
  pid (double proportionalGain = 1, double integralGain = 0, double derivativeGain = 0, double Ambiant = 0) {
    kp = proportionalGain; // higher moves faster
    ki = integralGain; // higher fixes ofset and faster
    kd = derivativeGain; // higher settes faster but creastes ofset
    kAmbiant = Ambiant; // experimental, fixes ofset?, may remove. leave at zero if unshure
  }
  
  double calculate(double currentTemp, double targetTemp){ // gets tempature and performs pid calculation returns error in degrees C
    lastTime = currentTime;
    currentTime = millis();
    lError = pError;
    pError = targetTemp - currentTemp;
    iError = pError * (double)(currentTime - lastTime);
    dError = (pError - lError) / (double)(currentTime - lastTime);
    return kp * pError + ki * iError + kd * dError + (currentTemp - 27) * (currentTemp - 27) * (currentTemp - 27) * kAmbiant;
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
    Serial.print(tempK - 273.15);
    Serial.print(" ");
    
    // noise peak removal
    if (tempK - lastK > 0.5 + abs(lastK -299.15) * 0.1) {
      if (spike0u == 0) {
        spike0u = tempK - lastK;
      }
      tempK -= spike0u;
    } else {
      spike0u = 0;
    }

    if (tempK - lastK < -20) {
      tempK += spike0u;
      spike0u = 0;
    }
    
    lastK = tempK;
    return (tempK - 273.15); // convert kelvin to celcius
  }
};


// setup pieltier tempature sensor
TempSensor pieltierT(thermP);

// setup pieltier PID
pid pieltierPID(1, 0.1, 500);

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

  // set initial pin state to off
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);
  analogWrite(ppwm, 0);
  analogWrite(fpwm, 0);
}

bool b1 = false; // code testing stuff
bool b2 = false; // code testing stuff
bool power = false; // code testing stuff, but maybe shouldent be

void loop() {
  // code testing stuff
  if (digitalRead(2) == HIGH) { // change target pieltier tempature
    if (b1 == false) {
      if (targetPieltierTemp == 27) {
        targetPieltierTemp = 100;
      } else {
        targetPieltierTemp = 27;
      }
      b1 = true;
    }
    
  } else {
    b1 = false;
  }

  if (digitalRead(3) == HIGH) { // turn on or off
    if (b2 == false) {
      if (power) {
        power = false;
      } else {
        power = true;
      }
      b2 = true;
    }
    
  } else {
    b2 = false;
  }
  


  currentPieltierTemp = pieltierT.getTemp(); // read pieltier temp
  pieltierDelta = pieltierPID.calculate(currentPieltierTemp, targetPieltierTemp); // calculate pid and set to output
  pieltierDelta = min(60, max(-60, pieltierDelta)); // clamp output between -60 and 60

  // for graphing system state
  Serial.print(currentPieltierTemp);
  Serial.print(" ");
  Serial.print(targetPieltierTemp);
  Serial.print(" ");
  Serial.print(pieltierDelta);
  Serial.println();
  
  if (!power || currentPieltierTemp > 150) {// shut off system if over 150 degrees for safety
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(fpwm, 1024);
    analogWrite(ppwm, 0);
    pieltierT.resetTemp();
    return;
  }
  

  // convert pieltierDelta to pwm, inA, inB, and fan signals
  analogWrite(ppwm, abs(pieltierDelta * pwmDivPieltierDelta));
  if (pieltierDelta > 0) {
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
    analogWrite(fpwm, 0);
  } else {
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
    analogWrite(fpwm, abs(pieltierDelta * pwmDivPieltierDelta * 3));
  }
}
