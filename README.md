# flxlibs - FELIX card software and utilities 
Appfwk DAQModules, utilities, and scripts for DUNE Upstream DAQ FELIX Readout Software.

## Building

For the dependencies, you need the following external package that ships a build of a partial set of the ATLAS FELIX Software suite:

    udaq_readout_deps v0_0_1

Please modify your `.dunedaq_area` file in your work area, by appending the following item to your `dune_products_dir` set:

    "/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev"

And add the `udaq_readout_deps` package to your `dune_products` set:

    "udaq_readout_deps v0_0_1"


## Examples
After succesfully building the package, from another terminal go to your `workarea` directory and set up the runtime environment:

    setup_runtime_environment
    
After that, launch a readout emaulation via:

    daq_application -c stdin://sourcecode/readout/test/felixreadout-commands.json
    
Then start typing commands as follows.
