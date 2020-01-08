#!/bin/sh
#$ -N neubymu
#$ -e /global/project/projectdirs/dayabay/scratch/sklin/Analysis/NuWaAlgorithms/PhysicsProcesses/MuonLookupTable/script/log/error
#$ -o /global/project/projectdirs/dayabay/scratch/sklin/Analysis/NuWaAlgorithms/PhysicsProcesses/MuonLookupTable/script/log/output

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

OUTBASEPATH=/global/project/projectdirs/dayabay/scratch/sklin/RootFile/MuonLookupTable/MuonInducedNeutron

i=0
while read line
do
  let i++
  if [ $(( i )) -eq $SGE_TASK_ID ]; then
    . ~/Scripts/nuwaenv
    . /global/project/projectdirs/dayabay/scratch/sklin/Analysis/NuWaAlgorithms/PhysicsProcesses/MuonLookupTable/cmt/setup.sh
    OUTFILEPATH=$OUTBASEPATH/`echo ${line}|cut -d'/' -f7-10`
    if [ ! -d "$OUTFILEPATH" ]; then
      mkdir -p $OUTFILEPATH
    fi
    OUTFILENAME=`echo ${line}|rev|cut -d'/' -f1|rev|cut -d'.' -f2-`
    OUTFILENAME=$OUTFILEPATH/neutron.$OUTFILENAME
    time nuwa.py -A 1*s -n -1 -l4 -m"MuonLookupTable.MuonInducedNeutronAlg -o $OUTFILENAME -i $line" $line;
  fi
  if [ $(( i )) -gt $SGE_TASK_LAST ]; then
    break
  fi
done < "fileList"
