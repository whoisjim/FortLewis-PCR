import matplotlib.pyplot as plt
import numpy
from scipy.optimize import curve_fit
fileName = input("Please enter PCR file name ")
PCRFile = open(str(fileName + ".txt"), 'r')
fileName = input("Please enter Thermo file name ")


readTemp = []
targetTemp = []
signal = []
startTimeStr = PCRFile.readline()
startTime = 3600 * float(startTimeStr[0:2]) + 60 * float(startTimeStr[3:5]) + float(startTimeStr[6:13])
data = [float(num) for num in PCRFile.readline().split()]
while data[0] != "e":
    try:
        data = [float(num) for num in data]
        if len(data) == 5:
    	    readTemp.append(data[1])
    	    targetTemp.append(data[0])
    	    signal.append(data[4])
    except:
        pass
    data =  PCRFile.readline().split()
PCRFile.close()

elapsedTime = float(data[1])
times = numpy.linspace(0, elapsedTime, num = len(readTemp)).tolist()

tFile = open(str(fileName + ".txt"), 'r')
tData = [[], [], [], [], [], []]
data = [num for num in tFile.readline().split('\t')]
while data != ['']:
    data = data[1:]
    for i in range(len(data)):
        tData[i].append(data[i])
    data =  tFile.readline().split('\t')
tFile.close()

tData = [[float(num) for num in tData[0]], [float(num) for num in tData[1]], [float(num) for num in tData[2]], [float(num) for num in tData[3]], [float(num) for num in tData[4]],  [3600 * float(num[0:2]) + 60 * float(num[3:5]) + float(num[6:13]) - startTime for num in tData[5]]]

# plot data and format
fig = plt.figure(figsize=(20, 6))
tempPlot = fig.add_subplot(211)
func = fig.add_subplot(212)
for i in range(5):
    tempPlot.plot(tData[5], tData[i], c = "g", linewidth = 1.0)

tempPlot.plot(times, readTemp, c = "b", linewidth = 1.0)
tempPlot.plot(times, targetTemp, c = "r", ls = "--", linewidth = 1.0)
tempPlot.axis([0, elapsedTime, None, None])
tempPlot.set_ylabel("Temperature (C)")
tempPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
tempPlot.legend(("T0", "T1", "T2", "T3", "T4", "Current Temperature", "Target Temperature"), loc="upper left")
func.plot(readTemp, signal)
def eq(x, m, b):
    return m * x + b
popt, pcov = curve_fit(eq, readTemp[0:85919], tData[0])
func.plot(readTemp[0:85919], tData[0]) # make actyally line up
x = (0, 100)
y = [eq(i, *popt) for i in x]
func.plot(x, y)
fig.tight_layout()
fig.savefig(fileName + ".png", dpi = 500)
plt.show()
print("y =", popt[0], "* x +", popt[1])
print(len(tData[5]), len(times))

