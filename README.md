# flxlibs - FELIX card software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE Upstream DAQ FELIX Readout Software.

## Building

For the dependencies, you need the `felix` external package that ships a build of a partial set of the ATLAS FELIX Software suite. Please modify your `dbt-settings` file in your work area, by enabling the following item to your `dune_products_dir` set:

    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev"

And add the following line to your `dune_externals` set:

    "felix v1_1_1 e19:prof"

## Configure the FELIX card
Please ensure the following:
   1. For the physical setup, please refer to the the [Initial setup of FELIX Wiki](https://github.com/DUNE-DAQ/flxlibs/wiki/Initial-setup-of-FELIX).
   2. For the card configuration, one needs compatible firmware and FELIX configuration sets. The list is under [FELIX Assets Wiki](https://github.com/DUNE-DAQ/flxlibs/wiki/FELIX-assets:-Firmware-and-config-files#compatility-list)
   3. Configure the FELIX card, as explained in the [Configure the FELIX card](https://github.com/DUNE-DAQ/flxlibs/wiki/Configure-the-FELIX-card) short manual.

## Examples
After succesfully building the package, from another terminal go to your `workarea` directory and set up the runtime environment:

    setup_runtime_environment
    
After that, launch a readout emaulation via:

    daq_application -c stdin://sourcecode/readout/test/felixreadout-commands.json
    
Then start typing commands as follows.
