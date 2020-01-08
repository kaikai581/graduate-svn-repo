#!/bin/sh

EXEDIR=`pwd -P`
pushd . >/dev/null
cd $EXEDIR/../

time ./stopmu -i $1 -o $2

popd >/dev/null
