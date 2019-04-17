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
    curAction = 'await'
    statuses = {'await': '@awaiting', 'call' :'@call', 'accept': '@accept', 'reject' : '@reject', 'endcall': '@endcall'}
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
            elif action == '@accept':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif action == '@endcall':
                command = handle + action + ' ' + Broadcast.deviceToCall
            else:
                command = handle + '@awaiting'
            
            sock.sendto(command.encode('UTF-8'), ('<broadcast>', PORT))
            data, addr = sock.recvfrom(1024)
            senderHandle, senderData = data.decode("UTF-8").split('@')

            recieveAction, recieverHandle = recieveAction.split(' ')

            if recieveAction == 'call' and action != '@call' and recieverHandle == handle:
                Display.status = 'Incoming call from ' + data
            elif recieveAction == 'reject':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif recieveAction == 'accept':
                command = handle + action + ' ' + Broadcast.deviceToCall
            elif recieveAction == 'endcall':
                command = handle + action + ' ' + Broadcast.deviceToCall
            else:
                recieveAction = handle + '@awaiting'
            if data != handle:
                Broadcast.discovered[data] = [datetime.now(), action]
            time.sleep(1)

class Display(Thread):
    status = 'Awaiting call'
    selected = 0
    def __init__(self):
        super().__init__()
        
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
            print(f'STATUS: {Display.status}')
            time.sleep(1)

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
                Display.status = 'Calling ' + list(Broadcast.discovered)[Display.selected]
                Broadcast.deviceToCall = list(Broadcast.discovered)[Display.selected]
                Broadcast.curAction = 'call'
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