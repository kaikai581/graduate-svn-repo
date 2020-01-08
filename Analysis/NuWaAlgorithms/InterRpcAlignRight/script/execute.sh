#!/bin/sh

if [[ `hostname` == *heplinux* ]]; then
  RUNLEVEL=3
else
  RUNLEVEL=4
fi

time nuwa.py -A 1*s -n -1 -l${RUNLEVEL} -m"InterRpcAlignRight.InterRpcAlignRightAlg -o $2" $1
