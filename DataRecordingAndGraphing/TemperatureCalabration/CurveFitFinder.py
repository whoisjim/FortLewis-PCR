import matplotlib.pyplot as plt
import numpy
import numpy as np
from matplotlib.collections import LineCollection
from matplotlib.colors import ListedColormap, BoundaryNorm
from scipy.optimize import curve_fit

startT = float(input("Please desired start time "))
endT = float(input("Please desired length ")) + startT

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
        if len(data) == 4:
    	    readTemp.append(data[1])
    	    targetTemp.append(data[0])
    	    signal.append(data[2])
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

# clip data to specified bounds
while times[0] < startT:
    times.pop(0)
    readTemp.pop(0)
    targetTemp.pop(0)
while times[len(times) - 1] > endT:
    times.pop(len(times) - 1)
    readTemp.pop(len(readTemp) - 1)
    targetTemp.pop(len(targetTemp) - 1)
times = [t - startT for t in times] # move times to start time

# clip data to specified bounds
while tData[5][0] < startT:
    for i in range(6):
        tData[i].pop(0)
while tData[5][len(tData[5]) - 1] > endT:
    for i in range(6):
        tData[i].pop(len(tData[i]) - 1)
tData[5] = [t - startT for t in tData[5]] # move times to start time

tAvg = [sum([tData[j][i] for j in range(5)])/5 for i in range(len(tData[0]))]

# plot data and format
fig, axs = plt.subplots(2, 1)
for i in range(5):
    axs[0].plot(tData[5], tData[i], c = "k", ls = "--", linewidth = 1)
axs[0].plot(tData[5], tAvg, c = "k", lineWidth = 2.0)

points = np.array([numpy.asarray(times), numpy.asarray(readTemp)]).T.reshape(-1, 1, 2)
segments = np.concatenate([points[:-1], points[1:]], axis=1)
lc = LineCollection(segments, cmap='jet')
lc.set_array(numpy.asarray(times))
lc.set_linewidth(2)
line = axs[0].add_collection(lc)

axs[0].plot(times, targetTemp, c = "r", ls = "--", linewidth = 2.0)
axs[0].axis([0, endT - startT, None, None])

def getClose(data, time, target):
    for i in range(len(time)):
        if target < time[i]:
            return data[i]
    return data[-1]

closeData = [getClose(readTemp, times, target) for target in tData[5]] 

def eq(x, m, b):
    return m * x + b

popt, pcov = curve_fit(eq, closeData, tAvg)
#popt, pcov = curve_fit(eq, readTemp[0:len(tData[0])], tAvg)

print("y =", popt[0], "* x +", popt[1])

points = np.array([numpy.asarray(closeData), numpy.asarray(tAvg)]).T.reshape(-1, 1, 2)
#points = np.array([numpy.asarray(readTemp[0:len(tData[0])]), numpy.asarray(tAvg)]).T.reshape(-1, 1, 2)

segments = np.concatenate([points[:-1], points[1:]], axis=1)
lc = LineCollection(segments, cmap='jet')
lc.set_array(numpy.asarray(tData[5]))
lc.set_linewidth(2)
line = axs[1].add_collection(lc)

x = (50, 110)
y = [eq(i, *popt) for i in x]
axs[1].plot(x, y, c = "k", ls = "--", lineWidth = 2.0)
fig.tight_layout()
fig.savefig(fileName + ".png", dpi = 500)
plt.show()


