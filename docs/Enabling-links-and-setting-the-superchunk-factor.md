## Enabling links and setting up the card

You can do this either using the felix UPS product in the dune-daq software or a custom build of the felix software suite provided by ATLAS TDAQ.

To use the UPS product, you can use these tools where you did run:
```
dbt-workarea-env
```

In order to enable links, the corresponding registers need to be set: `DECODING_LINK00_EGROUP0_CTRL_EPATH_ENA`, where LINK marks the link identifier. In order to enable the 10 links on the card (5-5 on the 2 SLRs), do this:

```
echo Disabling every link on SLR 0...
for i in $(seq 0 9); do flx-config -d 0 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=0; done;
for i in $(seq 10 11); do flx-config -d 0 set DECODING_LINK${i}_EGROUP0_CTRL_EPATH_ENA=0; done;
echo Disabling every link on SLR 1...
for i in $(seq 0 9); do flx-config -d 1 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=0; done;
for i in $(seq 10 11); do flx-config -d 1 set DECODING_LINK${i}_EGROUP0_CTRL_EPATH_ENA=0; done;
echo Enable 5-5 links on the 2 SLRs
for i in $(seq 0 4); do flx-config -d 0 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;
for i in $(seq 0 4); do flx-config -d 1 set DECODING_LINK0${i}_EGROUP0_CTRL_EPATH_ENA=1; done;

```

Now you can check the superchunk factor for each elink by listing the register values using `flx-config`:
```
flx-config list | grep SUPER
```
and you will get an output similar to:

```
0x2de0 [RW 07:00]                 SUPER_CHUNK_FACTOR_LINK_00                0x01  number of chunks glued together
0x2df0 [RW 07:00]                 SUPER_CHUNK_FACTOR_LINK_01                0x01  number of chunks glued together
...
```

Be careful, as these register are SuperLogic Region (SLR) based! To change the register values for factor 12, for both SLRs' first 10 virtual links, run the following:
```
for i in $(seq 0 9); do flx-config -d 0 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;
for i in $(seq 0 9); do flx-config -d 1 set SUPER_CHUNK_FACTOR_LINK_0${i}=0x0c; done;

```

and then rerun `flx-config list | grep SUPER` to check all the values changed. The superchunk factor is changed to 0x0c as the readout software is prepared to receive chunks with an aggregation factor of 12.
