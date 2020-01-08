#!/bin/sh

EXEDIR=`pwd -P`
pushd . >/dev/null
cd $EXEDIR/../

time ./pro2db -i $1 -o $2 -u $3 -d

popd >/dev/null
