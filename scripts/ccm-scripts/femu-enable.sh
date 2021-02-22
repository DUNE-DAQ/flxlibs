#!/bin/bash

if [ "$#" -ne 3 ] || [ "$1" == "-h" ]
then
  echo "Usage 'femu-enable.sh cardid' "
else
  cardid=$1
  echo "card id" $cardid
  femu -d $cardid -e

  echo "card id" $((cardid+1))
  femu -d $((cardid+1)) -e
fi
