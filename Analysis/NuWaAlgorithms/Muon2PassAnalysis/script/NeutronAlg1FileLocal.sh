#!/bin/sh

# sample usage:
# $ ./NeutronAlg1FileLocal.sh /eliza16/dayabay/data/exp/dayabay/2012/p12e/Neutrino/0101/recon.Neutrino.0021358.Physics.EH1-Merged.P12B-P._0001.root 100

if [[ `hostname` == *heplinux* ]]; then
  RUNLEVEL=3
else
  RUNLEVEL=4
fi

. ~/Scripts/nuwaenv
. ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/Muon2PassAnalysis/cmt/setup.sh

FILENAME=$1
RUNNUMBER=`echo $FILENAME|cut -d'.' -f3`
FILENUMBER=`echo $FILENAME|cut -d'.' -f7|tr -d '_'`

NEVENT=-1
if [[ $2 != "" ]]; then
  NEVENT=$2
fi

time nuwa.py -A 1*s -n ${NEVENT} -l${RUNLEVEL} -m"Muon2PassAnalysis.NeutronAlg -r $RUNNUMBER -f $FILENUMBER" $1
