#!/bin/bash
set -e # exit if any command exits with non-zero status

### tests low level tools to communicate/control the card ##

# full reset of card
flx-reset ALL
flx-init
flx-reset GTH
# check connection to front end devices
flx-info POD
flx-info LINK

echo "enabling readout links:"
sleep 1
# check elink status before
echo "elink status before setting"
for i in $(seq 0 5); do flx-config -d 0 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;
for i in $(seq 0 5); do flx-config -d 1 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;

for i in $(seq 0 5); do flx-config -d 0 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;
for i in $(seq 0 5); do flx-config -d 1 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;

# check elink status has changed
echo "elink status after setting"
for i in $(seq 0 5); do flx-config -d 0 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;
for i in $(seq 0 5); do flx-config -d 1 DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA; done;

echo "setting superchunk factor:"
sleep 1
# check initial superchunk factors
echo "superchunk factor before setting"
for i in $(seq 0 5); do flx-config -d 0 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;
for i in $(seq 0 5); do flx-config -d 1 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;

for i in $(seq 0 5); do flx-config -d 0 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;
for i in $(seq 0 5); do flx-config -d 1 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;

# check superchunk factors have changed
echo "superchunk factor after setting"
for i in $(seq 0 5); do flx-config -d 0 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;
for i in $(seq 0 5); do flx-config -d 1 list | grep SUPER_CHUNK_FACTOR_LINK_0${i}; done;

sleep 1
EMU_FILE="${DBT_AREA_ROOT}/dtp-patterns/EMUInput/FixedHits_A_emuconfig"
echo "loading pattern: ${EMU_FILE}"
if [ -f "${EMU_FILE}" ]
then
    echo "emulator file found"
    flx-config -d 0 load ${EMU_FILE}
    flx-config -d 1 load ${EMU_FILE}
else
    echo "emulator file not found!"
    exit 1
fi
# enable internal emulators
femu -d 0 -e
femu -d 1 -e

echo "Test complete!"
