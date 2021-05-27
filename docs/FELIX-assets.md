## FELIX apparatus and availability
Testing and development hosts with a FELIX card are the following:
* **CERN NP04**
   * np04-srv-028 - RM5 based test platform & hosting 2 FELIX cards (DS Intel Cascade Lake Gold, SNC2)
   * np04-srv-029 - RM5 based test platform (DS Intel Cascade Lake Gold) 
   * np04-srv-030 - RM5 based test platform (DS Intel Cascade Lake Silver)

* **CERN DT-DI DAQ-lab**
   * epdtdi104 - RM5 based development platform (DS Intel Cascade Lake Gold, SNC2, with PMEM and NVMe)
   * epdtdi105 - RM5 based development platform (DS AMD EPYC with PCIe SSD and NVMe)

* **Bristol Lab**
   * ??? - (Host with WIB to FELIX connectivity)

_DS stands for: Dual Socket with 2 CPUs_

_SNC stands for: Sub-NUMA Clustering. SNC2 means 2 cluster per socket._

<a name="firmware_versions"></a>
## Currently used FELIX firmware versions
Commit hashes, build timestamps can be found based on the name and in the firmware itself.

1. **RM4 ATLAS Phase1 JBSC**: https://cernbox.cern.ch/index.php/s/JyhtS7Odj5rvsFP
   1. This firmware is/was the production firmware in ProtoDUNE-SP.
2. **RM5 ATLAS Phase2**: https://cernbox.cern.ch/index.php/s/7gY7HkSBDsz6lCV 
   1. As on ATLAS FELIX firmware [Espace](https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/Bitfiles_development/FLX712_FULLMODE_24CH_CLKSELECT_GIT_phase2-FLX-1368_PEXInitForEPYC_rm-5.0_1070_201113_11_29.tar.gz) 
   2. Built-in JumboBlock/Superchunk support,
   3. Contains AMD EPYC architecture specific fixes

## Register Map configuration files (SLR config files)
_(SLR stands for: "SuperLogic Region", which represent a "half" physical card a.k.a. logical unit.)_

The location of configuration files for elinks and emulators can be found here: https://cernbox.cern.ch/index.php/s/uUi31McesqajCR6

1. **RM5 configs, ONLY compatible with firmware 2**:
   1. [rm5-emu-5links-slr1](https://cernbox.cern.ch/index.php/s/o1u6HLtpS6yC0OZ/download) - Enables on SLR 5 x FM links and uploads 464B, 1Idle, Incremental pattern emulation. (Fanout selector unlocked, toggle possible between real and EMU data.)
   2. [rm5-emu-5links-slr2](https://cernbox.cern.ch/index.php/s/tUSn0gjehHfvca7/download) - Enables on SLR 5 x FM links and uploads 464B, 1Idle, Incremental pattern emulation. (Fanout selector unlocked, toggle possible between real and EMU data.)

<a name="compatibility_list"></a>
## Compatibility list
This section describes which DUNE-DAQ software and `flxlibs` version are compatible with which firmware and configuration set.

   | DUNE-DAQ      | flxlibs       | `felix` external | Firmware     | Configuration |
   |:-------------:|:-------------:|:----------------:|:------------:|:-------------:|
   |  v2.4.0       | develop       | v1_1_1 e19:prof  | RM5 (2.)     | 1.1 & 1.2     |
