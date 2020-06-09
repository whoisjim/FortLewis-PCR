import matplotlib
import matplotlib.pyplot as plt
import numpy

fig = plt.figure(figsize=(5, 3))
tempPlot = fig.add_subplot(111)

temps = {'old0' : [21], 'old1' : [21], 'old2' : [21], 'old3' : [21], 'new0' : [21], 'new1' : [21], 'new2' : [21], 'new3' : [21]}

for key in temps.keys():
  file = open(key + ".txt", 'r')

  data = [float(num) for num in file.readline().split()]
  while data[0] != "e":
    try:
      data = [float(num) for num in data]
      if abs(data[0] - temps[key][-1]) < 2:
        temps[key].append(data[0])
      else:
        temps[key].append(temps[key][-1])
    except:
      pass
    data = file.readline().split()
    while data == []:
      data = file.readline().split()
  file.close()

  elapsedTime = float(data[1])
  times = numpy.linspace(0, elapsedTime, num = len(temps[key])).tolist()

  # clip data to specified bounds
  while temps[key][0] < 25:
    times.pop(0)
    temps[key].pop(0)
  while temps[key][-1] < 71:
    times.pop(len(times) - 1)
    temps[key].pop(len(temps[key]) - 1)
  while temps[key][len(times) - 1] > 70:
    times.pop(len(times) - 1)
    temps[key].pop(len(temps[key]) - 1)
  times = [t - times[0] for t in times] # move times to start time

  # plot data and format
  if key[0:3] == "new":
    tempPlot.plot(times, temps[key], c = [int(key[-1]) / 6 + 2/6, int(key[-1]) / 10 + 1/5, int(key[-1]) / 6 + 2/6], linewidth = 2.0)
  else:
    tempPlot.plot(times, temps[key], c = [int(key[-1]) / 10 + 1/5, int(key[-1]) / 6 + 2/6, int(key[-1]) / 6 + 2/6], linewidth = 2.0)

  print(key, times[-1])

tempPlot.set_ylabel("Temperature (C)")
tempPlot.set_xlabel("Time (S)")
tempPlot.grid(linestyle="-")
tempPlot.legend(temps.keys(), loc="lower right")
fig.tight_layout()
fig.savefig("temps.png", dpi = 600)
plt.show()
