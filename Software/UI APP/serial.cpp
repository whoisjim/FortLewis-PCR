#include "serial.h"

PCRSerial::PCRSerial (std::string path)  {
  serialPort_ = open(path.c_str(), O_RDWR);

  if (serialPort_ < 0) {
    printf("Error %i opening serial port: %s\n", errno, strerror(errno));
  }

  struct termios tty;
  memset(&tty, 0, sizeof(tty));

  if (tcgetattr(serialPort_, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }

  tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
  tty.c_cflag |= CREAD | CLOCAL | CS8;

  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

  tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  tty.c_oflag &= ~(OPOST | ONLCR);

  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;

  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  if (tcsetattr(serialPort_, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }

  targetTemperature_ = 0;
  peltierTemperature_ = 0;
  lidTemperature_ = 0;
  PWM_ = 0;
}

void PCRSerial::start () {
  loop_ = true;
  while (loop_) {
    if (!commandBuffer_.empty()) { // send command
      writeSerial(commandBuffer_.front());
      commandBuffer_.pop();
      sleep(2);
    } else { // check temperature
      writeSerial("d\n");
      sleep(1);

      std::string data = readSerial();

      if (log_) {
      	logFile_ << targetTemperature_ << " " << data; 
      }
      
      std::stringstream serialStringStream(data);
      
      std::string word;
      serialStringStream >> word;
      if (word != "") {
      	peltierTemperature_ = std::stof(word);
      }
      serialStringStream >> word;
      if (word != "") {
        PWM_ = std::stof(word);
      }
      serialStringStream >> word;
      if (word != "") {
        lidTemperature_ = std::stof(word);
      } 
      sleep(1);
    }
  }
}

void PCRSerial::writeSerial (std::string message) {
  write(serialPort_, message.c_str(), sizeof(message.c_str()));
}

std::string PCRSerial::readSerial () {
  char readBuffer [256];
  std::memset(&readBuffer, '\0', sizeof(readBuffer));
  read(serialPort_, &readBuffer, 256);
  return std::string(readBuffer);
}

void PCRSerial::stop () {
  loop_ = false;
}

float PCRSerial::getPeltierTemperature () {
  return peltierTemperature_;
}

float PCRSerial::getLidTemperature () {
  return lidTemperature_;
}

int PCRSerial::getPWM () {
  return PWM_;
}

void PCRSerial::setDataLog (bool state) {
  log_ = state;
  if (log_) {
    static int logFileNumber;
    logFile_.open("log" + std::to_string(logFileNumber++) + ".txt");
    gettimeofday(&logStartTime_, NULL);
  } else {
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    logFile_ << "e " << (currentTime.tv_sec - logStartTime_.tv_sec) + (currentTime.tv_usec - logStartTime_.tv_usec) * 1e-6 << std::endl;
    logFile_.close(); 
  }
}

void PCRSerial::setPower (bool state) {
  if (state) {
    commandBuffer_.push("on\n");
  } else {
    commandBuffer_.push("off\n"); 
  }
}

void PCRSerial::setLidPower (bool state) {
  if (state) {
    commandBuffer_.push("onl\n");
  } else {
    commandBuffer_.push("offl\n"); 
  }
}

void PCRSerial::setPeltierPower(bool state) {
  if (state) {
    commandBuffer_.push("onp\n");
  } else {
    commandBuffer_.push("offp\n"); 
  }
}

void PCRSerial::setPeltierTemp(float temperature) {
  commandBuffer_.push("pt" + std::to_string((int)temperature) + '\n');
  targetTemperature_ = temperature;
}

PCRSerial::~PCRSerial () {
  close(serialPort_);
}
