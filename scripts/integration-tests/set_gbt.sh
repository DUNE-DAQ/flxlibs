#!/bin/bash
set -e # exit if any command exits with non-zero status

### enables hitfinding TPG in GBT mode ###

hfButler.py flx-0-p2-hf init
hfButler.py flx-1-p2-hf init
hfButler.py flx-0-p2-hf cr-if --on --drop-empty
hfButler.py flx-1-p2-hf cr-if --on --drop-empty

# set source to gbt i.e. process data from the elinks
hfButler.py flx-0-p2-hf flowmaster --src-sel gbt --sink-sel hits
hfButler.py flx-1-p2-hf flowmaster --src-sel gbt --sink-sel hits

# set the threshold of the hitfinder
hfButler.py flx-0-p2-hf link hitfinder -t 20
hfButler.py flx-1-p2-hf link hitfinder -t 20

# mask all but channel 0
hfButler.py flx-0-p2-hf link mask enable -c 1-63
hfButler.py flx-1-p2-hf link mask enable -c 1-63

hfButler.py flx-0-p2-hf link config --dr-on --dpr-mux passthrough --drop-empty
hfButler.py flx-1-p2-hf link config --dr-on --dpr-mux passthrough --drop-empty

if [[ "$1" = "watch-0" ]]
then
    #watch -c "hfButler.py flx-0-p2-hf link watch -RB | grep -v WARNING"
    watch -c "hfButler.py flx-0-p2-hf link -i 4 watch -RB | grep -v WARNING"
fi

if [[ "$1" = "watch-1" ]]
then
    #watch -c "hfButler.py flx-1-p2-hf link watch -RB | grep -v WARNING"
    watch -c "hfButler.py flx-1-p2-hf link -i 4 watch -RB | grep -v WARNING"
fi