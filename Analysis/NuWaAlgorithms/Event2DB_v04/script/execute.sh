#!/bin/sh

. ~/Scripts/nuwaenv
. ~/Work/Analysis/NuWaAlgorithms/PhysicsProcesses/Event2DB_v04/cmt/setup.sh

time nuwa.py -A 1*s -n -1 -l4 -m"Event2DB_v04.Event2DB_v042Alg" $1
