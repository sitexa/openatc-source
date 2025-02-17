#!/bin/bash

pushd build_arm

cmake -DUSE_EXTERNAL_ICONV=1 -DXC_RULES_FILE=rules_arm-linux-gnueabihf_4.7.txt .. && make

popd
