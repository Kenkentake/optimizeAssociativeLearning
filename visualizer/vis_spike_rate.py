import matplotlib.pyplot as plt
import numpy as np
import math
import sys
import os.path
import matplotlib as mpl
import matplotlib.pyplot as plt
plt.switch_backend('agg')
mpl.use('Agg')
mpl.rc('xtick',labelsize = 15)
mpl.rc('ytick',labelsize = 15)


def readSpikeRecordFile(filename):
    datafile = open(filename,'r')
    data = datafile.readlines()
    global interval, delay, size, tstop,istim1,istim2
    interval = int(data[0].split(":")[1])
    delay = float(data[1].split(":")[1])
    size = int(data[2].split(":")[1])
    tstop = int(data[3].split(":")[1])
    spt = [] #spike timing
    for i in range(len(data)):
        if(data[i][0]!='$'):
            time = data[i].rstrip('\n')
            #print time
            #spt.append(float(time))
            try :
                spt.append(float(time))
            except ValueError:
                pass
    #print spt
    datafile.close()
    return spt


spike_time_list = readSpikeRecordFile(sys.argv[1])
record_time_list = list(range(30, 691, 15))

freq_list = []
spike_counter = 0
for target_time in record_time_list:
    for spike_time in spike_time_list:
        if target_time-30 < spike_time and spike_time < target_time:
            spike_counter = spike_counter + 1
    freq_list.append(spike_counter / 0.03)
    spike_counter = 0

for freq in freq_list:
    print freq
