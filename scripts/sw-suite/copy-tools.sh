#!/bin/bash

# copy tools...

mkdir -p flxlibs-deps/bin

cp x86_64-centos7-gcc8-opt/flxcard/flx-* flxlibs-deps/bin

cp x86_64-centos7-gcc8-opt/ftools/libFlxTools* flxlibs-deps/lib
cp x86_64-centos7-gcc8-opt/ftools/feconf* flxlibs-deps/bin
cp x86_64-centos7-gcc8-opt/ftools/femu* flxlibs-deps/bin
