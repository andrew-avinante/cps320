#import alsaaudio # use sudo apt install python3-pyaudio to get this module
import wave
import socket
import sys
import time
from datetime import datetime
from threading import Thread
from time import sleep
import pyttsx3
import alsaaudio

start_time = datetime.now()

# returns the elapsed milliseconds since the start of the program
def millis():
   dt = datetime.now() - start_time
   ms = (dt.days * 24 * 60 * 60 + dt.seconds) * 1000 + dt.microseconds / 1000.0
   return ms

class Broadcast(Thread):
    discovered = {}
    deviceToCall = ''
    partyHandle = ''
    curAction = 'await'
    statuses = {'await': '@awaiting', 'call' :'@call', 'accept': '@accept', 'incall': '@incall', 'reject' : '@reject', 'endcall': '@endcall', 'incoming' : '@awaiting'}
    action = statuses[curAction]
    incomingRequest = False
    incall = False
    def __init__(self, handle):
        super().__init__()
        self.handle = handle
        self.engine = pyttsx3.init()  
        self.hostname = socket.gethostname()    
        self.IPAddr = socket.gethostbyname(self.hostname)  
        
    def run(self):
        while True:
            command = ''
            Broadcast.action = Broadcast.statuses[Broadcast.curAction]
            if Broadcast.action == '@call':
                command = handle + Broadcast.action + ' ' + Broadcast.deviceToCall + ' ' + self.IPAddr
            elif Broadcast.action == '@reject':
                command = handle + Broadcast.action + ' ' + Broadcast.deviceToCall
                Broadcast.curAction = 'await'
                Broadcast.partyHandle = ''
                Broadcast.incomingRequest = False
            elif Broadcast.action == '@accept':
                Broadcast.incall = True
                command = handle + Broadcast.action + ' ' + Broadcast.deviceToCall + ' ' + self.IPAddr
            elif Broadcast.action == '@endcall':
                command = handle + Broadcast.action + ' ' + Broadcast.deviceToCall
            elif Broadcast.action == '@incall':
                command = handle + Broadcast.action
            else:
                command = handle + '@awaiting'
            
            sock.sendto(command.encode('UTF-8'), ('<broadcast>', PORT))
            time.sleep(1)

class Recieve(Thread):
    partyIP = ''
    def __init__(self):
        super().__init__()
        self.engine = pyttsx3.init()

    def run(self):
        while True:
            data, addr = sock.recvfrom(1024)
            senderHandle, senderData = data.decode("UTF-8").split('@')

            recieveAction = ''
            
            if 'awaiting' not in senderData and 'incall' not in senderData:
                recieveAction, recieverHandle, ip = senderData.split(' ')
            elif senderHandle == Broadcast.partyHandle and not Broadcast.incall:
                Broadcast.curAction = 'await'
                Broadcast.partyHandle = ''
                Broadcast.incomingRequest = False

            if recieveAction == 'call' and Broadcast.action != '@call' and recieverHandle == handle and not Broadcast.incall:
                Broadcast.incomingRequest = True
                Broadcast.curAction = 'incoming'
                Broadcast.partyHandle = senderHandle
                self.engine.say("Call from " + senderHandle)
                self.engine.runAndWait()
                Recieve.partyIP = ip
            elif recieveAction == 'reject':
                Broadcast.deviceToCall = ''
                Broadcast.curAction = 'await'
                self.engine.say("You just got REJECTED")
                self.engine.runAndWait()
            elif recieveAction == 'accept':
                Broadcast.incomingRequest = False
                Broadcast.incall = True
                Broadcast.curAction = 'incall'
                Broadcast.partyHandle = senderHandle
                Recieve.partyIP = ip
            elif recieveAction == 'endcall':
                command = handle + ' ' + Broadcast.deviceToCall
                Broadcast.incall = False
                Broadcast.curAction = 'await'
            elif recieveAction == 'incall':
                Broadcast.curAction = 'incall'

            if senderHandle != handle:
                Broadcast.discovered[senderHandle] = [datetime.now(), senderData]

