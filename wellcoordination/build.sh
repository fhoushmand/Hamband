#!/bin/bash

#set -e

rm -rf build
mkdir build
pushd build

conan install .. --build missing 
#--profile /rhome/fhous001/farzin/FastChain/dory/conan/profiles/gcc-debug.profile
conan build ..
