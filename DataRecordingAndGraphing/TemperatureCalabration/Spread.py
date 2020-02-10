import matplotlib.pyplot as plt
import numpy
import numpy as np
from matplotlib.collections import LineCollection
from matplotlib.colors import ListedColormap, BoundaryNorm
from scipy.optimize import curve_fit

fileName = input("Please enter Thermo file name ")
tFile = open(str(fileName + ".txt"), 'r')
tData = [[], [], [], [], [], []]
data = [num for num in tFile.readline().split('\t')]
while data != ['']:
    data = data[1:]
    for i in range(len(data)):
        tData[i].append(data[i])
    data =  tFile.readline().split('\t')
tFile.close()

tData = [[float(num) for num in tData[0]], [float(num) for num in tData[1]], [float(num) for num in tData[2]], [float(num) for num in tData[3]], [float(num) for num in tData[4]],  [3600 * float(num[0:2]) + 60 * float(num[3:5]) + float(num[6:13]) for num in tData[5]]]
tAvg = [sum([tData[j][i] for j in range(5)])/5 for i in range(len(tData[0]))]
maxError = max([max([abs(tData[j][i] - tAvg[i]) for j in range(5)]) for i in range(len(tAvg))])
print(maxError)
for i in range(5):
    plt.plot(tData[5], tData[i])
plt.show()
