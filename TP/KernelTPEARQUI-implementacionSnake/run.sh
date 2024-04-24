# #!/bin/bash
# qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 
#!/bin/bash
# Set audio device variable
if [ "$(uname)" = "Darwin" ]; then
    audio="coreaudio"
else
    audio="alsa"
fi
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 --rtc base=localtime
#-audio driver=$audio,model=ac97,id=audio0 


