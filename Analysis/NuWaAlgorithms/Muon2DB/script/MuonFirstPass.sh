#!/bin/sh

if [[ `hostname` == *heplinux* ]]; then
  RUNLEVEL=3
else
  RUNLEVEL=4
fi

. ~/Scripts/nuwaenv
. ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/Muon2DB/cmt/setup.sh

FILENAME=$1
RUNNUMBER=`echo $FILENAME|cut -d'.' -f3`
FILENUMBER=`echo $FILENAME|cut -d'.' -f7|tr -d '_'`

time nuwa.py -A 1*s -n -1 -l${RUNLEVEL} -m"Muon2DB.FirstPassAlg -r $RUNNUMBER -f $FILENUMBER" $1
