This section is important if you have any optical fibres connected to the FELIX, either to another device (WIB, ZCU, etc.) or if you have the FELIX fibres in loopback.

Firstly, most of this relates to Rx fibres (fibres which receive light) and the number of fibres the firmware supports depends on the firmware (24 or 12). Whenever you plug/unplug fibres run the commands

```
flx-init; flx-reset GTH; flx-info POD; flx-info LINK
```

where the first two commands `flx-init; flx-reset GTH` resets the state of the card, and the second two display information about the links and channel alignment.

---

To check for LOS and to monitor the miniPODs run

```
flx-info POD
```

which will produce a similar output to

```
MiniPODs status
------------------------
Only the 4 active MiniPODs will be shown
NOTE: The MiniPODs of both devices will be shown

               |  1st TX |  1st RX |  2nd TX |  2nd RX |
               |=========|=========|=========|=========|
Temperature [C]|      60 |      59 |      60 |      50 |
3.3 VCC     [V]|    3.28 |    3.32 |    3.26 |    3.30 |
2.5 VCC     [V]|    2.41 |    2.41 |    2.41 |    2.44 |

How to the read the table below:
# = FLX link endpoint OK     (no LOS)
- = FLX link endpoint not OK (LOS)
First letter:  Current channel status
Second letter: Latched channel status
Example: #(-) means channel had lost the signal in the past but the signal is present now.

Latched / current link status of channel:
        |   0  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |  10  |  11  |
        |======|======|======|======|======|======|======|======|======|======|======|======|
 1st TX | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) |
 1st RX | -(-) | -(-) | #(#) | #(#) | #(#) | #(#) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) |
 2nd TX | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) |
 2nd RX | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) |

Optical power (rx or tx) of channel in uW:
        |     0   |     1   |     2   |     3   |     4   |     5   |     6   |     7   |     8   |     9   |    10   |    11   |
        |=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|
 1st TX | 1065.50 |  999.70 | 1035.70 | 1071.10 | 1066.10 | 1062.80 | 1045.00 | 1094.30 | 1072.90 | 1015.00 | 1128.80 | 1125.40 |
 1st RX |    0.00 |   86.70 |  703.70 |  649.20 |  692.30 |  707.70 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |
 2nd TX | 1039.90 |  906.00 |  963.40 |  933.50 |  952.90 |  927.90 |  966.90 |  891.60 |  949.60 |  899.70 |  965.00 |  841.10 |
 2nd RX |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |

                |         1st TX |         1st RX |         2nd TX |         2nd RX |
                |================|================|================|================|
Name            | AVAGO          | AVAGO          | AVAGO          | AVAGO          |
OUI             | 0x00 0x17 0x6a | 0x00 0x17 0x6a | 0x00 0x17 0x6a | 0x00 0x17 0x6a |
Part number     | AFBR-814FN1Z   | AFBR-824FN1Z   | AFBR-814FN1Z   | AFBR-824FN1Z   |
Revision number | 32 32          | 32 32          | 32 32          | 32 32          |
Serial number   | A19105017      | A190340B8      | A1910501M      | A190340A5      |
Date code       | 2019030        | 2019012        | 2019030        | 2019012        |
```

So here, the main things to look out for are the latched status of the channel:

```
How to the read the table below:
# = FLX link endpoint OK     (no LOS)
- = FLX link endpoint not OK (LOS)
First letter:  Current channel status
Second letter: Latched channel status
Example: #(-) means channel had lost the signal in the past but the signal is present now.

Latched / current link status of channel:
        |   0  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |  10  |  11  |
        |======|======|======|======|======|======|======|======|======|======|======|======|
 1st TX | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) |
 1st RX | -(-) | -(-) | #(#) | #(#) | #(#) | #(#) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) |
 2nd TX | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) | #(#) |
 2nd RX | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) | -(-) |
```

and the optical power of the fibres:

```
Optical power (rx or tx) of channel in uW:
        |     0   |     1   |     2   |     3   |     4   |     5   |     6   |     7   |     8   |     9   |    10   |    11   |
        |=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|=========|
 1st TX | 1065.50 |  999.70 | 1035.70 | 1071.10 | 1066.10 | 1062.80 | 1045.00 | 1094.30 | 1072.90 | 1015.00 | 1128.80 | 1125.40 |
 1st RX |    0.00 |   86.70 |  703.70 |  649.20 |  692.30 |  707.70 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |
 2nd TX | 1039.90 |  906.00 |  963.40 |  933.50 |  952.90 |  927.90 |  966.90 |  891.60 |  949.60 |  899.70 |  965.00 |  841.10 |
 2nd RX |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |    0.00 |
```

On both tables there are two sets of TX/RX channels. The 1st set are channels on device 0 and 2nd set are on device 1.  The output shown is for the Bristol test stand with fibres from the QSFP of the ZCU102 connected to channels 2, 3, 4 and 5 so note the optical power. If the power is quite low (sub 100 uW) then it is likely the optical fibres are not connected properly, or the MTP is not seated correctly (double check cables!).

---

To check the link alignment status

```
flx-info LINK
```

and the output should look something like

```
Card type: FLX-712

Link alignment status
------------------------
(entire FLX-712/711):
Channel |  0    1    2    3    4    5    6    7    8    9   10   11
        ------------------------------------------------------------
Aligned | NO   NO   YES  YES  YES  YES  NO   NO   NO   NO   NO   NO
Channel | 12   13   14   15   16   17   18   19   20   21   22   23
        ------------------------------------------------------------
Aligned | NO   NO   NO   NO   NO   NO   NO   NO   NO   NO   NO   NO
```

So again the output is for the setup as described above. Channels 0-11 relate to device 0 and 12-23 relate to device 1. For each Rx fibre with an input connected you should see the channel aligned,  flagged by `YES`. If not, then then channel alignment is lost along one of the fibres. This occurs if fibres are disconnected, the data source connected is not streaming idle characters (K28.5) at the correct rate (9.618Gbit/s). If you make changes to the data source then always check the links are aligned, if they are not then run through the commands `flx-init; flx-reset GTH; flx-info POD; flx-info LINK` again.