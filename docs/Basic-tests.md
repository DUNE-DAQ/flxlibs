## Basic tests

### Low level FELIX tools
`fdaq` is a tool that reads data from the FELIX card, and is provided by FELIX software suite. For running 5 seconds the emulator on SLR0, do the following:

```
fdaq -e -d 0 -t 10
```
For checking the second half of the card, you can read SLR1 with:

```
fdaq -e -d 1 -t 10
```

The output should be similar to this:
```
[np04daq@np04-srv-030 dunedaq]$ fdaq -e -d 0 -t 5
Consume FLX-device data while checking the data (blockheader and trailers),
counts errors including chunk truncation, halts when the memory buffer is near overflowing.
Also counts chunk CRC errors.
Opened FLX-device 0, firmw FLX712-FM-6chan-2106091300-GIT:rm-5.0/1577, trailer=32bit block=4K, buffer=1024MB
**START(emulator)** using DMA #0 polling
  Secs | Recvd[MB/s] | File[MB/s] | Total[(M)B] | Rec[(M)B] | Buf[%] | Wraps
-------|-------------|------------|-------------|-----------|--------|-------
     1        4642.9          0.0        4642.9           0       10       4
     2        4657.8          0.0        9300.7           0        8       8
     3        4657.9          0.0       13958.6           0        3      12
     4        4652.8          0.0       18611.3           0        2      17
     5        4654.0          0.0       23265.3           0        1      21
**STOP**
-> Data checked: Blocks 5680005, Errors: header=0 trailer=0
Exiting..
```

### flxlibs applications
From this point, the FELIX dataflow can be started, and `flxlibs` modules can read the memory blocks that contain data produced by the FELIX card. In order to be sure that the configuration step was successful, one can try to run the `CardWrapper` test app:

```
flxlibs_test_cardwrapper
```
The expected output is something similar to the following:

```
(dbt-pyvenv) [epdtdi@epdtdi104 minidaqapp]$ flxlibs_test_cardwrapper 
2021-Mar-03 20:47:38,399 LOG [main(...) at test_cardwrapper_app.cxx:42] Creating CardWrapper...
2021-Mar-03 20:47:38,399 LOG [main(...) at test_cardwrapper_app.cxx:34] Application will terminate in 5s and show encountered ELink IDs in BLOCK headers...
2021-Mar-03 20:47:38,406 LOG [main(...) at test_cardwrapper_app.cxx:62] Init CardWrapper...
2021-Mar-03 20:47:38,406 LOG [main(...) at test_cardwrapper_app.cxx:65] Configure CardWrapper...
2021-Mar-03 20:47:38,406 LOG [CardWrapper::configure(...) at CardWrapper.cpp:83] Configuring CardWrapper of card [id:0 slr:0]
2021-Mar-03 20:47:38,413 LOG [main(...) at test_cardwrapper_app.cxx:68] Start CardWrapper...
2021-Mar-03 20:47:38,413 LOG [main(...) at test_cardwrapper_app.cxx:71] Flipping killswitch in order to stop...
2021-Mar-03 20:47:43,407 LOG [main(...) at test_cardwrapper_app.cxx:76] Stop CardWrapper...
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:79] Number of blocks DMA-d: 5767168-> Per elink: 
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(0): 1153433
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(64): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(128): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(192): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(256): 1153433
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:85] Exiting.
```

The critical part that indicates success is the following:
```
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:79] Number of blocks DMA-d: 5767168-> Per elink: 
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(0): 1153433
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(64): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(128): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(192): 1153434
2021-Mar-03 20:47:43,427 LOG [main(...) at test_cardwrapper_app.cxx:82]   elink(256): 1153433
```

If you observe different values, the EMU data might not be configured properly.
