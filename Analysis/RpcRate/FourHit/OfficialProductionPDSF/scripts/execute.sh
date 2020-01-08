#!/bin/sh
if [ "$1" == "" ]; then
  EXEDIR=".."
else
  EXEDIR="$1"
fi
echo "executing file: "
echo "$2"
$EXEDIR/calcrate -i $2 -o $3
