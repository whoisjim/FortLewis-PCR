import matplotlib.pyplot as plt
import numpy
startTime = 0
endTime = float(input("Please desired length ")) + startTime
times = [[],[],[],[]]
numData = [[[],[],[]],[[],[],[]],[[],[],[]],[[],[],[]]]

file = open(str("2mod15v.txt"), 'r')
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    data = [float(num) for num in data]
    numData[0][0].append(data[0])
    numData[0][1].append(data[1])
    numData[0][2].append(data[2])
    data = file.readline().split()
file.close()
elapsedTime = float(data[1])
times[0] = numpy.linspace(0, elapsedTime, num = len(numData[0][0])).tolist()

for i in range(len(numData[0][0]) - 1, -1, -1):
    if numData[0][1][i] != numData[0][1][len(numData[0][0]) - 1]:
        startTime = times[0][i]
        break

# clip data to specified bounds
while times[0][0] < startTime:
    times[0].pop(0)
    numData[0][0].pop(0)
    numData[0][1].pop(0)
    numData[0][2].pop(0)
while times[0][len(times[0]) - 1] > endTime:
    times[0].pop(len(times[0]) - 1)
    numData[0][0].pop(len(numData[0][0]) - 1)
    numData[0][1].pop(len(numData[0][1]) - 1)
    numData[0][2].pop(len(numData[0][2]) - 1)
times[0] = [t - startTime for t in times[0]] # move times to start time

file = open(str("4mod15v.txt"), 'r')
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    data = [float(num) for num in data]
    numData[1][0].append(data[0])
    numData[1][1].append(data[1])
    numData[1][2].append(data[2])
    data = file.readline().split()
file.close()
elapsedTime = float(data[1])
times[1] = numpy.linspace(0, elapsedTime, num = len(numData[1][0])).tolist()

for i in range(len(numData[1][0]) - 1, -1, -1):
    if numData[1][1][i] != numData[1][1][len(numData[1][0]) - 1]:
       startTime = times[1][i]
       break

# clip data to specified bounds
while times[1][0] < startTime:
    times[1].pop(0)
    numData[1][0].pop(0)
    numData[1][1].pop(0)
    numData[1][2].pop(0)
while times[1][len(times[1]) - 1] > endTime:
    times[1].pop(len(times[1]) - 1)
    numData[1][0].pop(len(numData[1][0]) - 1)
    numData[1][1].pop(len(numData[1][1]) - 1)
    numData[1][2].pop(len(numData[1][2]) - 1)
times[1] = [t - startTime for t in times[1]] # move times to start time

file = open(str("4mod7v.txt"), 'r')
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    data = [float(num) for num in data]
    numData[2][0].append(data[0])
    numData[2][1].append(data[1])
    numData[2][2].append(data[2])
    data = file.readline().split()
file.close()
elapsedTime = float(data[1])
times[2] = numpy.linspace(0, elapsedTime, num = len(numData[2][0])).tolist()

for i in range(len(numData[2][0]) - 1, -1, -1):
    if numData[2][1][i] != numData[2][1][len(numData[2][0]) - 1]:
        startTime = times[2][i]
        break

# clip data to specified bounds
while times[2][0] < startTime:
    times[2].pop(0)
    numData[2][0].pop(0)
    numData[2][1].pop(0)
    numData[2][2].pop(0)
while times[2][len(times[2]) - 1] > endTime:
    times[2].pop(len(times[2]) - 1)
    numData[2][0].pop(len(numData[2][0]) - 1)
    numData[2][1].pop(len(numData[2][1]) - 1)
    numData[2][2].pop(len(numData[2][2]) - 1)
times[2] = [t - startTime for t in times[2]] # move times to start time

file = open(str("4mod3v.txt"), 'r')
data = [float(num) for num in file.readline().split()]
while data[0] != "e":
    data = [float(num) for num in data]
    numData[3][0].append(data[0])
    numData[3][1].append(data[1])
    numData[3][2].append(data[2])
    data = file.readline().split()
file.close()
elapsedTime = float(data[1])
times[3] = numpy.linspace(0, elapsedTime, num = len(numData[3][0])).tolist()

for i in range(len(numData[3][0]) - 1, -1, -1):
    if numData[3][1][i] != numData[3][1][len(numData[3][0]) - 1]:
        startTime = times[3][i]
        break

# clip data to specified bounds
while times[3][0] < startTime:
    times[3].pop(0)
    numData[3][0].pop(0)
    numData[3][1].pop(0)
    numData[3][2].pop(0)
while times[3][len(times[3]) - 1] > endTime:
    times[3].pop(len(times[3]) - 1)
    numData[3][0].pop(len(numData[3][0]) - 1)
    numData[3][1].pop(len(numData[3][1]) - 1)
    numData[3][2].pop(len(numData[3][2]) - 1)
times[3] = [t - startTime for t in times[3]] # move times to start time

# plot data and format
fig = plt.figure(figsize=(20, 6))
tempPlot = fig.add_subplot(211)
signalPlot = fig.add_subplot(212)
tempPlot.plot(times[1], numData[1][0], c = "#BDD9BF", linewidth = 2.0)
tempPlot.plot(times[2], numData[2][0], c = "#A997DF", linewidth = 2.0)
tempPlot.plot(times[3], numData[3][0], c = "#E5323B", linewidth = 2.0)
tempPlot.plot(times[0], numData[0][0], c = "#2E4052", linewidth = 2.0)
tempPlot.axis([0, endTime - startTime, None, None])
tempPlot.set_ylabel("Temperature (C)")
tempPlot.tick_params( axis='x', which='both', bottom=False, top=False, labelbottom=False)
tempPlot.legend(("2 @ 15v", "4 @ 15v", "4 @ 7.5v", "4 @ 3.7v"), loc="lower right")
tempPlot.grid(linestyle="-")
signalPlot.plot(times[1], [numData[1][1][i] * numData[1][2][i] for i in range(len(numData[1][0]))], c = "#BDD9BF", linewidth = 2.0)
signalPlot.plot(times[2], [numData[2][1][i] * numData[2][2][i] for i in range(len(numData[2][0]))], c = "#A997DF", linewidth = 2.0)
signalPlot.plot(times[3], [numData[3][1][i] * numData[3][2][i] for i in range(len(numData[3][0]))], c = "#E5323B", linewidth = 2.0)
signalPlot.plot(times[0], [numData[0][1][i] * numData[0][2][i] for i in range(len(numData[0][0]))], c = "#2E4052", linewidth = 2.0)
signalPlot.axis([0, endTime - startTime, None, None])
signalPlot.set_xlabel("Time (s)")
signalPlot.set_ylabel("Power Consumed (W)")
signalPlot.legend(("2 @ 15v", "4 @ 15v", "4 @ 7.5v", "4 @ 3.7v"), loc="lower right")
signalPlot.grid(linestyle="-")
fig.tight_layout()
fig.savefig("efficiency.png", dpi = 100)
plt.show()
