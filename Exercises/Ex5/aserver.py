import sys
import alsaaudio
import wave
import socket
from datetime import datetime

PORT = 2000    # Port to listen on

start_time = datetime.now()

# returns the elapsed milliseconds since the start of the program
def millis():
   dt = datetime.now() - start_time
   ms = (dt.days * 24 * 60 * 60 + dt.seconds) * 1000 + dt.microseconds / 1000.0
   return ms

if len(sys.argv) != 3:
    print('Usage: python3 aserver.py sample_rate period_size')
    sys.exit(1)

sample_rate = int(sys.argv[1])
period_size = int(sys.argv[2])

device = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK, device='default')

device.setchannels(1)
device.setrate(sample_rate)
device.setformat(alsaaudio.PCM_FORMAT_S16_LE) # 16 bit little endian frames
device.setperiodsize(period_size)

size_to_rw = period_size * 2  # 2 bytes per mono sample

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
s.bind(('0.0.0.0', PORT)) 

print('Listening on port', PORT)
print('Rate:', sample_rate, 'Period Size:', period_size)
print('Size to r/w:', size_to_rw)

start = millis()
prev_elapsed_time = start
bytecount = 0
packet_count = 0

try:
    while True:
        data = s.recv(size_to_rw)
        if data:
            device.write(data)
            bytecount += len(data)
            packet_count += 1
        elapsed_time = millis() - start
        if elapsed_time - prev_elapsed_time > 1000:        
            cur_elapsed_time = elapsed_time - prev_elapsed_time
            bps_rate = bytecount / cur_elapsed_time * 1000
            print('{: 4.0f} sec {: 5} bytes, {: 3} packets, rate = {: 6.0f} bytes/s  '.format(
                elapsed_time / 1000, bytecount, packet_count, bps_rate))
            packet_count = 0
            bytecount = 0
            prev_elapsed_time = elapsed_time

except KeyboardInterrupt:
    s.close()
    print("\nExiting code.")
    device.close()
    