#!/bin/sh
qemu -s -fda bin/grubbootimage.img -hda fat:bin/ -boot a -m 32
