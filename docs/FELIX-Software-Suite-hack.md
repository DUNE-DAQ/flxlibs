## FELIX sw suite distribution notes
[DUNE FELIX Distribution Notes](files/DUNE_FELIX_Distribution_list.pdf "DUNE FELIX Distribution Notes")

## Extract minimal set instructions
1. Clone the following repository, and `cd` to it. All the commands below should executed in the same sequence, otherwise it won't work.
   [https://gitlab.cern.ch/atlas-tdaq-felix/software](https://gitlab.cern.ch/atlas-tdaq-felix/software)

2. To clone the repositories needed for the minimal set, download the `clone_all-minimal.sh` script:
      ```
      wget https://raw.githubusercontent.com/DUNE-DAQ/flxlibs/develop/scripts/sw-suite/clone_all-minimal.sh
      chmod +x clone_all-minimal.sh
      ```
   
   Then clone the repositories by running the script:
   
   `./clone_all-minimal.sh ssh`

   Or if you have a kerberos ticket:
   
   `./clone_all-minimal.sh krb5`

3. Checkout appropriate branches. (For now, the rm5 ones):
   ```
   ./checkout_all.sh rm-5.0
   ```

3. Setup FELIX environment:
   ```
   source cmake_tdaq/bin/setup.sh x86_64-centos7-gcc8-opt
   ```

4. Wget the "hack" script from this repository, that overrides the CMake and GCC versions:
   ```
   wget https://raw.githubusercontent.com/DUNE-DAQ/flxlibs/develop/scripts/sw-suite/change-compiler.sh
   wget https://raw.githubusercontent.com/DUNE-DAQ/flxlibs/develop/scripts/sw-suite/copy-minimal.sh
   chmod +x copy-minimal.sh
   ```

5. Do the following steps:
   ```
   source change-compiler.sh 
   cmake_config x86_64-centos7-gcc8-opt
   cd x86_64-centos7-gcc8-opt
   make -j
   cd ..

   ```

6. If everything compiled successfully, copy the minimal set to a target dir:
   ```
   rm -rf flxlibs-deps
   ./copy-minimal.sh
   ```

7. We have the minimal set of dependencies required by `flxlibs` in the `flxlibs-deps` directory. One would need to produce a UPS or other package from this.