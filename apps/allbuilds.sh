#!/bin/bash

# Check out
set -e

# Change this to the right place for your PC
NERVES_SYSTEM_BR=~/nerves/nerves_system_br
if [[ ! -e $NERVES_SYSTEM_BR/create-build.sh ]]; then
    echo "ERROR: Can't find the create-build.sh script."
    echo "       Check the NERVES_SYSTEM_BR setting in this file."
    exit 1
fi

# List all of the systems to build
CONFIGS="rpi"

for CONFIG in $CONFIGS; do
    # Run the create-build.sh script to create/update the build directory
    $NERVES_SYSTEM_BR/create-build.sh $CONFIG/nerves_defconfig o/$CONFIG
done

# Build configurations
if false; then
    for CONFIG in $CONFIGS; do
        make -C o/$CONFIG
    done
fi
