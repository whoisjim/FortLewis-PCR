import matplotlib
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
while times[len(times) - 1] > endTime:
    times.pop(len(times) - 1)
    readTemp.pop(len(readTemp) - 1)
    targetTemp.pop(len(targetTemp) - 1)
times = [t - startTime for t in times] # move times to start time

# plot data and format
matplotlib.rcParams['font.serif'] = "Times New Roman"
# Then, "ALWAYS use sans-serif fonts"
matplotlib.rcParams['font.family'] = "serif"

ticSpace = 15
fig = plt.figure(figsize=(5, 3))
tempPlot = fig.add_subplot(111)
tempPlot.plot(times, readTemp, c = "#A7C1AE", linewidth = 2.0)
tempPlot.plot(times, targetTemp, c = "#B03642", ls = "--", linewidth = 2.0)
tempPlot.axis([0, endTime - startTime, None, None])
tempPlot.set_ylabel("Temperature (C)")
tempPlot.set_xlabel("Time (S)")
tempPlot.set_xticks([i for i in range(0, int(endTime - startTime), ticSpace)])
tempPlot.legend(("Block", "Target"), loc="lower right")
tempPlot.grid(linestyle="-")
fig.tight_layout()
fig.savefig(fileName + ".png", dpi = 600)
plt.show()
