#!/bin/sh
#$ -N neutronyield

i=0
while read line
do
  let i++
  if [ $(( i )) -eq $SGE_TASK_ID ]; then
    . ~/Scripts/rootenv
    time ./neutron -r $line
  fi
  if [ $(( i )) -gt $SGE_TASK_LAST ]; then
    break
  fi
done < "runlist.txt"
