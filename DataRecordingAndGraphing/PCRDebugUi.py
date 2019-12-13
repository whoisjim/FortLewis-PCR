# PCR Testing and Recording

from PyQt5 import QtCore, QtGui, QtWidgets
import pyqtgraph as pg
import sys
import numpy as np
import serial
import sip
import time

class PCRDebugApp(QtGui.QMainWindow):
    
    class cycle():
        def __init__(self, temperature, duration):
            self.temperature = temperature
            self.duration = duration

    class cycleQue():
        def __init__(self):
            self.cycleStartTime = None
            self.done = False
            self.cycles = []
        def add(self, newCycle):
            self.cycles.append(newCycle)
        def getTemp(self):
            if self.cycleStartTime == None:
                self.cycleStartTime = time.time()
            if self.cycles == []:
                return None
            if len(self.cycles) == 1:
                return self.cycles[0].temperature
                self.done = True
            if self.cycles[0].duration < time.time() - self.cycleStartTime:
                self.cycleStartTime += self.cycles[0].duration
                self.cycles.pop(0)
            return self.cycles[0].temperature
        def clear(self):
            self.cycles = []
            self.cycleStartTime = None
            self.done = False

    def __init__(self):
        super().__init__()
        self.port = serial.Serial()
        
        # states
        self.connected = False
        self.collect = False
        self.isCycle = False
        
        self.CPTemp = [] # curent peltier temp over time
        self.TPTemp = [] # target peltier temp over time
        self.pSignal = [] # curent signal being sent to curent drivers
        self.targetPeltierTemp = 40
        for i in range(2000):
           self.CPTemp.append(0)
           self.TPTemp.append(0)
           self.pSignal.append(0)
        self.X = np.arange(2000)
        
        self.cycles = self.cycleQue() 
        
        # for data recording
        self.logStartTime = 0
        self.logFile = None
        
        self.initUi()

    def initUi(self):
        self.setWindowTitle("Fort Lewis PCR")
        self.resize(1400, 600)
        
        windowLayout = QtWidgets.QGridLayout()
        centerWidget = QtWidgets.QWidget()
        centerWidget.setLayout(windowLayout)
        self.setCentralWidget(centerWidget)
        
        self.comBtn = QtGui.QPushButton("Connect", self)
        self.comBtn.clicked.connect(self.comConnect)   
        comText = QtGui.QLabel("COM", self)
        self.comTextBox = QtGui.QLineEdit(self)
        
        self.runBtn = QtGui.QPushButton("Start", self)
        self.runBtn.clicked.connect(self.startStop)
        self.cycleBtn = QtGui.QPushButton("Cycle", self)
        self.cycleBtn.clicked.connect(self.cycleOnOff)

        setBtn = QtGui.QPushButton("Set", self)
        setBtn.clicked.connect(self.setTemp)
        self.setTextBox = QtGui.QLineEdit(self)
        self.setTextBox.returnPressed.connect(self.setTemp)
        
        sendBtn = QtGui.QPushButton("Send", self)
        sendBtn.clicked.connect(self.sendCommand)
        self.sendTextBox = QtGui.QLineEdit(self)
        self.sendTextBox.returnPressed.connect(self.sendCommand)
        
        self.zoom = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.zoom.setRange(50, 2000)
        self.zoom.setValue(1000)
        
        cycleAddBtn = QtGui.QPushButton("Add", self)
        cycleAddBtn.clicked.connect(self.addCycleStep)
        
        cycleRmBtn = QtGui.QPushButton("Remove", self)
        cycleRmBtn.clicked.connect(self.rmCycleStep)

        self.cycleLayout = QtWidgets.QGridLayout()
        cycleWidget = QtWidgets.QWidget()
        cycleWidget.setLayout(self.cycleLayout)
        tempText = QtGui.QLabel("Temperature", self)
        durText = QtGui.QLabel("Duration", self)
        self.cycleNumTextBox = QtGui.QLineEdit(self)
        
        self.cycleTextBoxes = [(QtGui.QLineEdit(self),QtGui.QLineEdit(self)),(QtGui.QLineEdit(self),QtGui.QLineEdit(self)),(QtGui.QLineEdit(self),)]
        self.updateCycleUi()
        
        self.cycleLayout.addWidget(cycleAddBtn, 0, 0)
        self.cycleLayout.addWidget(cycleRmBtn, 0, 1)
        self.cycleLayout.addWidget(self.cycleNumTextBox, 1, 0)
        self.cycleLayout.addWidget(tempText, 2, 0)
        self.cycleLayout.addWidget(durText, 2, 1)
        
        self.plotPT = pg.PlotWidget()
        self.plotPT.hideAxis("bottom")
        self.pennCPTemp = pg.mkPen('b', style = QtCore.Qt.SolidLine)
        self.pennTPTemp = pg.mkPen('r', style = QtCore.Qt.SolidLine)
        self.plotPT.setXRange(1000, 2000)
        self.plotPT.setYRange(20, 90)
        style = {"color": "#FFF", "font-size": "20px"}
        self.plotPT.setLabel("left", "Temperature", 'C', **style)

        self.plotPS = pg.PlotWidget()
        self.plotPS.hideAxis("bottom")
        self.pennPSignal = pg.mkPen('g', style = QtCore.Qt.SolidLine)
        self.plotPS.setXRange(1000, 2000)
        self.plotPS.setYRange(-1024, 1024)
        self.plotPS.setLabel("left", "PWM", '', **style)
        
        windowLayout.addWidget(comText, 0, 0)
        windowLayout.addWidget(self.comTextBox, 0, 1)
        windowLayout.addWidget(self.comBtn, 0, 2, 1, 2)
        windowLayout.addWidget(self.runBtn, 1, 0, 1, 2)
        windowLayout.addWidget(self.cycleBtn, 1, 2, 1, 2)
        windowLayout.addWidget(setBtn, 2, 0)
        windowLayout.addWidget(self.setTextBox, 2, 1)
        windowLayout.addWidget(sendBtn, 2, 2)
        windowLayout.addWidget(self.sendTextBox, 2, 3)
        windowLayout.addWidget(self.zoom, 3, 0, 1, 4)
        windowLayout.addWidget(cycleWidget, 4, 0, 2, 4)
        windowLayout.addWidget(self.plotPT, 0, 4, 5, 20)
        windowLayout.addWidget(self.plotPS, 5, 4, 5, 20)

    def comConnect(self):
        if self.connected and not self.collect:
            self.port.close()
            self.connected = False
            self.comBtn.setText("Connect")
            self.comTextBox.setReadOnly(False)
        elif not self.collect:
            try:
                self.port = serial.Serial('COM' + str(self.comTextBox.text()), 9600, timeout=0.5) 
                self.connected = True
                self.comBtn.setText("Disconnect")
                self.comTextBox.setReadOnly(True)
                self.port.write("off\n".encode())
                self.port.write(("pt" + str(self.targetPeltierTemp) + "\n").encode())
            except:
                pass

    def startStop(self):
        if self.collect and self.connected:
            self.collect = False
            self.isCycle = False
            self.port.write("off\n".encode())
            self.runBtn.setText("Start")
            self.cycleBtn.setText("Cycle")
            self.logFile.write("e " + str(time.time() - self.logStartTime))
            self.logFile.close()
        elif self.connected:
            self.collect = True
            self.port.write("on\n".encode())
            self.runBtn.setText("Stop")
            self.port.flushInput()
            fileName = "Data"
            fileIndex = 0
            while True:
                try:
                    file = open(str(fileName + str(fileIndex) + ".txt"), 'r')
                    file.close()
                    fileIndex += 1
                except:
                    break
            self.logFile = open(str(fileName + str(fileIndex) + ".txt"), 'w')
            self.logStartTime = time.time()

    def setTemp(self):
        self.targetPeltierTemp = float(self.setTextBox.text())
        self.setTextBox.clear()
        
    def sendCommand(self):
        if self.sendTextBox.text()[0:2] == "pt":
            self.targetPeltierTemp = float(self.sendTextBox.text()[2:])
        else:
            self.port.write((self.sendTextBox.text() + "\n").encode())
        self.sendTextBox.clear()
    
    def cycleOnOff(self):
        if self.isCycle:
            self.isCycle = False
            self.cycleBtn.setText("Cycle")
        elif self.collect:
            self.isCycle = True
            self.cycleBtn.setText("Stop")
            self.cycles.clear()
            for j in range(int(self.cycleNumTextBox.text())):
                for i in range(len(self.cycleTextBoxes) - 1):
                    self.cycles.add(self.cycle(float(self.cycleTextBoxes[i][0].text()), float(self.cycleTextBoxes[i][1].text())))
            self.cycles.add(self.cycle(float(self.cycleTextBoxes[len(self.cycleTextBoxes) - 1][0].text()), 1))
    
    def addCycleStep(self):        
        self.cycleTextBoxes.insert(len(self.cycleTextBoxes) - 1, (QtGui.QLineEdit(self),QtGui.QLineEdit(self)))
        self.updateCycleUi()
        
    def rmCycleStep(self):
        try:
            sip.delete(self.cycleTextBoxes[len(self.cycleTextBoxes) - 2][1])
            sip.delete(self.cycleTextBoxes[len(self.cycleTextBoxes) - 2][0])
            self.cycleTextBoxes.pop(len(self.cycleTextBoxes) - 2)
        except:
            pass
        self.updateCycleUi()
        
    def updateCycleUi(self):
        for i in reversed(range(10, self.cycleLayout.count())): 
            self.cycleLayout.itemAt(i).widget().setParent(None)
            
        for i in range(len(self.cycleTextBoxes)):
            for j in range(len(self.cycleTextBoxes[i])):
                self.cycleLayout.addWidget(self.cycleTextBoxes[i][j], i + 3, j)
                
    def update(self):
        if self.connected:
            data = []
            try:
                portBytes = self.port.readline()
                line = str(portBytes[0 : len(portBytes) - 2].decode("utf-8"))
                data = [float(num) for num in line.split()]
            except:
                pass
            
            if self.isCycle:
                self.targetPeltierTemp = self.cycles.getTemp()
                
            if self.targetPeltierTemp != self.TPTemp[len(self.TPTemp) - 1]: # sends command for temp set
                self.port.write(("pt" + str(self.targetPeltierTemp) + "\n").encode())
            
            self.TPTemp.append(self.targetPeltierTemp) 
            self.TPTemp.pop(0)
            if len(data) == 2:
                self.CPTemp.append(data[0])
                self.pSignal.append(data[1])
                if self.collect:
                    self.logFile.write(str(data[0]) + " " + str(self.targetPeltierTemp) + " " + str(data[1]) + "\n")
            else:
                self.CPTemp.append(self.CPTemp[len(self.CPTemp) - 1])
                self.pSignal.append(self.pSignal[len(self.pSignal) - 1])
                if self.collect:
                    self.logFile.write(str(self.CPTemp[len(self.CPTemp) - 1]) + " " + str(self.targetPeltierTemp) + " " + str(self.pSignal[len(self.pSignal) - 1]) + "\n")
            self.CPTemp.pop(0)
            self.pSignal.pop(0)
            
        self.plotPT.setXRange(2000 - self.zoom.value(), 2000)
        self.plotPS.setXRange(2000 - self.zoom.value(), 2000)
        self.plotPT.plot(self.X, self.CPTemp, pen = self.pennCPTemp, clear = True)
        self.plotPT.plot(self.X, self.TPTemp, pen = self.pennTPTemp, clear = False)
        self.plotPS.plot(self.X, self.pSignal, pen = self.pennPSignal, clear = True)
        QtCore.QTimer.singleShot(1, self.update)
        
    def closeEvent(self, event):
        try:
            self.port.write("off\n".encode())
            self.port.close()
        except:
            pass

app = QtGui.QApplication(sys.argv)
window = PCRDebugApp()
window.show()
window.update()
app.exec_()