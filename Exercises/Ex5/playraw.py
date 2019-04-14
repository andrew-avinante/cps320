#!/usr/bin/env python

# Simple test script that plays (some) wav files

import sys
import wave
import getopt
import alsaaudio

def play(device, f):    
    # Set attributes
    device.setchannels(1)
    device.setrate(44100)
    device.setformat(alsaaudio.PCM_FORMAT_S16_LE)

    periodsize = 160

    device.setperiodsize(periodsize)

    bytes_per_frame = 2
    bytes_to_rw = periodsize * bytes_per_frame
    
    data = f.read(bytes_to_rw)
    while data:
        device.write(data)
        data = f.read(bytes_to_rw)


def usage():
    print('usage: playraw.py [-d <device>] <file>', file=sys.stderr)
    sys.exit(2)

if __name__ == '__main__':

    device = 'default'

    opts, args = getopt.getopt(sys.argv[1:], 'd:')
    for o, a in opts:
        if o == '-d':
            device = a

    if not args:
        usage()
        
    f = open(args[0], 'rb')
    device = alsaaudio.PCM(device=device)

    play(device, f)

    f.close()
