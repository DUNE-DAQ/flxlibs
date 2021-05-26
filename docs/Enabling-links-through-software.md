By default all e-links and emulator are disabled when programming the FELIX, so you will need to enable them each time you re-program the card. You can do this by either using `flx-config` (needs some testing). You can enable e-links (make sure they are set to FULLMODE), configure the emulator or load a generated config file.

(enabling elinks using `flx-config` to be done)

To configure the emulator with a pre-generated file, use the following command:

```
flx-config -d <device number, 0 or 1> load <pre-generated file>
```

To generate the file, you can use emuConfigGen.cpp which creates a configuration file to play randomly generated packets with varying size and random ADC values (plus other configurable options). (need to upload to repo?)

To know which elinks to enable, first you need to figure out which optical fibres you have connected, and what channel number comes up on the outputs of `flx-info POD` or `flx-info LINK`.  examples of the outputs are given in [this wiki page](https://github.com/DUNE-DAQ/flxlibs/wiki/Checking-Debugging-Optical-Fibre-Status). The mapping between channel number and elink number should be linear i.e. channel 0 on 1st Rx relates to elink 0 on device 0, so figuring out which links to use should be (relatively) straight forward (this is the case with the FELIX firmware `FLX712_FULLMODE_24CH_CLKSELECT_GIT_phase2-FLX-1368_PEXInitForEPYC_rm-5.0_1070_201113_11_29`).

(Note that `flx-info LINK` labels the channels on device 0 "0-11" and "12-23" for device 1, so 2nd Rx channel 0 comes up as channel 12 in the output of `flx-info LINK` etc.)