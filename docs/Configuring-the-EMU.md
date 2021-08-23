## Configuring the FELIX internal emulator

You can do this either using the felix UPS product in the dune-daq software or a custom build of the felix software suite provided by ATLAS TDAQ.

To use the UPS product, you can use these tools where you did run:
```
dbt-workarea-env
```

to setup the workspace including the ups product.

### Generating a flx-config loadable file
First a configuration file needs to be generated or acquired. These files have lines of data in the 33b format as well as register values which the command `flx-config` uses to configure the internal emulator.

To generate simple configurations (incremental data, constant value etc.), you can run the `flxlibs_emu_confgen` app in flxlibs. By default it produces an emulator configuration files which had incremental data with a chunk size of 464 and 1 idle character between chunks. You can see the specifications of the data generated in the config when running the app, below shows the output after running with default settings.

So if you run:
```
flxlibs_emu_confgen
```

The output should be similar to this:
```
number of lines : 8192
chunk size      : 464
idle characters : 1
pattern type    : 0
output file     : emuconfigreg_464_1_0
```
For information on what can be configured use the `-h/--help` options. Using the default load the emulator with WIB frame sized chunks.

To upload a configuration file to the emulator, use `flx-config` on the following way:
```
flx-config -d <device number, 0 or 1> load <config file>
```
So based on the example above, load the emulator of the 2 SLRs via:

```
flx-config -d 0 load emuconfigreg_464_1_0
flx-config -d 1 load emuconfigreg_464_1_0

```

### Fanout selector between real and emulated links
The fanout selector tool is called `femu` that can toggle the emulator mode on and off.

In order to toggle the fanout selector to use the emulated links instead of the optical links:
```
femu -d 0 -e 
femu -d 1 -e

```

In order to toggle back to real optical links from the MTP connectors, disable the emulator fanout via:
```
femu -d 0 -n
femu -d 1 -n

```
