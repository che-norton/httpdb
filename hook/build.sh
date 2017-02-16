#!/bin/sh

arm-brcm-linux-uclibcgnueabi-gcc -fPIC -shared -o hook.so hook.c -ldl
arm-brcm-linux-uclibcgnueabi-gcc nat5.c -o nat5 -ldl
#cp hook.so /mnt/tmp-asus/