class VOIP(Thread):
    def __init__(self):
        super().__init__()
        self.sample_rate = 44100
        self.period_size = 1000
        self.device = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, device='default')
        self.device.setchannels(1)
        self.device.setrate(self.sample_rate)
        self.device.setformat(alsaaudio.PCM_FORMAT_S16_LE) # 16 bit little endian frames
        self.device.setperiodsize(self.period_size)
        self.size_to_rw = self.period_size * 2  # 2 bytes per mono sample
        self.prev_elapsed_time = self.start
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def run(self):
        start = millis()
        prev_elapsed_time = start
        while True:
            if(Broadcast.incall):
                self.sock.bind((Recieve.partyIP, 4098)) 
                numframes, data = self.device.read()
                self.sock.sendto(data, (Recieve.partyIP, 4098))
                #sock.send(data)
                elapsed_time = millis() - start
                if elapsed_time - prev_elapsed_time > 1000:        
                    cur_elapsed_time = elapsed_time - prev_elapsed_time
                data = self.sock.recv(size_to_rw)
                if data:
                    self.device.write(data)
                if elapsed_time - prev_elapsed_time > 1000:        
                    cur_elapsed_time = elapsed_time - prev_elapsed_time
                    self.prev_elapsed_time = elapsed_time

        

class Display(Thread):
    selected = 0
    def __init__(self):
        super().__init__()
            
    def getStatus(self):
        statusText = {'await': 'Awaiting call', 'call' :'Calling ' + Broadcast.deviceToCall, 'accept': 'Call in progress', 'incall': 'Call in progress', 'reject' : 'Awaiting call', 'endcall': 'Awaiting call', 'incoming' : 'Incoming call from ' + Broadcast.partyHandle}
        return  statusText[Broadcast.curAction]
        
    def run(self):
        while True:
            remove = []
            print('\fCONTROLS\n---------------\nw - select up\ns - select down\nc - call selected user\nx - cancel call\nr - reject call\na - accept call\nh - hangup\n')
            print('DEVICES')
            count = 0
            for i in Broadcast.discovered:
                dt = datetime.now() - Broadcast.discovered[i][0]
                if dt.days * 24 * 60 * 60 + dt.seconds * 1000 + dt.microseconds / 1000.0 > 5000:
                    remove.append(i)
                else:
                    bullet = '- ' if Display.selected != count else '* '
                    print(bullet + i)
                count += 1
            for i in remove:
                del Broadcast.discovered[i]
                remove.remove(i)
                if Broadcast.deviceToCall == i:
                    Broadcast.curAction = 'await'
            print(f'STATUS: {self.getStatus()}')
            time.sleep(1)

class Input(Thread):
    def __init__(self):
        super().__init__()
        self.engine = pyttsx3.init()
        
    def run(self):
        while True:
            inputChar = input()
            if inputChar == 'w' and Display.selected + 1 < len(Broadcast.discovered):
                Display.selected += 1
                self.engine.say(list(Broadcast.discovered)[Display.selected])
                self.engine.runAndWait()
            elif inputChar == 's' and Display.selected - 1 >= 0:
                Display.selected -= 1
                self.engine.say(list(Broadcast.discovered)[Display.selected])
                self.engine.runAndWait()
            elif inputChar == 'c' and len(Broadcast.discovered) != 0:
                Broadcast.deviceToCall = list(Broadcast.discovered)[Display.selected]
                Broadcast.curAction = 'call'
            elif inputChar == 'x' and len(Broadcast.discovered) != 0:
                Broadcast.deviceToCall = ''
                Broadcast.curAction = 'await'
            elif inputChar == 'r' and len(Broadcast.discovered) != 0 and Broadcast.incomingRequest:
                Broadcast.deviceToCall = ''
                Broadcast.curAction = 'reject'
            elif inputChar == 'a' and len(Broadcast.discovered) != 0 and Broadcast.incomingRequest:
                Broadcast.curAction = 'accept'
            elif inputChar == 'h' and len(Broadcast.discovered) != 0 and Broadcast.incall:
                Broadcast.curAction = 'endcall'
            else:
                self.engine.say(list(Broadcast.discovered)[Display.selected])
                self.engine.runAndWait()

PORT = 2000    # Port to transmit to

if len(sys.argv) != 2:
    print('Usage: python3 voip.py handle')
    sys.exit(1)

handle = sys.argv[1]
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(("", 2000))

start_time = datetime.now()

broadcast = Broadcast(handle)
display = Display()
inputs = Input()
recieve = Recieve()
voip = VOIP()
print(voip)
broadcast.start()
display.start()
inputs.start()
recieve.start()
voip.start()