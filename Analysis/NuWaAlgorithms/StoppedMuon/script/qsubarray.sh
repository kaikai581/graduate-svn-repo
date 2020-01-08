#!/bin/sh
#$ -N neubymu

# Usage:
# For EH1:
# qsub -t 5682-9883 qsubarray.sh
# For EH3:
# qsub -t 563-638 qsubarray.sh
# For 2012/01/01:
# qsub -t 1336-5681 qsubarray.sh
# For comparison with Yung-Shun's neutron list:
# qsub -t 1-224 qsubarray.sh
# Start EH3 roduction:
# qsub -t 29281-39638 qsubarray.sh

STARTTIME=20
ENDTIME=1000
TRACKCYLINDERRADIUS=1000

OUTBASEPATH=/global/project/projectdirs/dayabay/scratch/sklin/RootFile/StoppedMuonsNuWa

i=0
while read line
do
  let i++
  if [ $(( i )) -eq $SGE_TASK_ID ]; then
    . ~/Scripts/nuwaenv
    . ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/StoppedMuon/cmt/setup.sh
    OUTFILEPATH=$OUTBASEPATH/`echo ${line}|cut -d'/' -f7-10`
    if [ ! -d "$OUTFILEPATH" ]; then
      mkdir -p $OUTFILEPATH
    fi
    OUTFILENAME=`echo ${line}|rev|cut -d'/' -f1|rev|cut -d'.' -f2-`
    OUTFILENAME=$OUTFILEPATH/neutron.$OUTFILENAME
    time nuwa.py -A 1*s -n -1 -l4 -m"StoppedMuon.MuonLookBack -o $OUTFILENAME -s $STARTTIME -e $ENDTIME -r $TRACKCYLINDERRADIUS" $line;
  fi
  if [ $(( i )) -gt $SGE_TASK_LAST ]; then
    break
  fi
done < "fileList"
