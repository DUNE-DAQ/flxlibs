#!/bin/bash
set -e # exit if any command exits with non-zero status

### Tests dataflow of TP links with data from the optical fibers using low level tools ###

# full reset of card
flx-reset ALL; flx-init; flx-reset GTH

echo "enabling readout links:"
sleep 1
for i in $(seq 0 5); do flx-config -d 0 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;
for i in $(seq 0 5); do flx-config -d 1 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;

# check elink status
for i in $(seq 0 5); do flx-config -d 0 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;
for i in $(seq 0 5); do flx-config -d 1 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;

echo "setting superchunk factor:"
sleep 1
for i in $(seq 0 5); do flx-config -d 0 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;
for i in $(seq 0 5); do flx-config -d 1 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;

# check superchunk factors have changed
for i in $(seq 0 5); do flx-config -d 0 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;
for i in $(seq 0 5); do flx-config -d 1 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;

# diable internal emulators for front end tests
femu -d 0 -n
femu -d 1 -n

echo "While fdaq is running configure front end to play data!"
sleep 5
echo "running fdaq on SLR 0:"
fdaq -d 0 -t 60 > out_fdaq_0.log &
echo "running fdaq on SLR 1:"
fdaq -d 1 -t 60 > out_fdaq_1.log &


wait
echo "Test Complete!"
echo "quick summary: "
tail -n 4 out_fdaq_0.log
tail -n 4 out_fdaq_1.log