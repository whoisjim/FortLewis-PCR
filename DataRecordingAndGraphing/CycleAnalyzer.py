import matplotlib.pyplot as plt
import numpy
fileName = input("Please enter file name ")
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
    while len(data) < 2: # skip malformed data
        data = file.readline().split()
file.close()

elapsedTime = float(data[1])
times = numpy.linspace(0, elapsedTime, num = len(readTemp)).tolist()

startTime = 0

for i in range(1, len(times)):
    if targetTemp[i] != targetTemp[i-1]:
        startTime = times[i]
        break

while times[0] < startTime:
    times.pop(0)
    readTemp.pop(0)
    targetTemp.pop(0)
    signal.pop(0)
    power.pop(0)
times = [t - startTime for t in times] # move times to start time

cycleDiv = [(0, 95),]

for i in range(1, len(times)):
    if targetTemp[i] != targetTemp[i-1]:
        cycleDiv.append((i, targetTemp[i]))

print("Number of cycles = ", len(cycleDiv)/2)

avgHTime = 0
avgCTime = 0
cycleLength = 20

for i in range(len(cycleDiv) - 1):
    if cycleDiv[i][1] == 55:
        avgCTime += times[cycleDiv[i+1][0]] - times[cycleDiv[i][0]] - cycleLength
    else:
        avgHTime += times[cycleDiv[i+1][0]] - times[cycleDiv[i][0]] - cycleLength

avgHTime /= len(cycleDiv)/2
avgCTime /= len(cycleDiv)/2

print("Avg Heating", avgHTime, "s")
print("Avg cooling", avgCTime, "s")

print("time for n cycles")
for i in range(len(cycleDiv)):
    if cycleDiv[i][1] == 95:
        print(i, times[cycleDiv[i][0]] // 60, times[cycleDiv[i][0]] % 60)
