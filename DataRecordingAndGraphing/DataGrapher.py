import matplotlib.pyplot as plt
import numpy
fileName = "Data"
fileIndex = input("Please enter file number ")
startTime = float(input("Please desired start time "))
endTime = float(input("Please desired length ")) + startTime
file = open(str(fileName + str(fileIndex) + ".txt"), 'r')

readTemp = []
targetTemp = []
signal = []
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    data = [float(num) for num in data]
    readTemp.append(data[0])
    targetTemp.append(data[1])
    signal.append(data[2])
    data = file.readline().split()
file.close()

elapsedTime = float(data[1])
times = numpy.linspace(0, elapsedTime, num = len(readTemp)).tolist()


# clip data to specified bounds
while times[0] < startTime:
    times.pop(0)
    readTemp.pop(0)
    targetTemp.pop(0)
    signal.pop(0)
while times[len(times) - 1] > endTime:
    times.pop(len(times) - 1)
    readTemp.pop(len(readTemp) - 1)
    targetTemp.pop(len(targetTemp) - 1)
    signal.pop(len(signal) - 1)
times = [t - startTime for t in times] # move times to start time

# plot data and format
fig = plt.figure(figsize=(20, 6))
tempPlot = fig.add_subplot(211)
signalPlot = fig.add_subplot(212)
tempPlot.plot(times, readTemp, c = "b", linewidth = 1.0)
tempPlot.plot(times, targetTemp, c = "r", ls = "--", linewidth = 1.0)
tempPlot.axis([0, endTime - startTime, None, None])
tempPlot.set_ylabel("Temperature (C)")
tempPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
tempPlot.legend(("Current Temperature", "Target Temperature"), loc="upper left")
signalPlot.plot(times, signal, c = "g", linewidth = 1.0)
signalPlot.axis([0, endTime - startTime, None, None])
signalPlot.set_xlabel("Time (s)")
signalPlot.set_ylabel("PID Signal")
signalPlot.legend(("PID Signal",), loc="upper left")
fig.tight_layout()
fig.savefig(fileName + str(fileIndex) + ".png", dpi = 500)
plt.show()