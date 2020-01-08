#!/bin/sh

# This script is meant to be called by ./qsubmassproduction.sh

if [ "$1" == "" ]; then
  echo "Please input date as an argument."
fi
INFILEPATH=/eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino/$1
pushd . > /dev/null
cd ..
WORKDIR=`pwd`
popd > /dev/null
OUTFILEPATH=${WORKDIR}/file/2012/p12b/Neutrino/$1
if [ ! -d "$OUTFILEPATH" ]; then
  mkdir -p $OUTFILEPATH
fi

for fn in `ls ${INFILEPATH}/*.root`
do
  ${WORKDIR}/evtsel -i ${fn} -o ${OUTFILEPATH} > /dev/null
done
