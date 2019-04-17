#import alsaaudio # use sudo apt install python3-pyaudio to get this module
import wave
import socket
import sys
import time
from datetime import datetime
from threading import Thread
from time import sleep
import pyttsx3

class Broadcast(Thread):
    discovered = {}
    deviceToCall = ''
    partyHandle = ''
    curAction = 'await'
    statuses = {'await': '@awaiting', 'call' :'@call', 'accept': '@accept', 'reject' : '@reject', 'endcall': '@endcall', 'incoming' : '@awaiting'}
    incomingRequest = False
    def __init__(self, handle):
        super().__init__()
        self.handle = handle
        
    def run(self):
        while True:
            command = ''
            action = Broadcast.statuses[Broadcast.curAction]
            if action == '@call':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif action == '@reject':
                command = handle + action + ' ' + Broadcast.deviceToCall
                action = 'await'
            elif action == '@accept':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif action == '@endcall':
                command = handle + action + ' ' + Broadcast.deviceToCall
            else:
                command = handle + '@awaiting'
            
            sock.sendto(command.encode('UTF-8'), ('<broadcast>', PORT))
            data, addr = sock.recvfrom(1024)
            senderHandle, senderData = data.decode("UTF-8").split('@')

            recieveAction = ''

            if 'awaiting' not in senderData:
                recieveAction, recieverHandle = senderData.split(' ')
            if recieveAction == 'call' and action != '@call' and recieverHandle == handle:
                Broadcast.incomingRequest = True
                Broadcast.curAction = 'incoming'
                Broadcast.partyHandle = senderHandle
            elif recieveAction == 'reject':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif recieveAction == 'accept':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif recieveAction == 'endcall':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif recieveAction == 'await' and senderHandle == Broadcast.partyHandle:
                Broadcast.curAction = 'await'
                Broadcast.partyHandle = ''
                Broadcast.incomingRequest = False

            if senderHandle != handle:
                Broadcast.discovered[senderHandle] = [datetime.now(), action]
            time.sleep(.1)

class Display(Thread):
    selected = 0
    def __init__(self):
        super().__init__()
            
    def getStatus(self):
        statusText = {'await': 'Awaiting call', 'call' :'Calling ' + Broadcast.deviceToCall, 'accept': 'Call in progress', 'reject' : 'Awaiting call', 'endcall': 'Awaiting call', 'incoming' : 'Incoming call from ' + Broadcast.partyHandle}
        return  statusText[Broadcast.curAction]
        
    def run(self):
        while True:
            remove = []
            print('\fSTATS\n---------------')
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
            time.sleep(.1)

class Input(Thread):
    def __init__(self):
        super().__init__()
        self.engine = pyttsx3.init()
        
    def run(self):
        while True:
            if input() == 'w' and Display.selected + 1 < len(Broadcast.discovered):
                Display.selected += 1
                self.engine.say(list(Broadcast.discovered)[Display.selected])
                self.engine.runAndWait()
            elif input() == 's' and Display.selected - 1 >= 0:
                Display.selected -= 1
                self.engine.say(list(Broadcast.discovered)[Display.selected])
                self.engine.runAndWait()
            elif input() == 'c' and len(Broadcast.discovered) != 0:
                Broadcast.deviceToCall = list(Broadcast.discovered)[Display.selected]
                Broadcast.curAction = 'call'
            elif input() == 'x' and len(Broadcast.discovered) != 0:
                Broadcast.deviceToCall = ''
                Broadcast.curAction = 'await'
            elif input() == 'r' and len(Broadcast.discovered) != 0 and Broadcast.incomingRequest:
                Broadcast.deviceToCall = ''
                Broadcast.curAction = 'reject'
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

broadcast = Broadcast(handle)
display = Display()
inputs = Input()

broadcast.start()
display.start()
inputs.start()