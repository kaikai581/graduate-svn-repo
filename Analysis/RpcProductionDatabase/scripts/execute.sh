#!/bin/sh
if [ "$1" == "" ]; then
  EXEDIR=".."
else
  EXEDIR="$1"
fi
echo "executing file: "
echo "$2"
pushd `pwd -P`
cd $EXEDIR
./rpc2db -i $2 -o $3
popd
