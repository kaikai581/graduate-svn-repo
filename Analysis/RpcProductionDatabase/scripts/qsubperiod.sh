#!/bin/sh

INBASEDIR=/eliza16/dayabay/data/exp/dayabay


i=0
for year in `ls $INBASEDIR`
do
  ver=p12b
  for date in `ls $INBASEDIR/$year/$ver/Neutrino`
  do
    arryear[$i]=$year
    arrdate[$i]=$date
    arrver[$i]=$ver
    let i++
  done
done


arrlen=${#arryear[*]}
i=0
while [ $i -lt $arrlen ]; do
  ./qsubdaily.sh ${arryear[$i]}${arrdate[$i]} p12b
  while [ `qstat -u sklin|grep r|wc -l` -ne 0 ] || [ `qstat -u sklin|grep qw|wc -l` -ne 0 ]; do
    sleep 60
  done
  let i++
done
