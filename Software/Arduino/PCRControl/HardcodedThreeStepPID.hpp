// the PID system used for the first pcr experiment
class HardcodedThreeStepPID {
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
    if (70 > targetTemp) {
      return 19 * pError + 0.2 * iError + 10000000 * dError;
    } else if (80 > targetTemp) {
      return 9 * pError + 0.17 * iError + 10000000 * dError;
    } else {
      return 9 * pError + 0.40 * iError + 10000000 * dError;
    }
  }
};