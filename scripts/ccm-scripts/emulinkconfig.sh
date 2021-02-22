#!/bin/bash

if [ "$#" -ne 3 ] || [ "$1" == "-h" ]
then
  echo "Usage 'emulinkconfig.sh cardid slr1-config-file slr2-config-file' "
else
  cardid=$1
  rmfile1=$2
  rmfile2=$3
  echo "card id" $cardid
  flx-config -d $cardid load $rmfile1
  femu -d $cardid -e

  echo "card id" $((cardid+1))
  flx-config -d $((cardid+1)) load $rmfile2
  femu -d $((cardid+1)) -e
fi
