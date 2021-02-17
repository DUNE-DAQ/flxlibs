# The location and version of CMake to use
CMAKE_BASE=
CMAKE_VERSION=${CMAKE_VERSION:=3.17.2}
CMAKE_PATH=/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/cmake/v3_17_2/Linux64bit+3.10-2.17/bin

# Alter compiler to be used
BASE=/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/gcc/v8_2_0/Linux64bit+3.10-2.17

export PATH=$BASE/bin:$PATH
export MANPATH=$BASE/share/man:$MANPATH

if [ -e "${BASE}/lib64" ]; then
    export LD_LIBRARY_PATH="$BASE/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi
if [ -e "${BASE}/lib" ]; then
    # Add lib if exists
    export LD_LIBRARY_PATH="$BASE/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

# Export package specific environmental variables

gcc_home=/cvmfs/sft.cern.ch/lcg/releases/gcc/8.3.0-cebb0/x86_64-centos7 

export FC=`which gfortran`
export CC=`which gcc`
export CXX=`which g++`

export COMPILER_PATH=${gcc_home}


