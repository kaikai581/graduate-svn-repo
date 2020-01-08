#!/bin/sh
#$ -N neutronyield
#$ -e /global/project/projectdirs/dayabay/scratch/sklin/Analysis/StandAlone/MuonInducedNeutronsBinSky/log/error
#$ -o /global/project/projectdirs/dayabay/scratch/sklin/Analysis/StandAlone/MuonInducedNeutronsBinSky/log/output


i=0
while read line
do
  let i++
  if [ $(( i )) -eq $SGE_TASK_ID ]; then
    . ~/Scripts/rootenv
    time ./binsky -r $line
  fi
  if [ $(( i )) -gt $SGE_TASK_LAST ]; then
    break
  fi
done < "runlist.txt"
