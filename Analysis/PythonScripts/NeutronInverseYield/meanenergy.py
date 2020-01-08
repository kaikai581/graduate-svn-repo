#!/usr/bin/python

import sys
import numpy
import math

filenames = []
filenames.append("/disk1/NuWa/external/data/0.0/Muon/mountain_DYB")
filenames.append("/disk1/NuWa/external/data/0.0/Muon/mountain_LA")
filenames.append("/disk1/NuWa/external/data/0.0/Muon/mountain_Far_80m")


selectedfilename = filenames[int(sys.argv[1])]
print("muon energy spectrum from:")
print(selectedfilename)


# omit the first 6 lines
linecount = 0
energies = []
for line in open(selectedfilename):
  linecount += 1
  if linecount <= 6:
    continue
  columns = line.split()
  if len(columns) >= 1:
    energies.append(float(columns[0]))


# power mean
exp = 0.78
pwm = 0
totMuNum = 0
for en in energies:
  totMuNum += 1
  pwm += math.pow(en, exp)

print("total number of muons: "+str(totMuNum))
print("mean muon energy: "+str(numpy.mean(energies))+" GeV")
print("power mean energy: "+str(math.pow(pwm/totMuNum,1./exp))+" GeV")
