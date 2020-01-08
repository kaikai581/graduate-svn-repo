# Later on I found scripts might be able to excute in background.

# The job gets killed because of the ampersand.
# This script never worked.

#!/bin/sh
if [ "$1" == "" ]; then
  echo "no limit on number of jobs"
  njobs=1000000
else
  if [ "$1" != "-q" ]; then
    echo "number of jobs: $1"
    njobs=$1
  else
    njobs=1000000
  fi
fi
INFILEPATH=/eliza16/dayabay/data/exp/dayabay/2011/p12b/Neutrino
pushd . > /dev/null
cd ..
WORKDIR=`pwd`
popd > /dev/null
LISTDIR=`pwd`
i=0
for DATE in `ls ${INFILEPATH}`
do
  OUTFILEPATH=/eliza7/dayabay/scratch/sklin/RootFile/RpcEffRec/2011/p12b/Neutrino/$DATE
  if [ ! -d "$OUTFILEPATH" ]; then
    mkdir -p $OUTFILEPATH    
    for fn in `ls ${INFILEPATH}/${DATE}/*.root`
    do
#      ${WORKDIR}/evtsel -i ${fn} -o ${OUTFILEPATH}&
      if [ $i -ge $njobs ]; then
        exit
      fi
      if [ "$1" == "-q" ]; then
        echo ${checkfn}
      else
        if grep -q ${fn} ${LISTDIR}/NoRpcFileList.txt; then
          echo "bad RPC file:"
          echo "${fn}"
        else
          qsub -V -l eliza16io=1 execute.sh ${WORKDIR} ${fn} ${OUTFILEPATH}&
          let i++
        fi
      fi
    done
# If the directory already exists, check if any file was left out.
  else
    for fn in `ls ${INFILEPATH}/${DATE}/*.root`
    do
      choppedfn=${fn#*.}
      checkfn=${OUTFILEPATH}/eff.${choppedfn}
      if [ ! -e "$checkfn" ]; then
        if [ $i -ge $njobs ]; then
          exit
        fi
        if [ "$1" == "-q" ]; then
          echo ${checkfn}
        else
          if grep -q ${fn} ${LISTDIR}/NoRpcFileList.txt; then
            echo "bad RPC file:"
            echo "${fn}"
          else
            qsub -V -l eliza16io=1 execute.sh ${WORKDIR} ${fn} ${OUTFILEPATH}&
            let i++
          fi
        fi
      fi
    done
  fi
done
