#!/usr/bin/python3

import os
import time
import json
import serial
import urllib.request
import paramiko
from paramiko import SSHConfig
from gpiozero import DigitalOutputDevice

"""
sudo apt install python3-paramiko
sudo apt install python3-serial
"""

pi_os_pin = DigitalOutputDevice(4)
baremetal_kernel_pin = DigitalOutputDevice(3)

target_pi = os.environ['TARGETPI']
tasmota_plug = os.environ['TASMOTAPLUG']

def runnew(host, user):
    ret = None
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(hostname=host, username=user)
    stdin, stdout, stderr = ssh.exec_command('cd /home/pi/ramrecovery/src-module; sudo insmod ramrec.ko writetoram=true filename="mona-big.tga" singleimage=true',get_pty=True)
    stdin, stdout, stderr = ssh.exec_command('dmesg  | grep "Writing to address"',get_pty=True)
    for line in stdout.readlines():
        print(line)
        if "Writing to" in line:
            print(line)
            ret = int(line[line.rfind(" "):].strip(), 16)
            ssh.close()
            return ret
    return None

def power_change(on=False):
    on = "On" if on else "Off"
    url = f"{tasmota_plug}cm?cmnd=Power%20{on}"
    with urllib.request.urlopen(url) as response:
        res = json.loads(response.read())['POWER']
        return res == "ON"

def power_status():
    url = tasmota_plug + "cm?cmnd=Status"
    with urllib.request.urlopen(url) as response:
        return json.loads(response.read())['Status']['Power'] == 1

def choose_usb(pios=True):
    if pios:
        pi_os_pin.off()
        baremetal_kernel_pin.on()
    else:
        pi_os_pin.on()
        baremetal_kernel_pin.off()

def read_serial(filename):
    leng = 0
    dat = open(filename, "wb")
    with serial.Serial('/dev/ttyUSB0', int(1e6), timeout=160) as ser:
        while True:
            s = ser.read(1024 * 1024)
            if s:
                dat.write(s)
                leng += len(s)
                dat.flush()
                if leng >= 70 * 1024 * 1024:
                    break

def write_image(serial_filename, image_filename, addr):
    mona = open("../src-module/mona-big.tga", "rb").read()
    mona_header = mona[0:18]
    dump = open(serial_filename, "rb").read()
    tga = open(image_filename, "wb")
    tga.write(mona_header)
    tga.write(dump[addr+18:addr+len(mona)])
    tga.close()

def main():
    power_change(False)
    time.sleep(10)

    # See image after approx, X seconds
    for exp in [0.9]:
        # Choose raspberry pi OS
        choose_usb(True)
        # Turn power on
        power_change(True)
        # Wait for OS to startup
        time.sleep(2 * 60)
        # Fill memory with Mona Lisa
        addr = runnew(target_pi, "pi")
        print(addr)
        # Turn power off
        power_change(False)
        # Wait for particular duration in minutes
        time.sleep(exp)
        # Choose bare metal kernel
        choose_usb(False)
        # Turn power on
        power_change(True)
        # Read serial data from bare metal kernel
        read_serial(f"serial_{exp}.bin")
        # Turn power off
        power_change(False)
        # Write image
        write_image(f"serial_{exp}.bin", f"image_{exp}.tga", addr)
        # Wait 20 mins
        time.sleep(20 * 60)

if __name__ == "__main__":
    main()
