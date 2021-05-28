# flxlibs - FELIX card software and utilities 
Appfwk DAQModules, utilities, and scripts for the FELIX readout card.

## Building

For the dependencies, you need the `felix` external package that ships a build of a partial set of the ATLAS FELIX Software suite. You'll want to use a work area created using daq-buildtools from the dunedaq-v2.4.0 DAQ Suite or later, otherwise this package won't be in your area's `dbt-settings` file. 

## Disclaimer
The [Initial setup of FELIX page](Initial-setup-of-FELIX.md) doesn't mean to be a replacement for the official FELIX User Manual. For a much more detailed documentation, always refer to that appropriate RegisterMap version and device's user manual, that are found under the official [FELIX Project Webpage](https://atlas-project-felix.web.cern.ch/atlas-project-felix/).

## Configure the FELIX card
Please ensure the following:

1. For the physical setup, please refer to the [Initial setup of FELIX](Initial-setup-of-FELIX.md).

2. For the card configuration, one needs compatible firmware and FELIX configuration sets. The list is under [FELIX Assets](FELIX-assets.md#compatibility_list)

3. Configure the FELIX card, as explained in the [Configure the FELIX card](Configure-the-FELIX-card.md) short manual.

## Examples
After successfully building the package, you can try passing the readout package's `felixreadout-commands.json` to `daq_application`. I.e., assuming you've cloned `readout` into your source area, launch a readout emulation via:

    daq_application -n pippo -c stdin://sourcecode/readout/test/felixreadout-commands.json
    
Then start typing commands as usual. 

