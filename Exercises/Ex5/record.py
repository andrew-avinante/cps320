#!/usr/bin/env python

## recordtest.py
##
## This is an example of a simple sound capture script.
##
## The script opens an ALSA pcm device for sound capture, sets
## various attributes of the capture, and reads in a loop,
## writing the data to standard out.
##
## To test it out do the following:
## python recordtest.py out.raw # talk to the microphone
## aplay -r 8000 -f S16_LE -c 1 out.raw

#!/usr/bin/env python

from __future__ import print_function

import sys
import time
import getopt
import alsaaudio

def usage():
    print('usage: record.py [-d <device>] <file>', file=sys.stderr)
    sys.exit(2)

if __name__ == '__main__':

    recorddevice = 'default'

    opts, args = getopt.getopt(sys.argv[1:], 'd:')
    for o, a in opts:
        if o == '-d':
            recorddevice = a

    if not args:
        usage()

    f = open(args[0], 'wb')

    inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, device=recorddevice)

    # Set attributes: Mono, 44100 Hz, 16 bit little endian samples
    inp.setchannels(1)
    inp.setrate(44100)
    inp.setformat(alsaaudio.PCM_FORMAT_S16_LE)
    
    inp.setperiodsize(160)
    
    print('Recording... press Ctrl-C to end.')

    try:
        while True:
            # Read data from device
            numframes, data = inp.read()
        
            if numframes:
                f.write(data)
            else:
                print('Unexpected: Read no audio data from input device.')
    except KeyboardInterrupt:   
        print("\nRecording terminated.")
        f.close()

