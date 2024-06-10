#!/bin/bash

# Set audio device variable
if [ "$(uname)" = "Darwin" ]; then
    audio="coreaudio"
else
    audio="alsa"
fi

# Check for gdb argument
if [ "$1" = "gdb" ]; then
    qemu-system-x86_64 -s -S -hda Image/x64BareBonesImage.qcow2 -m 512
else
    qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512
fi

# Uncomment the following line to enable audio
# qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audio driver=$audio,model=ac97,id=audio0
