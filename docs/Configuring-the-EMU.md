This is a short set of instruction to exercise the internal FLX emulator:
* After this you will know how to manipulate the internal emulator, load and verify some data coming out of it.

You either use the felix UPS product in the dune-daq software or a custom build of the felix software suite provided by ATLAS tDAQ.

To use the ups product make sure to setup your DUNE-DAQ workspace following the instructions [here](https://github.com/DUNE-DAQ/minidaqapp/wiki/Instructions-for-setting-up-a-v2.6.0-development-environment).

You also need to make sure that the felix ups product is part of dbt-settings e.g. `"felix  v1_1_2  e19:prof"`
in `dune_externals`.

Then run:
```
dbt-workarea-env
```

to setup the workspace including the ups product.

---
First a configuration file needs to be generated or acquired. These files have lines of data in the 33b format as well as register values which the command `flx-config` uses to configure the internal emulator.

To generate simple configurations (incremental data, constant value etc.), you can run the `flxlibs_emu_confgen` app in flxlibs. By default it produces an emulator configuration files which had incremental data with a chunk size of 464 and 1 idle character between chunks. You can see the specifications of the data generated in the config when running the app, below shows the output after running with default settings:
```
number of lines : 8192
chunk size      : 464
idle characters : 1
pattern type    : 0
output file     : emuconfigreg_464_1_0
```
for information on what can be configured use the `-h/--help` options (NOTE: changing the number of lines from the default value may cause errors to occur when reading out using `fdaq`).

More complex configuration files which allows the emulator to play 64 wib frames in 33b format can be found here: [dtp-patterns/EMUInput](https://gitlab.cern.ch/dune-daq/readout/dtp-patterns/-/tree/master/EMUInput).

---

By default all e-links and emulator are disabled when programming the FELIX, so you will need to enable them each time you re-program the card. You can do this by either using `flx-config` or using FLX tool _elinkconfig_. You can enable e-links (make sure they are set to FULLMODE), configure the emulator or load a generated config file.

**Note to use the internal emulator make sure at least one elink is enabled for the specific SLR. The data rate will be proportional to the number of links enabled.**

Make sure the emulator is enabled using the following command:
```
femu -d <device number, 0 or 1> -n/e < disable/enable the emulator>
```
for more options:
```
femu --help
```

To upload a configuration file to the emulator:
```
flx-config -d <device number, 0 or 1> load <config file>
```
The output then can be checked using the command `fdaq`, the guide to using `fdaq` can be found [here](https://github.com/DUNE-DAQ/flxlibs/wiki/Using-fdaq).

---
Here is an example of generating the default config file, loading it into the internal emulator and using `fdaq` to play the data from the emulator:

```
femu -d 0 -e
```
```
FullMode Emulator, FROMHOST_FANOUT=00000000 LOCK=0, TOHOST_FANOUT=00ffffff LOCK=0
```

```
flxlibs_emu_confgen
```
```
number of lines : 8192
chunk size      : 464
idle characters : 1
pattern type    : 0
output file     : emuconfigreg_464_1_0
```
```
flx-config -d 0 load emuconfigreg_464_1_0
```
```
fdaq -d 0 -t 5 -e
```
```
Consume FLX-device data while checking the data (blockheader and trailers),
counts errors including chunk truncation, halts when the memory buffer is near overflowing.
Also counts chunk CRC errors.
Opened FLX-device 0, firmw FLX712-FM-6chan-2106091300-GIT:rm-5.0/1577, trailer=32bit block=4K, buffer=1024MB
**START(emulator)** using DMA #0 polling
  Secs | Recvd[MB/s] | File[MB/s] | Total[(M)B] | Rec[(M)B] | Buf[%] | Wraps
-------|-------------|------------|-------------|-----------|--------|-------
     1         929.0          0.0         929.0           0        1       0
     2         929.3          0.0        1858.3           0        1       1
     3         931.7          0.0        2790.0           0        2       2
     4         929.3          0.0        3719.3           0        0       3
     5         931.4          0.0        4650.7           0        0       4
**STOP**
-> Data checked: Blocks 1134653, Errors: header=0 trailer=0
Exiting..
```

you can see Recvd [MB/s] is around 930 due to only one elink was enabled.
 