bool pPower = false; // software pielter on/off
double targetPeltierTemp = 0;
double currentPeltierTemp = 0;

void setup() {
  // setup serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  if (Serial.available() > 0) {
    String incomingCommand = Serial.readString();
    
    if (incomingCommand == "d\n") {
      Serial.print(currentPeltierTemp);
      Serial.print(" ");
      Serial.print(targetPeltierTemp - currentPeltierTemp);
      Serial.print(" ");
      Serial.print(23.5);
      Serial.print("\n");
    }
    if (incomingCommand == "off\n") {
      pPower = false;
    } else if (incomingCommand.substring(0,3) == "off") {
      pPower = false;
      targetPeltierTemp = incomingCommand.substring(3).toFloat();
    }
    if (incomingCommand == "on\n") {
      pPower = true;
    } else if (incomingCommand.substring(0,2) == "on") {
      pPower = true;
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
    if (incomingCommand.substring(0,2) == "pt") {
      targetPeltierTemp = incomingCommand.substring(2).toFloat();
    }
  }

  if (pPower) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  if (currentPeltierTemp - targetPeltierTemp < 0.01) {
    currentPeltierTemp += 0.001;
  } else if (currentPeltierTemp - targetPeltierTemp > -0.01) {
    currentPeltierTemp -= 0.001;
  }
}
