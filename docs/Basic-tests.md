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
