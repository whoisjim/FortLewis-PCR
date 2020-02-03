import matplotlib.pyplot as plt
import numpy
fileName = input("Please enter file name ")
startTime = float(input("Please desired start time "))
endTime = float(input("Please desired length ")) + startTime
file = open(str(fileName + ".txt"), 'r')

readTemp = []
targetTemp = []
signal = []
power = []
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    try:
        data = [float(num) for num in data]
        readTemp.append(data[1])
        targetTemp.append(data[0])
        signal.append(data[2])
        power.append(abs(data[2]) * (14.76 / 255.0) * data[3])
    except:
        pass
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
    power.pop(0)
while times[len(times) - 1] > endTime:
    times.pop(len(times) - 1)
    readTemp.pop(len(readTemp) - 1)
    targetTemp.pop(len(targetTemp) - 1)
    signal.pop(len(signal) - 1)
    power.pop(len(power) - 1)
times = [t - startTime for t in times] # move times to start time

# plot data and format
ticSpace = 10
fig = plt.figure(figsize=(20, 6))
tempPlot = fig.add_subplot(311)
signalPlot = fig.add_subplot(312)
powerPlot = fig.add_subplot(313)
tempPlot.plot(times, readTemp, c = "#A7C1AE", linewidth = 2.0)
tempPlot.plot(times, targetTemp, c = "#B03642", ls = "--", linewidth = 2.0)
tempPlot.axis([0, endTime - startTime, None, None])
tempPlot.set_ylabel("Temperature (C)")
tempPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
tempPlot.set_xticks([i for i in range(0, int(endTime - startTime), ticSpace)])
tempPlot.legend(("Current Temperature", "Target Temperature"), loc="upper left")
tempPlot.grid(linestyle="-")
signalPlot.plot(times, signal, c = "#2E4052", linewidth = 2.0)
signalPlot.axis([0, endTime - startTime, None, None])
signalPlot.set_ylabel("PID Signal")
signalPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
signalPlot.set_xticks([i for i in range(0, int(endTime - startTime), ticSpace)])
signalPlot.grid(linestyle="-")
powerPlot.plot(times, power, c = "#867EB6", linewidth = 2.0)
powerPlot.axis([0, endTime - startTime, None, None])
powerPlot.set_xlabel("time (s)")
powerPlot.set_ylabel("Power consumed (W)")
powerPlot.grid(linestyle="-")
powerPlot.set_xticks([i for i in range(0, int(endTime - startTime), ticSpace)])
fig.tight_layout()
fig.savefig(fileName + ".png", dpi = 100)
plt.show()
