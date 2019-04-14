import alsaaudio # use sudo apt install python3-pyaudio to get this module
import wave
import socket
import sys
import time
from datetime import datetime
import time

PORT = 2000    # Port to transmit to

start_time = datetime.now()

# returns the elapsed milliseconds since the start of the program
def millis():
   dt = datetime.now() - start_time
   ms = (dt.days * 24 * 60 * 60 + dt.seconds) * 1000 + dt.microseconds / 1000.0
   return ms

if len(sys.argv) != 5:
    print('Usage: python3 aclient.py sample_rate period_size hostname <file>')
    sys.exit(1)

sample_rate = int(sys.argv[1])
period_size = int(sys.argv[2])
hostname=sys.argv[3]
file_read = sys.argv[4]

size_to_rw = period_size * 2  # 2 bytes per mono sample

print('Starting client...')
print('Rate:', sample_rate, 'Period Size:', period_size)
print('Size to r/w:', size_to_rw)

print("Transmitting mic data to ", hostname)
start = millis()
prev_elapsed_time = start
bytecount = 0
packet_count = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
f = open(file_read, 'rb')
data = "a"
try:
    while data:
        data = f.read(size_to_rw)
        sock.sendto(data, (hostname, PORT))
        time.sleep(1/size_to_rw/4.5)
        #sock.send(data)
        bytecount += len(data)
        packet_count += 1
        elapsed_time = millis() - start
        if elapsed_time - prev_elapsed_time > 1000:        
            cur_elapsed_time = elapsed_time - prev_elapsed_time
            bps_rate = bytecount / cur_elapsed_time * 1000
            print('{: 4.0f} sec {: 5} bytes, {: 3} packets, rate = {: 6.0f} bytes/s'.format(
                elapsed_time / 1000, bytecount, packet_count, bps_rate))
            packet_count = 0
            bytecount = 0
            prev_elapsed_time = elapsed_time

except KeyboardInterrupt:   
    print("\ndevice off.")
