# PCR Testing and Recording

from PyQt5 import QtCore, QtGui, QtWidgets
import pyqtgraph as pg
import sys
import numpy as np
import serial
import matplotlib.pyplot as plt
import random
import copy
import time

class serialGraphApp(QtGui.QMainWindow):
    
    class cycle():
        def __init__(self, temperature, duration):
            self.temperature = temperature
            self.duration = duration

    class cycleQue():
        def __init__(self):
            self.cycleStartTime = None
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
            if self.cycles[0].duration < time.time() - self.cycleStartTime:
                self.cycleStartTime += self.cycles[0].duration
                self.cycles.pop(0)
            return self.cycles[0].temperature
        def clear(self):
            self.cycles = []
            self.cycleStartTime = None

    def __init__(self):
        super().__init__()
        self.port = serial.Serial()

        self.CPTemp = [] # curent peltier temp over time
        self.TPTemp = [] # target peltier temp over time
        self.pSignal = [] # curent signal being sent to curent drivers
        self.recordTime = [] # what time the points where recorded at
        self.targetPeltierTemp = 27
        for i in range(2000):
           self.CPTemp.append(0)
           self.TPTemp.append(0)
           self.pSignal.append(0)
           self.recordTime.append(time.time())
        self.X = np.arange(2000)
        
        self.cycles = self.cycleQue() 

        self.initUi()

    def initUi(self):
        self.setWindowTitle("Fort Lewis PCR")
        self.setFixedSize(1400, 600)
        self.comBtn = QtGui.QPushButton("Connect", self)
        self.comBtn.clicked.connect(self.comConnect)
        self.comBtn.resize(100, 30)
        self.comBtn.move(0,30)
        self.connected = False
        self.collect = False
        self.isCycle = False

        self.comText = QtGui.QLabel("COM", self)
        self.comText.move(13, 1)

        self.comTextBox = QtGui.QLineEdit(self)
        self.comTextBox.resize(50, 30)
        self.comTextBox.move(50,0)
        
        self.runBtn = QtGui.QPushButton("Start", self)
        self.runBtn.clicked.connect(self.startStop)
        self.runBtn.resize(100, 30)
        self.runBtn.move(0, 60)

        self.cycleBtn = QtGui.QPushButton("Cycle", self)
        self.cycleBtn.clicked.connect(self.cycleOnOff)
        self.cycleBtn.resize(100, 30)
        self.cycleBtn.move(0, 90)
        
        
        self.setBtn = QtGui.QPushButton("Set", self)
        self.setBtn.clicked.connect(self.setTemp)
        self.setBtn.resize(50, 30)
        self.setBtn.move(0, 120)
        
        self.setTextBox = QtGui.QLineEdit(self)
        self.setTextBox.resize(50, 30)
        self.setTextBox.move(50, 120)
        
        self.sendBtn = QtGui.QPushButton("Send", self)
        self.sendBtn.clicked.connect(self.sendCommand)
        self.sendBtn.resize(50, 30)
        self.sendBtn.move(0, 150)
        
        self.sendTextBox = QtGui.QLineEdit(self)
        self.sendTextBox.resize(50, 30)
        self.sendTextBox.move(50, 150)
        
        self.zoom = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.zoom.setRange(50, 2000)
        self.zoom.resize(100, 30)
        self.zoom.move(0, 180)
        self.zoom.setValue(1000)
        
        self.graphBtn = QtGui.QPushButton("Graph", self)
        self.graphBtn.clicked.connect(self.plotData)
        self.graphBtn.resize(100, 30)
        self.graphBtn.move(0, 210)

        self.graphText = QtGui.QLabel("Start     Length", self)
        self.graphText.move(4, 241)

        self.startTextBox = QtGui.QLineEdit(self)
        self.startTextBox.resize(50, 30)
        self.startTextBox.move(0, 270)

        self.lenTextBox = QtGui.QLineEdit(self)
        self.lenTextBox.resize(50, 30)
        self.lenTextBox.move(50, 270)

        self.plotPT = pg.PlotWidget(self)
        self.plotPT.hideAxis("bottom")
        self.plotPT.resize(1300, 300)
        self.plotPT.move(100, 0) 
        self.pennCPTemp = pg.mkPen('b', style = QtCore.Qt.SolidLine)
        self.pennTPTemp = pg.mkPen('r', style = QtCore.Qt.SolidLine)
        self.plotPT.setXRange(1000, 2000)
        self.plotPT.setYRange(20, 90)
        style = {"color": "#FFF", "font-size": "20px"}
        self.plotPT.setLabel("left", "Temperature", 'C', **style)

        self.plotPS = pg.PlotWidget(self)
        self.plotPS.hideAxis("bottom")
        self.plotPS.resize(1300, 300)
        self.plotPS.move(100, 300)
        self.pennPSignal = pg.mkPen('g', style = QtCore.Qt.SolidLine)
        self.plotPS.setXRange(1000, 2000)
        self.plotPS.setYRange(-1024, 1024)
        self.plotPS.setLabel("left", "PWM", '', **style)

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
        elif self.connected:
            self.collect = True
            self.port.write("on\n".encode())
            self.runBtn.setText("Stop")
            self.port.flushInput()

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
            self.cycles.add(self.cycle(40, 20))
            self.cycles.add(self.cycle(70, 20))
            self.cycles.add(self.cycle(40, 20))
            self.cycles.add(self.cycle(70, 20))
            self.cycles.add(self.cycle(40, 20))
            self.cycles.add(self.cycle(70, 20))
            self.cycles.add(self.cycle(55, 5))

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
            else:
                self.CPTemp.append(self.CPTemp[len(self.CPTemp) - 1])
                self.pSignal.append(self.pSignal[len(self.pSignal) - 1])
            self.CPTemp.pop(0)
            self.pSignal.pop(0)
            self.recordTime.append(time.time())
            self.recordTime.pop(0)
        self.plotPT.setXRange(2000 - self.zoom.value(), 2000)
        self.plotPS.setXRange(2000 - self.zoom.value(), 2000)
        self.plotPT.plot(self.X, self.CPTemp, pen = self.pennCPTemp, clear = True)
        self.plotPT.plot(self.X, self.TPTemp, pen = self.pennTPTemp, clear = False)
        self.plotPS.plot(self.X, self.pSignal, pen = self.pennPSignal, clear = True)
        QtCore.QTimer.singleShot(1, self.update)
        
    def plotData(self):
        readTemp = copy.deepcopy(self.CPTemp)
        targetTemp = copy.deepcopy(self.TPTemp)
        signal = copy.deepcopy(self.pSignal)
        times = [float(time) - float(self.recordTime[0]) for time in self.recordTime]
        
        # clip data to specified bounds
        while times[0] < float(self.startTextBox.text()):
            times.pop(0)
            readTemp.pop(0)
            targetTemp.pop(0)
            signal.pop(0)
        while times[len(times) - 1] > float(self.lenTextBox.text()) + float(self.startTextBox.text()):
            times.pop(len(times) - 1)
            readTemp.pop(len(readTemp) - 1)
            targetTemp.pop(len(targetTemp) - 1)
            signal.pop(len(signal) - 1)
        times = [t - float(self.startTextBox.text()) for t in times] # move times to start time

        # plot data and format
        fig = plt.figure(figsize=(20, 6))
        tempPlot = fig.add_subplot(211)
        signalPlot = fig.add_subplot(212)
        tempPlot.plot(times, readTemp, c = "b", linewidth = 1.0)
        tempPlot.plot(times, targetTemp, c = "r", ls = "--", linewidth = 1.0)
        tempPlot.axis([0, float(self.lenTextBox.text()), None, None])
        tempPlot.set_ylabel("Temperature (C)")
        tempPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
        tempPlot.legend(("Current Temperature", "Target Temperature"), loc="upper left")
        signalPlot.plot(times, signal, c = "g", linewidth = 1.0)
        signalPlot.axis([0, float(self.lenTextBox.text()), None, None])
        signalPlot.set_xlabel("Time (s)")
        signalPlot.set_ylabel("PID Signal")
        signalPlot.legend(("PID Signal",), loc="upper left")
        fig.tight_layout()
        fig.savefig("img.png", dpi = 500)
        plt.show()
    
    def closeEvent(self, event):
        try:
            self.port.write("off\n".encode())
            self.port.close()
        except:
            pass

app = QtGui.QApplication(sys.argv)
window = serialGraphApp()
window.show()
window.update()
app.exec_()
