#!/bin/sh


INBASEDIR=/eliza16/dayabay/data/exp/dayabay

INBASEPATH=/eliza16/dayabay/data/exp/dayabay
OUTBASEPATH=/eliza7/dayabay/scratch/sklin/RootFile/RpcPro2DBv2
DBOUTBASEPATH=/eliza7/dayabay/scratch/sklin/Database


i=0
for year in `ls $INBASEDIR`
do
  for date in `ls $INBASEDIR/$year/p12b/Neutrino`
  do
    if [ $year == 2011 ] || [ $year == 2012 ]; then
      arryear[$i]=$year
      arrdate[$i]=$date
      let i++
    fi
  done
done



NDATES=${#arrdate[*]}
for ((i=0;i<$NDATES;i++))
do
  INFILEPATH=$INBASEPATH/${arryear[$i]}/p12b/Neutrino/${arrdate[$i]}
  
  if [ ! -d "$INFILEPATH" ]; then
    echo "Directory $INFILEPATH doesn't exist"
  fi
  
  OUTFILEPATH=$OUTBASEPATH/${arryear[$i]}/p12b/Neutrino/${arrdate[$i]}
  
  if [ ! -d "$OUTFILEPATH" ]; then
    mkdir -p $OUTFILEPATH
  fi


  for fn in `ls ${INFILEPATH}/*.root`
  do
    OUTFILENAME=`echo ${fn}|cut -d'.' -f2-`
    OUTFILENAME=rpctrees.$OUTFILENAME
    if [ ! -f $OUTFILEPATH/$OUTFILENAME ]; then
      qsub -V -l eliza16io=1 execute.sh ${fn} ${OUTFILEPATH} ${DBOUTBASEPATH}&
    else
      echo "File $OUTFILEPATH/$OUTFILENAME exists"
    fi
    sleep 60
    while [ `qstat -u sklin|grep r|wc -l` -ne 0 ] || [ `qstat -u sklin|grep qw|wc -l` -ne 0 ]; do
      sleep 60
    done
  done

done
