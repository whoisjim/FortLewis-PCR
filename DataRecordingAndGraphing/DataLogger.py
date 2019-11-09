import serial
import time

def logToFile():
    s = serial.Serial("COM3")
    s.flushInput()
    fileName = "Data"
    fileIndex = 0
    while True:
        try:
            file = open(str(fileName + str(fileIndex) + ".txt"), 'r')
            file.close()
            fileIndex += 1
        except:
            break
    print("Start data log in ", fileName, fileIndex, ".txt", sep = '')
    file = open(str(fileName + str(fileIndex) + ".txt"), 'w')
    startTime = time.time()
    while True:
        try:
            sBytes = s.readline()
            sDecoded = str(sBytes[0:len(sBytes)-2].decode("utf-8"))
            print(sDecoded)
            file.write(sDecoded)
            file.write("\n")
        except:
            break
    endTime = time.time()
    file.write("e " + str(endTime - startTime))
    file.close()
    print("End data log in ", fileName, fileIndex, ".txt", sep = '')
    s.close()
    return

logToFile()