// for creating a temperature sensor
// see elegooThermalResistorSch.png for wireing
class TemperatureSensor {
  private:
  int pin;
  
  public:
  TemperatureSensor(int iPin) { 
    pin = iPin; // the pin that conected to the tempature network
  }

  double getTemp() { // returns the tempature from thermoristor connected to thermP in degrees C, includes noise reduction
    int tempReading = analogRead(pin);
    double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK ); // kelvin
    return (tempK - 273.15); // convert kelvin to celcius
  }
};