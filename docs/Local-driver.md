## What is the "local" driver?
In order to ensure absolute consistency with the software externals, the DUNE readout external packages from the ATLAS FELIX Software Suite are built in-house and deployed as external products. Hence the reason, the driver being a foundation that connects firmware with software, it is also part of this product.

## Support for CentOS 8 Stream
At the moment there is no available `felix` product that has driver pathes for kernel 4.10+ and mainline 5.10+. The new `felix` product release will have the necessary patches for the new and with backward compatibility for older kernels.

## How to build and start it
With `sudo` rights or as `root` one needs to do the following steps:
```
mkdir /opt/felix
cp -r /cvmfs/dunedaq.opensciencegrid.org/products/felix/v1_2_0 /opt/felix/
cd /opt/felix/v1_2_0/Linux64bit+3.10-2.17-e19-prof/drivers_rcc/src/
make -j
cd ../script
sed -i 's/gfpbpa_size=4096/gfpbpa_size=8192/g' ./drivers_flx_local
./drivers_flx_local stop
./drivers_flx_local start
./drivers_flx_local status
```

The expected output is similar to this:

```
[root@host script]# ./drivers_flx_local status
cmem_rcc             8431913  0 

>>>>>> Status of the cmem_rcc driver


CMEM RCC driver (FELIX release 4.5.0)

The driver was loaded with these parameters:
gfpbpa_size    = 8192
gfpbpa_quantum = 4
gfpbpa_zone    = 0
numa_zones     = 1

alloc_pages and alloc_pages_node
   PID | Handle |         Phys. address |               Size | Locked | Order | Type | Name

GFPBPA (NUMA = 0, size = 8192 MB, base = 0x00000013b1000000)
   PID | Handle |         Phys. address |               Size | Locked | Type | Name
 
The command 'echo <action> > /proc/cmem_rcc', executed as root,
allows you to interact with the driver. Possible actions are:
debug    -> enable debugging
nodebug  -> disable debugging
elog     -> Log errors to /var/log/messages
noelog   -> Do not log errors to /var/log/messages
freelock -> release all locked segments
io_rcc                 21598  0 

>>>>>> Status of the io_rcc driver

IO RCC driver for release tdaq831_for_felix (based on tag ROSRCDdrivers-00-01-00)
Dumping table of linked devices
Handle | Vendor ID | Device ID | Occurrence | Process ID
 
The command 'echo <action> > /proc/io_rcc', executed as root,
allows you to interact with the driver. Possible actions are:
debug   -> enable debugging
nodebug -> disable debugging
elog    -> Log errors to /var/log/messages
noelog  -> Do not log errors to /var/log/messages
Current values of the parameter(s)
debug    = 0
errorlog = 1
flx                    43300  0 

>>>>>> Status of the flx driver 

FLX driver 4.5.0 for RM4 F/W only

Debug                         = 0
Number of devices detected    = 2


Locked resources
      device | global_locks
=============|=============
           0 |   0x00000000
           1 |   0x00000000

Locked resources
device | resource bit |     PID |  tag
=======|==============|=========|=====

Error: Device 0 does not have the required F/W. The regmap register contains 0x00000500

Error: This version of the driver is for regmap 4.0

Error: Device 1 does not have the required F/W. The regmap register contains 0x00000500

Error: This version of the driver is for regmap 4.0
 
The command 'echo <action> > /proc/flx', executed as root,
allows you to interact with the driver. Possible actions are:
debug     -> Enable debugging
nodebug   -> Disable debugging
elog      -> Log errors to /var/log/message
noelog    -> Do not log errors to /var/log/message
swap      -> Enable automatic swapping of 0x7038 / 0x7039 and 0x427 / 0x428
noswap    -> Disable automatic swapping of 0x7038 / 0x7039 and 0x427 / 0x428
clearlock -> Clear all lock bits (Attention: Close processes that hold lock bits before you do this)
```

Please ignore the last errors which are complaining about regmap missmatch.

This step will be automated in the future.
