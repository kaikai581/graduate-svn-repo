#!/bin/sh

INBASEPATH=/eliza16/dayabay/data/exp/dayabay
OUTBASEPATH=/global/project/projectdirs/dayabay/scratch/sklin/RootFile/StoppedMuonsNuWa

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

for fn in `ls ${INFILEPATH}/$FILEFILTER`
do
  OUTFILENAME=`echo ${fn}|cut -d'.' -f2-`
  OUTFILENAME=stoppedmu.$OUTFILENAME
  #if [ ! -f $OUTFILEPATH/$OUTFILENAME ]; then
#    qsub -V -l projectio=1 -l eliza16io=1 -l eliza7io=1 execute.sh ${fn} ${OUTFILEPATH}/${OUTFILENAME} &
    qsub -V -l projectio=1 -l eliza16io=1 execute.sh ${fn} ${OUTFILEPATH}/${OUTFILENAME} &
  #else
    #echo "File $OUTFILEPATH/$OUTFILENAME exists"
  #fi
done
