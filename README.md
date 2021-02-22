# flxlibs - FELIX card software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE Upstream DAQ FELIX Readout Software.

## Building

For the dependencies, you need the following external package that ships a build of a partial set of the ATLAS FELIX Software suite:

    udaq_readout_deps v0_0_1

Please modify your `dbt-settings` file in your work area, by enabling the following item to your `dune_products_dir` set:

    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev"

And add the `udaq_readout_deps` package to your `dune_products` set:

    "udaq_readout_deps v0_0_1"

## Configure the FELIX card
   1. For the physical setup, please refer to the the [Initial setup of FELIX Wiki](https://github.com/DUNE-DAQ/flxlibs/wiki/Initial-setup-of-FELIX).
   2. For the card configuration, one needs compatible firmware and FELIX configuration sets. The list is under [FELIX Assets Wiki](https://github.com/DUNE-DAQ/flxlibs/wiki/FELIX-assets:-Firmware-and-config-files#compatility-list)
   3. The manual assumes, that you have low-level FELIX tools in `$PATH`, and `<slr1-config-file>` and `<slr2-config-file>` downloaded to `/tmp`, and the card is programmed with a compatible firmware.

If the points above are succesfully verified, one can do the following from the cloned repository's directory:
    
    ./scripts/ccm-scripts/init.sh <physical-card-id>
    ./scripts/ccm-scripts/linkconfig.sh <physical-card-id> /tmp/<slr1-config-file> /tmp/<slr2-config-file>
    
In case you want to enable the emulator in the FELIX, make sure that the SLR config files contain EMU config. After that, you also need to:

    ./scripts/ccm-scripts/femu-enable.sh <physical-card-id>

From this point, the FELIX dataflow can be started, and `flxlibs` modules can read the memory blocks that contain data produced by the FELIX card.

## Examples
After succesfully building the package, from another terminal go to your `workarea` directory and set up the runtime environment:

    setup_runtime_environment
    
After that, launch a readout emaulation via:

    daq_application -c stdin://sourcecode/readout/test/felixreadout-commands.json
    
Then start typing commands as follows.
