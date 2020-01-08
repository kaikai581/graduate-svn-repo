#!/bin/sh

INBASEDIR=/eliza16/dayabay/data/exp/dayabay


i=0
if [ -d "$INBASEDIR" ]; then
  for year in `ls $INBASEDIR`
  do
    ver=p12e
    if [ -d "$INBASEDIR/$year/$ver/Neutrino" ]; then
      for date in `ls $INBASEDIR/$year/$ver/Neutrino`
      do
        arryear[$i]=$year
        arrdate[$i]=$date
        arrver[$i]=$ver
        let i++
      done
    fi
  done
fi

arrlen=${#arryear[*]}
i=0
while [ $i -lt $arrlen ]; do
  ls -d $INBASEDIR/${arryear[$i]}/${arrver[$i]}/Neutrino/${arrdate[$i]}/*EH*.root >> fileList
  let i++
done
