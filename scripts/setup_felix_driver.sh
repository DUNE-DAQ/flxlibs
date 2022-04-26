#!/bin/bash

mkdir -p /opt/felix
cp -r /cvmfs/dunedaq.opensciencegrid.org/products/felix/v1_2_2 /opt/felix/
if [[ `lsb_release -rs` == 8 ]]; then cd /opt/felix/v1_2_2/Linux64bit+4.18-2.28-e19-prof/drivers_rcc/src/; else cd /opt/felix/v1_2_2/Linux64bit+3.10-2.17-e19-prof/drivers_rcc/src/; fi;
make -j
cd ../script
sed -i 's/gfpbpa_size=4096/gfpbpa_size=8192/g' ./drivers_flx_local
./drivers_flx_local stop
./drivers_flx_local start
./drivers_flx_local status &> flx_driver_status.log

