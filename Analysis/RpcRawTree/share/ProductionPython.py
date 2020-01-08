# mass producing root files with python algorithm instead of c++ algorithm
import os
import subprocess
import string
import socket
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
if not os.path.exists('mergedlist'):
  print 'run list doesn\'t exist'
  exit()
f = open('mergedlist', 'r')
lineRead = f.readline()
# remove the new line at the end
lineRead = lineRead.rstrip("\n")

fullpath = getInputFileName(lineRead)

if os.path.exists(fullpath):
  savedir = getSavePath(lineRead)
  if not os.path.exists(savedir):
    os.makedirs(savedir)
  savefilename = getSaveFileName(lineRead)
  savepathname = savedir + '/' + savefilename
  print savepathname
  
  # set offline database environment variable
  command = "DBCONF=offline_db "
  command = command + "nuwa.py -n 300 "
  algCmd = '-m"PythonMakeTree -o ' + savepathname + '" '
  command = command + algCmd
  print "producing root file with data:\n" + fullpath
  command = command + fullpath
#  print command
  
  # write raw data into root file only if it doesn't exist
  if not os.path.exists(savepathname):
    subprocess.call(command, shell=True)
else:
  print 'file in run list is not found'
#print multiprocessing.cpu_count()


