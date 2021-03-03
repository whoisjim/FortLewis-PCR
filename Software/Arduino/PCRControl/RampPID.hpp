// class for creating a pid system
class RampPID {
  private:
  double kp; // higher moves faster
  double ki; // higher fixes ofset and faster
  double kd; // higher settes faster but creastes ofset
  double kpc;
  double kic;
  double kdc;
  unsigned long currentTime, lastTime; // the time in millisecconds of this timesep and the last timestep
  double pError, lError, iError, dError; // error values for pid calculations. p: porportional, l: last, i: integerl, d: derivitive
  double iErrorLimit = 255;
  
  public:
  RampPID (double proportionalGain, double integralGain, double derivativeGain, double proportionalGainC, double integralGainC, double derivativeGainC) {
    kp = proportionalGain; // higher moves faster
    ki = integralGain; // higher fixes ofset and faster
    kd = derivativeGain; // higher settes faster but creastes ofset and amplifies noise

    kpc = proportionalGainC; // higher moves faster
    kic = integralGainC; // higher fixes ofset and faster
    kdc = derivativeGainC; // higher settes faster but creastes ofset and amplifies noise
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
    return (kp * targetTemp + kpc) * pError + (ki * targetTemp + kpc) * iError + (kd * targetTemp + kpc) * dError;
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

  void setKpc(float value) {
    kpc = value;
  }
  
  void setKic(float value) {
    kic = value;
  }
  
  void setKdc(float value) {
    kdc = value;
  }
};