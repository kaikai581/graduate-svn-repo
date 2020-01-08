import os
import subprocess
import string
import socket
import sys
# used for cpu core count
import multiprocessing


def getInputFileName(lineRead):
  disectedpath = string.split(lineRead,"/")
  # find out what computer this script is executed on
  # set data path correspondingly
  host = socket.gethostname()
  if host == 'heplinux9':
    fullpath = "/home/sklin/RemoteMounts/PDSFDayaBayDaya/"
    for i in range(4, len(disectedpath)-1):
      fullpath = fullpath + disectedpath[i] + "/"
    fullpath = fullpath + disectedpath[len(disectedpath)-1]
  else:
    fullpath = lineRead
  return fullpath

def getSavePath(lineRead):
  disectedpath = string.split(lineRead,"/")
  savedir = "rawroot"
  for i in range(6, 10):
    savedir = savedir + '/' + disectedpath[i]
  return savedir

def getSaveFileName(lineRead):
  disectedpath = string.split(lineRead,"/")
  savefilename = 'rpcraw'
  disectedfn = string.split(disectedpath[len(disectedpath)-1], '.')
  for i in range(1, len(disectedfn)-1):
    savefilename = savefilename + '.' + disectedfn[i]
  savefilename = savefilename + '.root'
  return savefilename


# mergedlist is generated manually by dyb_locate.sh on PDSF
def runNuWa(cmd):
  return subprocess.call(cmd, shell = True)



if __name__ == '__main__':
  
  # if a number is input as a parameter,
  # NCPU*(parameter) of files will be processed
  if len(sys.argv) >= 2:
    try:
      procGroup = string.atoi(sys.argv[1])
    except ValueError:
      procGroup = -1
  else:
    procGroup = -1

  if not os.path.exists('mergedlist'):
    print 'run list doesn\'t exist'
    exit()
  f = open('mergedlist', 'r')
  ncpu = multiprocessing.cpu_count()

  gIdx = 0
  while True:
    lineRead = []
    fullpath = []
    savedir  = []
    savefilename = []
    savepathname = []
    commands = []
    for cpuIdx in range(ncpu):
      thisline = f.readline()
      if thisline == "" or thisline == "\n":
        break
      lineRead.append(thisline)
      # remove the new line at the end
      lineRead[cpuIdx] = lineRead[cpuIdx].rstrip("\n")
      fullpath.append(getInputFileName(lineRead[cpuIdx]))
      
      if os.path.exists(fullpath[cpuIdx]):
        savedir.append(getSavePath(lineRead[cpuIdx]))
        if not os.path.exists(savedir[cpuIdx]):
          os.makedirs(savedir[cpuIdx])
        savefilename.append(getSaveFileName(lineRead[cpuIdx]))
        savepathname.append(savedir[cpuIdx] + '/' + savefilename[cpuIdx])
        
      # set offline database environment variable
      command = "DBCONF=offline_db "
      command = command + "nuwa.py -n -1 "
      algCmd = '-m"MakeTree -o ' + savepathname[cpuIdx] + '" '
      command = command + algCmd
#      print "producing root file with data:\n" + fullpath[cpuIdx]
      command = command + fullpath[cpuIdx]
      commands.append(command)
#    print commands
      
    if procGroup != -1:
      gIdx = gIdx + 1
      if gIdx > procGroup:
        break
    pool = multiprocessing.Pool(processes = ncpu)
    print pool.map(runNuWa, commands)
    if thisline == "" or thisline == "\n":
      break
        
  
  
  ##    command = command + "nuwa.py -n -1 -m\"Quickstart.Calibrate\" "

  
    # write raw data into root file only if it doesn't exist
    #if not os.path.exists(savepathname):
        #nGroups = nGroups + 1
        #print nGroups
        #subprocess.call(commands[cpuIdx], shell=True)
      #else:
        #print 'file in run list is not found'
        
    #if nGroups >=1000:
      #break
  
