#!/bin/sh
INFILEPATH=/eliza16/dayabay/data/exp/dayabay/2012/p12b/Neutrino
pushd . > /dev/null
cd ..
WORKDIR=`pwd`
popd > /dev/null
for DATE in `ls ${INFILEPATH}`
do
  OUTFILEPATH=${WORKDIR}/file/2012/p12b/Neutrino/$DATE
  if [ ! -d "$OUTFILEPATH" ]; then
    mkdir -p $OUTFILEPATH    
    qsub -V ./massproduction.sh $DATE &
  fi
done
