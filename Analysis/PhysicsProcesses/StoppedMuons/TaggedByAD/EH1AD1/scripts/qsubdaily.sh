#!/bin/sh

INBASEPATH=/eliza16/dayabay/data/exp/dayabay
OUTBASEPATH=/eliza7/dayabay/scratch/sklin/RootFile/StoppedMuonsTaggedByAD

YEAR=$1
DATE=$2
VER=$3

INFILEPATH=$INBASEPATH/$YEAR/$VER/Neutrino/$DATE

if [ ! -d "$INFILEPATH" ]; then
  echo "Directory $INFILEPATH doesn't exist"
fi

OUTFILEPATH=$OUTBASEPATH/$YEAR/$VER/Neutrino/$DATE

if [ ! -d "$OUTFILEPATH" ]; then
  mkdir -p $OUTFILEPATH
fi

for fn in `ls ${INFILEPATH}/*.root`
do
  OUTFILENAME=`echo ${fn}|cut -d'.' -f2-`
  OUTFILENAME=stoppedmu.$OUTFILENAME
  if [ ! -f $OUTFILEPATH/$OUTFILENAME ]; then
    qsub -V -l eliza16io=1 -l eliza7io=1 execute.sh ${fn} ${OUTFILEPATH} &
  else
    echo "File $OUTFILEPATH/$OUTFILENAME exists"
  fi
done
