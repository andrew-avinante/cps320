import alsaaudio # use sudo apt install python3-pyaudio to get this module
import wave
import socket
import sys
import time
from datetime import datetime
from threading import Thread
from time import sleep

class Broadcast(Thread):
    discovered = {}
    def __init__(self, handle):
        super().__init__()
        self.handle = handle
        
    def run(self):
        while True:
            sock.sendto(handle.encode('UTF-8'), ('<broadcast>', PORT))
            data, addr = sock.recvfrom(1024)
            data = data.decode("UTF-8")
            if data != handle:
                discovered[data] = datetime.now()
            time.sleep(1)

class Display(Thread):
    def __init__(self):
        super().__init__()
        
    def run(self):
        while True:
            for i in Broadcast.discovered:
                if datetime.now() - Broadcast.discovered[i] > 10000:
                    del Broadcast.discovered[i]
            print(Broadcast.discovered)
            time.sleep(10)
        

PORT = 2000    # Port to transmit to

if len(sys.argv) != 2:
    print('Usage: python3 voip.py handle')
    sys.exit(1)

handle = sys.argv[1] + '@awaiting'
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(("", 2000))

broadcast = Broadcast(handle)
display = Display()
broadcast.start()
display.start()