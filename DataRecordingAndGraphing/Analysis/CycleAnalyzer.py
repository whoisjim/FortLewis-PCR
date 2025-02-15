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

cycleDiv = [(0, 94),]
temps = []

for i in range(1, len(times)):
    if targetTemp[i] != targetTemp[i-1]:
        cycleDiv.append((i, targetTemp[i]))

for i in cycleDiv:
    if i[1] not in temps:
        temps.append(i[1])

cycleNum = (len(cycleDiv) + 1) / len(temps)
print("temps", temps)
print("Number of cycles = ", cycleNum)

averages = [[],[],[]]
cycleLength = [20, 60, 60]

for i in range(len(temps) - 1, len(cycleDiv) - (len(temps))):
    for j in range(len(temps)):
        if cycleDiv[i][1] == temps[j]:
            averages[j].append(times[cycleDiv[i+1][0]] - times[cycleDiv[i][0]] - cycleLength[j])

for i in range(len(averages)):
    print("temp", temps[i], sum(averages[i]) / 33, "+-", 1.960 * numpy.sqrt(sum([(num - sum(averages[i]) / 33) ** 2 for num in averages[i]]) / 33) / numpy.sqrt(33), "s, Ci 95")

avgCTime = 0
for i in range(len(cycleDiv)):
    if cycleDiv[i][1] == 94:
        try:
            avgCTime += times[cycleDiv[i+3][0]] - times[cycleDiv[i][0]]
        except:
            pass
print("Average cycle time", avgCTime / 34, "s")
print("peak wats", max(power), "w")
print("Average wats", sum(power) / len(power))
print("+-", 1.960 * numpy.sqrt(sum([(num - 55.97) ** 2 for num in power]) / len(power)) / numpy.sqrt(len(power)), "w, Ci 95")

print("sample rate =", len(power) / elapsedTime)



