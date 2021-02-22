#!/bin/bash

if [ "$#" -ne 1 ] || [ "$1" == "-h" ]
then
  echo "Usage 'init.sh cardid'"
else
  cardid=$1
  echo "card id" $cardid
  echo "*** flx-init ***"
  flx-init -c $cardid
  echo "*** flx-info si5324 ***"
  flx-info si5345 -c $cardid
  echo "*** flx-reset GTH ***"
  flx-reset GTH -c $cardid
  echo "*** flx-info gbt ***"
  flx-info gbt -c $cardid
fi
