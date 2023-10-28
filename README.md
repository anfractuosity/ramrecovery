# ramrecovery

The program in src, loads 5100 images of the Mona Lisa into RAM on the Pi 4GB, running Raspberry Pi OS Lite.

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
grep --byte-offset --only-matching -aPo '\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x58\x02' out.dump | sed "s/:.*//g" | xargs -I {} dd if=out.dump bs=1 skip={} count=725444 of={}.tga
montage -border 0 -mode concatenate *.tga tiled.jpg
convert -resize "1000>" tiled.jpg tiled_small.jpg
```
