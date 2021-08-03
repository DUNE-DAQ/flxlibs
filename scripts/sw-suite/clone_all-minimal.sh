#!/bin/bash

usage()
{
  echo "usage: $(basename $0)  ssh|krb5|https|gitlab"
  exit 1
}

if [ $# -lt 1 ]; then
    usage
fi

if [[ $1 = "krb5" ]]; then
  PREFIX="https://:@gitlab.cern.ch:8443"
elif [[ $1 = "https" ]]; then
  PREFIX="https://gitlab.cern.ch"
elif [[ $1 = "gitlab" ]]; then
  PREFIX="https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.cern.ch"
else
  PREFIX="ssh://git@gitlab.cern.ch:7999"
fi
shift

if [ $# -lt 1 ]; then
    TAG=master
else
    TAG=$1
fi

git --version

listOfExternals="external/catch
external/yaml-cpp
external/json
external/pybind11"


listOfPackages="client-template
cmake_tdaq
drivers_rcc
flxcard
packetformat
regmap
ftools
flxcard_py"

for dir in $listOfExternals
do
  name=${dir//[\/]/-}
  echo "$dir"
  if [ ! -d "${dir}" ]; then
    git clone ${PREFIX}/atlas-tdaq-felix/${name}.git ${dir}
  else
    ( cd ${dir}; git pull )
  fi
  echo
done

for dir in $listOfPackages
do
  name=${dir//[\/]/-}
  echo "$dir"
  if [ ! -d "${dir}" ]; then
    git clone --branch ${TAG} ${PREFIX}/atlas-tdaq-felix/${name}.git ${dir}
  else
    ( cd ${dir}; git checkout ${TAG}; git pull )
  fi
  echo
done
