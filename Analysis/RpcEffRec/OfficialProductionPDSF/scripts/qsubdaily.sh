#!/bin/sh


# command example:
# $ ./qsubdaily.sh 20120217 kup


if [ "$1" == "" ]; then
  echo "Please enter the date to be processed."
else
  YEAR=${1:0:4}
  DATE=${1:4:4}
fi


if [ "$2" == "" ]; then
  VER="p12b"
else
  VER=$2
fi

INFILEPATH=/eliza16/dayabay/data/exp/dayabay/$YEAR/$VER/Neutrino/$DATE
if [ ! -d "$INFILEPATH" ]; then
  echo "Directory $INFILEPATH doesn't exist"
fi


pushd . > /dev/null
cd ..
WORKDIR=`pwd`
popd > /dev/null
LISTDIR=`pwd`


OUTFILEPATH=/eliza7/dayabay/scratch/sklin/RootFile/RpcEffRec/$YEAR/$VER/Neutrino/$DATE
if [ ! -d "$OUTFILEPATH" ]; then
  mkdir -p $OUTFILEPATH
fi


for fn in `ls ${INFILEPATH}/*.root`
do
  if grep -q ${fn} ${LISTDIR}/NoRpcFileList.txt; then
    echo "bad RPC file:"
    echo "${fn}"
  else
    OUTFILENAME=`echo ${fn}|cut -d'.' -f2-`
    OUTFILENAME=eff.$OUTFILENAME
    if [ ! -f $OUTFILEPATH/$OUTFILENAME ]; then
      qsub -V -l eliza16io=1 execute.sh ${WORKDIR} ${fn} ${OUTFILEPATH}&
    else
      echo "File $OUTFILEPATH/$OUTFILENAME exists"
    fi
  fi
done
