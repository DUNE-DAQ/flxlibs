#!/bin/bash

if [ "$#" -ne 1 ] || [ "$1" == "-h" ]
then
  echo "Usage 'reset-gth.sh cardid'"
else
  cardid=$1
  echo "*** flx-reset GTH ***"
  flx-reset GTH -c $cardid
fi
