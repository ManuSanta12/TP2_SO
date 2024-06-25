#!/bin/bash

# Set audio device variable
if [ "$(uname)" = "Darwin" ]; then
    audio="coreaudio"
else
    audio="alsa"
fi

# Check for gdb argument
if [ "$1" = "gdb" ]; then
    qemu-system-x86_64 -s -S -hda Image/x64BareBonesImage.qcow2 -m 512 -d int 2>&1 | grep "v="
else
    qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -d int 2>&1 | grep "v="
fi

