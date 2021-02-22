#!/bin/bash

if [ "$#" -ne 3 ] || [ "$1" == "-h" ]
then
  echo "Usage 'linkconfig.sh cardid N M' with N and M as number of enabled emu links on the endpoints"
else
  cardid=$1
  echo "card id" $cardid
  flx-config -d $cardid load emu-5links-slr1
  femu -d $cardid -n

  echo "card id" $((cardid+1))
  flx-config -d $((cardid+1)) load emu-5links-slr2
  femu -d $((cardid+1)) -n

fi
