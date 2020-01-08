#!/bin/sh

INBASEPATH=/eliza16/dayabay/data/exp/dayabay

YEAR=$1
DATE=$2
VER=$3
HALL=$4

INFILEPATH=$INBASEPATH/$YEAR/$VER/Neutrino/$DATE

if [ ! -d "$INFILEPATH" ]; then
  echo "Directory $INFILEPATH doesn't exist"
fi


FILEFILTER="*.root"
if [[ $HALL != "" ]]; then
  FILEFILTER="*$HALL*.root"
fi

for fn in `ls ${INFILEPATH}/$FILEFILTER`
do
    qsub -V -l projectio=1 -l eliza16io=1 MuonFirstPass.sh ${fn} &
done
