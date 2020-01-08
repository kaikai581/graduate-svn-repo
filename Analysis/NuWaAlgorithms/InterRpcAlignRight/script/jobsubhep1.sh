#!/bin/sh

INBASEPATH=/mnt/data2/dayabay_data/data_processed
OUTBASEPATH=/mnt/data6/scratch/sklin/MyData/DayaBay/InterRpcAlignRight

YEAR=$1
DATE=$2
VER=$3
HALL=$4

INFILEPATH=$INBASEPATH/$YEAR/$VER/Neutrino/$DATE

if [ ! -d "$INFILEPATH" ]; then
  echo "Directory $INFILEPATH doesn't exist"
fi

OUTFILEPATH=$OUTBASEPATH/$YEAR/$VER/Neutrino/$DATE

if [ ! -d "$OUTFILEPATH" ]; then
  mkdir -p $OUTFILEPATH
fi

FILEFILTER="*.root"
if [[ $HALL != "" ]]; then
  FILEFILTER="*$HALL*.root"
fi

i=1
for fn in `ls ${INFILEPATH}/$FILEFILTER`; do
  let "remainder = $i % 8"
  OUTFILENAME=`echo ${fn}|cut -d'.' -f2-`
  OUTFILENAME=interrpc.$OUTFILENAME
  ./execute.sh ${fn} ${OUTFILEPATH}/${OUTFILENAME} &
  pidlist[$remainder]=`echo ${!}`
  if [ "$remainder" -eq "0" ]; then
    for j in `seq 0 7`; do
#      echo "wait for pid: ${pidlist[${remainder}]}"
      wait ${pidlist[${j}]}
    done
  fi
  let i+=1
done
