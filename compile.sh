source load_modules.sh
./build.py crash-consensus;
crash-consensus/libgen/export.sh gcc-release;
crash-consensus/demo/using_conan_fully/build.sh gcc-release;
#crash-consensus/experiments/build.sh

# fix this
#export LD_LIBRARY_PATH=/path-to /dory/crash-consensus/experiments/exported:$LD_LIBRARY_PATH
