### Intro and prerequisites 
Always ensure that the selected motherboard, CPU and chipset combination is compatible with the FELIX card. If you are unsure, please get in touch with Upstream DAQ developers before you test with a potentially incompatible server. (The FELIX most probably won't work on desktop, but on server grade hardware environments.)

This documentation assumes that you are using an FLX-712 card, on a Linux CentOS 7 operating system. (Instructions might differ for BNL-711 and VC709 boards. Testing with CentOS 8 is ongoing, there is no stable release yet.)

### Installation
1. As presented in the [User Manual's Card Installation section](https://atlas-project-felix.web.cern.ch/atlas-project-felix/user/felix-user-manual/versions/4.0.6/3_hardware_setup.html#subsec:bnl711install), connect the FELIX-712 to a PCIe slot, which is at least Gen3 and x16.

2. **Warning:** The card requires 12V auxiliary power, connected from either the power supply unit (PSU), or directly from the motherboard. This is typically the same cable that feeds power for GPUs. Always check your motherboard's user-manual, as many vendors have proprietary pinouts on the motherboard side!
   * The Supermicro boards usually need this particular cable: SM CBL-PWEX-0665
   * Intel WolfPass (2600WF series) servers need GPU power cable: iPC – AXXGPGPUCABLE
   * DELL PowerEdge R740 01YM03 servers need GPU power cable from the DELL GPU enablement kit (with mini MOLEX to motherboard).

3. Start the machine, and after logging in, depending on the firmware loaded or flashed on the card, the device should be visible under 
   `lspci`. One of the two commands should result with PCIe endpoints visible:
   ```
   lspci | grep Xilinx
   lspci | grep CERN 
   ```
   If there is no output, ensure the following: The card is powered correctly, there is a valid firmware flashed on the partitions.

4. In order to program a different firmware (e.g.: DUNE specific JumboBlocks/SuperChunks variant FULLMODE, that are discussed under the [FELIX Assets](FELIX-assets.md#firmware_versions)) to the FELIX, connect the JTAG programmer to the card, and the USB side to a machine where you have Vivado Labs installed. (For further and in-depth description, please refer to the [User Manual's Firmware Programming section](https://atlas-project-felix.web.cern.ch/atlas-project-felix/user/felix-user-manual/versions/4.0.6/4_firmware_programming.html#_4_2_firmware_programming)! The dunedaq FELIX dependency package is shipped with the `fflashprog` tool.) After programming, reboot the machine in order for the firmware in the volatile memory to load:
   ```
   reboot
   ```

5. The dunedaq FELIX dependency package comes with the local driver, which is favorable to be used. The details of that is discussed under the [Local driver](https://github.com/DUNE-DAQ/flxlibs/wiki/Local-driver) readme. Installing the ATLAS TDAQ driver has a well detailed explanation under [User Manual's Driver Installation section](https://atlas-project-felix.web.cern.ch/atlas-project-felix/user/felix-user-manual/versions/4.0.6/5_software_installation.html#_5_2_1_driver_rpm_installation_instructions) User Manual's Driver Installation section.
Either of them needs `dkms` and `kernel-devel` installed on your system. The driver has 3 main kernel modules: `io_rcc`, `cmem_rcc` and `flx`. All three needs to show meaningful output, without errors, on the status output (see Local Driver doc).

6. Acquire the FELIX software suite with the appropriate version that is usable with your loaded firmware and driver, then ensure that low-level functionalities work as expected. One can use the `felix` DUNE DAQ external package that contains these low level tools. 
Preferably, acquire all the following tools' output:

    * flx-info
    * flx-init
    * flx-dma-test

