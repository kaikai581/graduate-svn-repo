#!/bin/sh

INBASEPATH=/mnt/data6/scratch/sklin/MyData/DayaBay/RpcProductionTree
OUTBASEPATH=/mnt/data6/scratch/sklin/MyData/DayaBay/RpcEnrichedTree

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

./addvariable -i $INFILEPATH -o $OUTFILEPATH
