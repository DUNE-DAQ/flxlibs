#!/bin/bash
set -e # exit if any command exits with non-zero status

### Tests dataflow of TP links with data from the wibulator using low level tools ###

# full reset of card
flx-reset ALL; flx-init; flx-reset GTH

echo "enabling TP link on SLR 0:"
sleep 1
flx-config -d 1 set DECODING_LINK05_EGROUP0_CTRL_EPATH_ENA=1

# check elink status
flx-config -d 1 DECODING_LINK05_EGROUP0_CTRL_EPATH_ENA

echo "setting superchunk factor:"
sleep 1
flx-config -d 1 set SUPER_CHUNK_FACTOR_LINK_05=0x0c

# check superchunk factors have changed
flx-config -d 1 list | grep SUPER_CHUNK_FACTOR_LINK_05

sleep 1
EMU_FILE="${DBT_AREA_ROOT}/dtp-patterns/FixedHits/A/FixedHits_A_wib_axis_32b.txt"
echo "loading pattern into wibulator: ${EMU_FILE}"

echo "running fdaq on SLR 0:"
fdaq -d 1 -t 60 -X -T out.bin > out_fdaq.log &

echo "Run hit finding (mask all channels execpt 0):"
hfButler.py flx-1-p2-hf init
hfButler.py flx-1-p2-hf cr-if --on --drop-empty
hfButler.py flx-1-p2-hf flowmaster --src-sel wibtor --sink-sel hits
hfButler.py flx-1-p2-hf link hitfinder -t 20
hfButler.py flx-1-p2-hf link mask enable -c 1-63
hfButler.py flx-1-p2-hf link config --dr-on --dpr-mux passthrough --drop-empty
hfButler.py flx-1-p2-hf wtor -i 0 config ${EMU_FILE}
hfButler.py flx-1-p2-hf wtor -i 0 fire -l
watch -c "hfButler.py flx-1-p2-hf link watch -RB | grep -v WARNING"

wait
echo "Test Complete!"
echo "quick summary: "
tail -n 4 out_fdaq.log