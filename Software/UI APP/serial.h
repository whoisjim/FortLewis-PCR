#include <string>
#include <queue>
#include <thread>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <fstream>

#include <iostream>

class PCRSerial {
  public:
    PCRSerial (std::string path);
    
    void start ();
    void stop ();

    float getPeltierTemperature ();
    float getLidTemperature ();
    int getPWM ();
    
    void setDataLog (bool);
    void setPower (bool);
    void setLidPower (bool);
    void setPeltierPower(bool);
    void setPeltierTemp(float);

    ~PCRSerial ();
  private:
    std::queue<std::string> commandBuffer_;
    int serialPort_;
    
    bool loop_;
    float targetTemperature_, peltierTemperature_, lidTemperature_;
    int PWM_;
    
    bool log_;
    std::ofstream logFile;
    timeval logStartTime;
    
    std::string readSerial ();
    void writeSerial (std::string);
};
