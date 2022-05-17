## Felix Firmware Integration Testing status
 [flx712-2.1.0 Testing status](https://docs.google.com/spreadsheets/d/1wXz7qx9HzWGdVpDBZSh985ecvFOZMB51oAWpZzaUU-k/edit#gid=1776651850)

## Initial setup
---

### Firmware:
Download a copy of the FELIX firmware [here](https://dune-fw.web.cern.ch/dune-fw/?dir=felix-pie/tags/v2.0.0/latest).

*Note, to get the 10 link build with the TPG firmware download the file starting with `flx712-dune-ipbus-pod-10hf`*

To program the FELIX card, you can use Vivado if you have not setup a dunedaq envirnomnent (see below), otherwise, you can use the low level tool `fflashprog` to flash the correct partition on the card. Each method uses a different file type, both of which are provided in the download.

#### Vivado:
To program with Vivado, you need to ensure you have X-11 forwarding or a VNC if you are connecting to the machine remotely, a copy of the .bit file of the firmware and can run as `sudo` or `root` on the machine you are working on. Run vivado:
```bash
vivado
``` 

and a GUI should appear, under Tasks select the option "Open Hardware Manager", then "tools" -> "open new target". This will open a window where you can connect to hardware locally or via a remote server. Assuming this is the machine with the FELIX card installed connect to local server and in the next window look for a hardware device (bottom table) with the device name including `xkcu115`. Click next then finish. You should see the device appear on the hardware tab on the left. Now click "tools" -> "program device" -> "xkcu115" and then specify the bitstream (.bit) file and click program. I the programming worked then you should see no errors in the tcl console (some warnings may appear but can be ingored), and now you can close vivado and reboot the machine with the FELIX card:

```bash
sudo reboot
```

#### `fflashprog`:
To program with `fflashprog` ensure you have a copy of the .mcs file of the firmware, already have a working dunedaq environment or a copy of the low level tools and can run as `sudo` or `root` on the machine you are working on. Then run the commands:

```bash
fflashprog -c 0 -f 3 <.mcs file to program> prog
```
to program the FELIX card (card number denoted by `-c`). Then to check if the firmware is programmed with the correct firmware:
```bash
fflashprog -c 0 -f 3 <.mcs file to program>
```
Finally reboot the machine with the FELIX card:
```bash
sudo reboot
```
*Note, the -f command indicates which partition to program, if programming on partition 3 doesn't work, run `flx-info` to get the correct partition number or program the card using vivado.*

### Drivers:

To install drivers for the FELIX card follow the instructions to setup local drivers:

https://github.com/DUNE-DAQ/flxlibs/wiki/Local-driver

Ensure the FELIX is programmed, the first check is to enable and get the status of the drivers:

```bash
sudo <location of local drivers>/drivers_flx start
sudo <location of local drivers>/drivers_flx status
```

if you see the following output after starting the drivers:

```bash
Starting cmem driver 
major number for cmem_rcc is 238

Starting io_rcc driver 
major number for io_rcc is 237

2 flx PCIe endpoints found
Starting flx driver 
major number for flx is 236
creating node /dev/flx0
creating node /dev/flx1
```

then it has successfully worked, if you see errors, then reboot the machine and try again.

*Note you can ignore error messeges about regmap verision mismatches*

### DUNEDAQ software:

First ensure you have setup a dunedaq enivronmet, following these instructions:
[https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/#running-a-release-from-cvmfs](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/#running-a-release-from-cvmfs)

Now to get access to the integration test bash scripts clone the flxlibs repository:
```bash
git clone https://github.com/DUNE-DAQ/flxlibs.git
```

Clone dtp-pattens in your working directory, a repository containing fake data patterns to test with the FELIX:
```bash
git clone ssh://git@gitlab.cern.ch:7999/dune-daq/readout/dtp-patterns.git
```

### DTP software (only for dunedaq versions less then v2.10.0):
To test hitfinding with the appropriate firmware, `dtp-controls` and `wupper-toybox` are needed to control the TPG (***Note `wupper-toybox` to be replaced with `DUNE-DAQ/uhallibs`***).

In your `sourcecode` directory, clone `wupper-toybox`:
```bash
git clone ssh://git@gitlab.cern.ch:7999/dune-daq/readout/wupper-toybox.git
```

and make a clean build of the dunedaq software:
```bash
dbt-build.sh -c
```

Next, clone `dtp-controls` in the working directory:
```bash
git clone ssh://git@gitlab.cern.ch:7999/dune-daq/readout/dtp-controls.git -b aearle/felixSupport
```

to begin using `dtp-controls`:
```bash
source dtp-controls/env.sh
```

Each time you open a new session, alongside setting up the dunedaq environment you must also setup `dtp-controls`, so the full set of startup commands are:

```bash
source /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh
setup_dbt dunedaq-v2.8.2
dbt-workarea-env
source dtp-controls/env.sh
```

## Running tests
### test dataflow with low level tools:
Integration test scripts are located in `flxlibs/scripts/integration-tests`
#### basic card communication
run the bash script:
```bash
./test-communication.sh
```
which will test the capability to reset the state of the FELIX, monitor optical links and configuring the card e.g. enabling elinks.

#### test dataflow of ADC links using the internal emulator
run the bash script:
```bash
./test-adc-dataflow-emu.sh
```
which will configure the card to play data from the internal emulator on the ADC links. this will run the fdaq command for each SLR. If the test is successful you should see a non-zero data rate received by the FELIX.

#### test dataflow of ADC links with optical links
to test receiving data over optical links you need to first run the following command:
```bash
./test-adc-dataflow-fe.sh
```
then in a second terminal you must enable the front end device to play data (include zcu102 instructions here?). If the test is successful you should see a non-zero data rate once the front end device is configured.

#### test dataflow of TP links with the wibulator
First run on a SLR 0 and 1 separately:
```bash
./test-tpg-dataflow-wib-slr0.sh
./test-tpg-dataflow-wib-slr1.sh
```
This will run `fdaq`, then configure the firmware TPG with `hfButler.py` and then spy on the link processors. if successful you should see the link processor state change over time for ~60s. After 60s the page will stop updating and you can safely kill the process (ctrl-C). Note you will also get an output file which logs the output of `fdaq`, as well as a binary file with the dumped TP's processed by the card.

Now run with both SLR's simultaneously:
```bash
./test-tpg-dataflow-wib.sh
```
This will run `fdaq` for each SLR for 60s and as before a log file plus binary file of dumped tp's are produced.

#### test dataflow of TP links with the internal FELIX emulator
Run the bash script
```bash
./test-tpg-dataflow-emu.sh
```
this will configure the card to play data from the internal emulator, run `fdaq` then configure the hitfinding tpg in gbt mode i.e. process data from elinks. The outputs of fdaq are stored in log files. if successful you should see the data rate increase twice, once when the adc data is received and then again when the tpg is configured.

#### test dataflow of TP links with optical links
First run the bash script
```bash
./test-tpg-dataflow-fe.sh
```
then configure the front end to produce data. now in another terminal run the bash script:
```bash
./set_gbt.sh
```
which will configure the hitfinding tpg to process data from the elinks. this will produce an output file for each `fdaq` run and if you want to monitor the link processors of the tpg then run `set_gbt.sh` with these arguements instead:
```bash
./set_gbt.sh watch-0 # watch processor on SLR 0
```
```bash
./set_gbt.sh watch-1 # watch processor on SLR 1
```
### test readout with DUNE-DAQ
***to correctly generate the configurations in dunedaq-v2.11.0, the develop branch of flxlibs is required:***
```bash
cd sourcecode
git clone https://github.com/DUNE-DAQ/flxlibs.git
dbt-workarea-env
dbt-build.py
cd ..
```
#### Test ADC recording with the internal FELIX emulator
to generate a 10 ADC link configuration run the following command:
```bash
python -m flxlibs.app_confgen -n 10 -t 0 -m "0-4:0-4" -e -E app_flx_10.json
```
where `-e` enables generation of fake timestamps when using fake data patterns and `-E` enables the internal emulator.
Then run the following:
```bash
./test-communication.sh
daq_application -c stdin://app_flx_10.json -n test_emu
```
once in the daq_application run the commands:
```
init conf start
```
and you should see dataflow on the datalinkhandlers. the key is to look for non-zero values in the following flags:
```
num_blocks_processed
num_chunks_processed
num_subchunks_processed
rate_blocks_processed
rate_chunks_processed
```
and the following should be zero if there is no errors
```
num_blocks_processed_with_error
num_chunks_processed_with_error
num_short_chunks_processed_with_error
num_subchunk_crc_errors
num_subchunk_errors
num_subchunk_trunc_errors
num_subchunks_processed_with_error
```
Note that during the start of dataflow some errors can occur, but this is due to data being readout between the start and end of a chunk. After ~10s the window should update and show no errors.

To stop the test run the following:
```
stop
scrap
```
then exit the program with Ctrl-D then Ctrl-C.
#### Test ADC recording with the optical links
run the following command:
```bash
python -m flxlibs.app_confgen -n 10 -t 0 -m "0-4:0-4" -e app_flx_10.json
```
which prevents the internal emulator from running.
Then run the following:
```bash
./test-communication.sh
femu -d 0 -n
femu -d 1 -n
daq_application -c stdin://app_flx_10.json -n test_fe
```
once in the daq_application run the commands:
```
init conf start
```
and here you should see no dataflow on any link handlers. Now configure the front end device to send data along the optical fibers and you should see the link handler information update similar to the previous test.

To stop the test run the following:
```
stop
scrap
```
then exit the program with Ctrl-D then Ctrl-C.
#### Test TP dataflow with the wibulator
run the following command:
```bash
python -m flxlibs.app_confgen -n 0 -t 2 -m "5:5" -e app_flx_2.json
```
run the daq_app as before:
```bash
./test-communication.sh
femu -d 0 -n
femu -d 1 -n
daq_application -c stdin://app_flx_2.json -n test_wib
init conf start
```
and then enable the wibulator in another terminal:
```bash
./set_wibulator.sh
```
and you can spy on the link processors by adding either `watch-0` or `watch-1` as an argument.
if successful you should see dataflow on the two link handlers.

To stop the test run the following:
```
stop
scrap
```
then exit the program with Ctrl-D then Ctrl-C.
#### Test TP dataflow with the internal emulator
run the following command:
```bash
python -m flxlibs.app_confgen -n 10 -t 2 -m "0-5:0-5" -e -E app_flx_12.json
```
run the daq_app as before:
```bash
./test-communication.sh
daq_application -c stdin://app_flx_12.json -n test_emu
init conf start
```
now enable the hitfinding TPG in GBT mode:
```bash
./set_gbt.sh
```
and you should see dataflow on link handlers 5 and 11. ***Note the TP links which have a smaller data rate than the adc links.***

To stop the test run the following:
```
stop
scrap
```
then exit the program with Ctrl-D then Ctrl-C.
#### Test TP dataflow with the optical links
run the following command:
```bash
python -m flxlibs.app_confgen -n 10 -t 2 -m "0-5:0-5" -e app_flx_12.json
```
run the daq_app as before:
```bash
./test-communication.sh
femu -d 0 -n
femu -d 1 -n
daq_application -c stdin://app_flx_12.json -n test_fe
init conf start
```
Next configure the front end to send data. Once done the adc links should have non zero data rates. 

Now enable the hitfinding TPG in GBT mode:
```bash
./set_gbt.sh
```
and you should see dataflow on link handlers 5 and 11.

To stop the test run the following:
```
stop
scrap
```
then exit the program with Ctrl-D then Ctrl-C.

### Test TP dataflow with `nanorc`
***First ensure you are able to ssh into the machine you are working in, as nanorc will run the various DAQ modules as localhosts.***

Create the following nanorc configuration:
```bash
daqconf_multiru_gen -f -e --host-ru localhost --region-id 0 --host-df localhost -o . -s 1 --number-of-data-producers 10 -t 0 --enable-firmware-tpg --enable-raw-recording --enable-tpset-writing --trigger-activity-config 'dict(prescale=500)' --trigger-candidate-config 'dict(prescale=20)' --tpg-channel-map ProtoDUNESP1ChannelMap flx-fw-json
```
to raw record data in nanorc you must create a file `record-cmd.json` with the following contents:
```json
{
    "data": {
        "modules": [
            {
                "data": {
                    "duration": 1
                },
                "match": ""
            }
        ]
    },
    "id": "record"
}
```
Now, depending on the tpye of data to readout you can follow the above steps to configure the FELIX a certain way. Here we will show how to readout data from the internal emulator.

In a new terminal (with the dunedaq environment setup), configure the card:
```bash
./test-communication.sh
```

In another terminal run nanorc:
```bash
nanorc flx-fw-json/ boot init conf start resume wait 60 stop scrap terminate
```

Which will boot the localhosts, initilise and configure the card, start dataflow, wait 60s stop and terminate the program.
During the wait period (a small loading bar will appear), configure the firmware tpg to process data from the elinks:
```bash
./set_gbt.sh
```
and once this is done you can record data for 1s using this command:
```bash
curl -d "@record.json" -H "Content-Type: application/json" -H "X-Answer-Port: 9876" -X POST <localhost-url>:3336/command
```
note if you are unsure of your local host url it will appear during the boot process of nanorc e.g. http://np04-srv-30:3336/

if you are unable to run the above commands in time then extend the wait period, and if you want to record for more than 1s you can modify record-cmd.json **but be warned, the data rate is very high**.
if successful, you will see output files for each link with a rather with file sizes of ~1GB