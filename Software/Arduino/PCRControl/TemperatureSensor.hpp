class TemperatureSensor {
  // Constants from online thermistor calculator 
  static constexpr double A = 0.001158848211;
  static constexpr double B = 0.0002252088104;
  static constexpr double C = 0.0000001528340862;

  private:
    int pin;

  public:
    TemperatureSensor(int iPin) {
      pin = iPin;
    }

    double getTemp(double targetTemp) {
      int tempReading = analogRead(pin);
      if (tempReading <= 1 || tempReading >= 1022) return NAN;
      
      double resistance = 10000.0 * (1023.0 / tempReading - 1.0);
      double logR = log(10000.0 * ((1024.0 / tempReading - 1)));
      double tempK = 1.0 / (A + (B + C * logR * logR) * logR);

      double tempC = tempK - 273.15;

      return tempC;
    }
};
