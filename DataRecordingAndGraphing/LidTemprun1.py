# -*- coding: utf-8 -*-
"""
Created on Thu Jan 16 12:00:46 2020

@author: Volta
"""

import matplotlib.pyplot as plt
#from matplotlib import style
import numpy


Lidfile = open("C:/Users/Volta/Documents/PCR Stuff/FortLewis-PCR/DataRecordingAndGraphing/HeatingLid200s.txt")
 
Etime = 201
LidData = []
Data = [float(num) for num in Lidfile.readline().split()]

while Data[0] != "e":

        Data = [float(num) for num in Data]
        LidData.append(Data[0])
        
        Data = Lidfile.readline().split()
        
Lidfile.close()
times = numpy.linspace(0,Etime,num = len(LidData)).tolist()


ticspace = 10 

fig = plt.figure(figsize=(10,20))

LidPlot = fig.add_subplot(311,facecolor='white')  

LidPlot.plot(times,LidData, linewidth =2.0)

LidPlot.axis([0,201, None,None])
LidPlot.axis(True,True)

LidPlot.set_ylabel("Temperature (C)",color='black')
LidPlot.set_xlabel("Time (s)",color='black')
LidPlot.tick_params( axis='x', which='both', bottom=True, top=False, labelbottom=True)
LidPlot.set_xticks([i for i in range(0, int(211), ticspace)])
LidPlot.set_yticks([i for i in range(0, int(121), ticspace)])
LidPlot.grid(linestyle="-",color='grey')
LidPlot.spines['top'].set_visible(True)
LidPlot.spines['right'].set_visible(True)
LidPlot.spines['left'].set_visible(True)
LidPlot.spines['bottom'].set_visible(True)
#LidPlot.set_axis_bgcolor("white") 

#plt.plot(times,LidData)
#plt.show()

#fig.tight_layout()
fig.savefig("HeatedLidGraph.png",dpi = 500,facecolor=fig.get_facecolor())
plt.show()
