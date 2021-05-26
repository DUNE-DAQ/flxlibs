## What is the "local" driver?
In order to ensure absolute consistency with the software externals, the DUNE readout external packages from the ATLAS FELIX Software Suite are built in-house and deployed as external products. Hence the reason, the driver being a foundation that connects firmware with software, it is also part of this product.

## How to build and start it
With `sudo` rights or as `root` one needs to do the following steps:
```
mkdir /opt/felix
cp -r /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev/felix/v1_1_0 /opt/felix/
cd /opt/felix/v1_1_0/Linux64bit+3.10-2.17-e19-prof/drivers_rcc/src/
make -j
cd ../script
sed -i 's/gfpbpa_size=4096/gfpbpa_size=8192/g' ./drivers_flx_local
./drivers_flx_local start
```

This step will be automated in the future.