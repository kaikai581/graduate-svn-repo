#!/bin/sh

if [[ `hostname` == *heplinux* ]]; then
  RUNLEVEL=3
else
  RUNLEVEL=4
fi

. ~/Scripts/nuwaenv
. ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/StoppedMuon/cmt/setup.sh

time nuwa.py -A 1*s -n -1 -l${RUNLEVEL} -m"StoppedMuon.MuonLookBack -o $2" $1
