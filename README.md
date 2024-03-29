# ramrecovery

Simple demo illustrating remanence of data in RAM. See https://en.wikipedia.org/wiki/Cold_boot_attack and associated papers for much more information!  In one of the original cold boot
attack papers "Lest We Remember: Cold Boot Attacks on Encryption Keys" by Halderman et al. they loaded an image of the Mona Lisa and "cut power for varying lengths of time".

* src - Program to fill memory with the Mona Lisa
* src-experiment - Automated experiment, uses Raspberry Pi OS to inject single image into contiguous RAM, then uses bare metal kernel to dump memory.  Makes use of modified USB hub that is controlled by relays,
to automatically choose which USB disk to boot from. Uses Wifi plug running Tasmota turn on/off Pi.  The experiment script controls the target Pi using SSH.
* src-memcopy - Bare metal kernel to dump memory over Raspberry Pi UART interface.
* src-module - Linux kernel module to inject image(s) into contiguous memory

Also see my little article about this project in https://pagedout.institute/?page=issues.php Issue #3, page 14.

The following image shows the setup for dumping memory from a Raspberry Pi using the aforementioned bare metal kernel.  A USB hub was modified so that active devices could be chosen via relays (should switch to using MOSFETs sometime!), to choose between a USB disk running the bare metal kernel or Raspberry Pi OS.  The memory could then be dumped, after varying delays from the Pi via UART at 1 MBaud to another Pi.

![Setup](images/setup.jpg)

The program in src, loads many images of the Mona Lisa into RAM on the Pi, running Raspberry Pi OS Lite.

```
cd src
make run
```

Then quickly turn off/on the power on the raspberry pi.

Using the memory forensics kernel module LiME, we can see there is still some remanence of the Mona Lisa in memory:

```
git clone https://github.com/504ensicsLabs/LiME.git
cd LiME/src/
make
```

```
sudo insmod ./lime-$(uname -r).ko "path=out.dump format=padded"
```

Extract Mona Lisa images from memory dump:

```
LANG=C grep --text --byte-offset --only-matching --perl-regexp '\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x58\x02\x93\x01\x58\x02\x18\x20' out.dump | LANG=C sed "s/:.*//g" | xargs -I {} dd if=out.dump bs=1 skip={} count=725444 of={}.tga
montage -border 0 -mode concatenate *.tga tiled.jpg
convert -resize "3000>" tiled.jpg tiled_small.jpg
```

## Kernel module for filling contiguous RAM with Mona Lisa

The kernel module obtains contiguous RAM and fills with Mona Lisa.

In /boot/cmdline.txt, added the following to allocate 700MB of contiguous kernel RAM:
```
cma=700M@36M
```

And the following (from https://forums.raspberrypi.com/viewtopic.php?p=2132596#p2132596) to /boot/config.txt (to maximise CMA):

```
[all]
device_tree_address=0x2000000
device_tree_end=0x20FFFFF
```

```
cd src-module
make
```

Use the following to fill contiguous RAM, with Mona.

```
sudo insmod ramrec.ko writetoram=true filename="mona.tga" singleimage=false
```

## Image recovered after 0.75s delay

The following image was extracted after a 0.75s delay, without cooling through the automated experiment (see src-experiment).  I noticed the images decayed very quickly with no cooling, for example they appeared almost completely decayed around 1s.

![Recovered Image](images/image_0.75.png)
