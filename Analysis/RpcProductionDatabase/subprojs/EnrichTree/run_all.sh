#!/bin/sh

INBASEDIR=/mnt/data6/scratch/sklin/MyData/DayaBay/RpcProductionTree
OUTBASEDIR=/mnt/data6/scratch/sklin/MyData/DayaBay/RpcEnrichedTree


i=0
for year in `ls $INBASEDIR`
do
  for ver in `ls $INBASEDIR/$year`
  do
    for date in `ls $INBASEDIR/$year/$ver/Neutrino`
    do
      arryear[$i]=$year
      arrdate[$i]=$date
      arrver[$i]=$ver
      OUTFILEPATH=$OUTBASEDIR/${arryear[$i]}/${arrver[$i]}/Neutrino/${arrdate[$i]}
      if [ ! -d $OUTFILEPATH ]; then
        mkdir -p $OUTFILEPATH
      fi
      let i++
    done
  done
done


arrlen=${#arryear[*]}

i=0
corecount=1
while [ $i -lt $arrlen ]; do
  INFILEPATH=$INBASEDIR/${arryear[$i]}/${arrver[$i]}/Neutrino/${arrdate[$i]}
  OUTFILEPATH=$OUTBASEDIR/${arryear[$i]}/${arrver[$i]}/Neutrino/${arrdate[$i]}
  INFILENUMBER=`ls -l $INFILEPATH 2>/dev/null|wc -l`
  OUTFILENUMBER=`ls -l $OUTFILEPATH 2>/dev/null|wc -l`
  if [ $OUTFILENUMBER -ne $INFILENUMBER ]; then
    echo "processing ${arrver[$i]} files on ${arryear[$i]}${arrdate[$i]}"
    #nice time ./addvariable -i $INFILEPATH -o $OUTFILEPATH &
    time ./addvariable -i $INFILEPATH -o $OUTFILEPATH &
    if test $corecount -ge 4; then
      wait
      corecount=1
    else
      let corecount=$corecount+1
    fi
  fi
  let i++
done
wait

