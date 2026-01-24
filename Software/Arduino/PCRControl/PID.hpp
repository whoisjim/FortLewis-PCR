class PID {
  private:
    double kp, ki, kd;
    double outputMin, outputMax;
    double pError = 0, iError = 0;
    double lastDTerm = 0;      
    double iErrorLimit = 50; 
    unsigned long currentTime = 0, lastTime = 0;
    bool firstRun = true;

  public:
    PID(double pGain = 1, double iGain = 0, double dGain = 0,
        double minOut = -255, double maxOut = 255)
    {
      kp = pGain; ki = iGain; kd = dGain;
      outputMin = minOut; outputMax = maxOut;
    }

    void reset() {
      iError = 0; pError = 0; lastDTerm = 0;
      firstRun = true;
      lastTime = millis();
    }

    double calculate(double currentTemp, double targetTemp) {
      currentTime = millis();

      if (firstRun) {
        lastTime = currentTime;
        pError = targetTemp - currentTemp; 
        firstRun = false;
        return 0;
      }

      double dt = (currentTime - lastTime) / 1000.0;
      if (dt <= 0) dt = 0.001;
      lastTime = currentTime;

      double currentError = targetTemp - currentTemp;
      
      // Calculate Derivative term 
      double rawDTerm = kd * ((currentError - pError) / dt);
      
      // Filter the derivative
      double dAlpha = 0.8; 
      double dTerm = (dAlpha * lastDTerm) + ((1.0 - dAlpha) * rawDTerm);
      lastDTerm = dTerm;

      // Update pError for the next cycle
      pError = currentError;
      double provisionalOutput = (kp * pError) + (ki * iError) + dTerm;
      
      if (!((provisionalOutput > outputMax && pError > 0) || 
            (provisionalOutput < outputMin && pError < 0))) {
        iError += pError * dt; 
        iError = constrain(iError, -iErrorLimit, iErrorLimit);
      }

      // Final output
      double output = (kp * pError) + (ki * iError) + dTerm;
      return constrain(output, outputMin, outputMax);
    }
    // Setters
    void setKp(double value) { kp = value; }
    void setKi(double value) { ki = value; }
    void setKd(double value) { kd = value; }

    void setOutputLimits(double minOut, double maxOut) {
      outputMin = minOut;
      outputMax = maxOut;
    }

    void setIntegratorLimit(double limit) {
      iErrorLimit = limit;
    }
};
