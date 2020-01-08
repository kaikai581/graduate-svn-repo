#!/bin/sh
#$ -N event2db

# Usage:
# For EH1:
# qsub -t 5682-9883 qsubarray.sh
# For EH3:
# qsub -t 563-638 qsubarray.sh
# For 2012/01/01:
# qsub -t 1336-5681 qsubarray.sh

i=0
while read line
do
  let i++
  if [ $(( i )) -eq $SGE_TASK_ID ]; then
    . ~/Scripts/nuwaenv
    . ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/Event2DB_v04/cmt/setup.sh
    time nuwa.py -A 1*s -n -1 -l4 -m"Event2DB_v04.Event2DB_v042Alg" $line
  fi
  if [ $(( i )) -gt $SGE_TASK_LAST ]; then
    break
  fi
done < "fileList"
