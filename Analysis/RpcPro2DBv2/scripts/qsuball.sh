#!/bin/sh


INBASEDIR=/eliza16/dayabay/data/exp/dayabay


i=0
for year in `ls $INBASEDIR`
do
  for date in `ls $INBASEDIR/$year/p12b/Neutrino`
  do
    if [ $year == 2011 ] || [ $year == 2012 ]; then
      arryear[$i]=$year
      arrdate[$i]=$date
      let i++
    fi
  done
done



NDATES=${#arrdate[*]}
for ((i=0;i<$NDATES;i++))
do
  ./qsubdaily.sh ${arryear[$i]} ${arrdate[$i]} p12b
  while [ `qstat -u sklin|grep r|wc -l` -ne 0 ] || [ `qstat -u sklin|grep qw|wc -l` -ne 0 ]; do
    sleep 60
  done
  rm -rfv execute.sh.*
done
