import sys
import re
import ctypes
import os

rdblah = ctypes.cdll.LoadLibrary("./readblah.so")
rdblah.read_blah.restype = ctypes.c_char_p
rdblah.open_blah.argtypes = [ctypes.c_char_p]

def search(path : str) -> list:
    connections = []
    for f in os.listdir(path):
        findString = ""
        absltPath = path + '/' + f
        if f.endswith('.txt'):
            findString = open(absltPath, 'r')
        elif f.endswith('.blah'):
            f = rdblah.open_blah(absltPath.encode('utf-8'))
            blahString = rdblah.read_blah(f)
            rdblah.close_blah(f)
            findString = blahString.decode('utf-8')
        if findString != "":
            for i in findString:
                match = re.findall(r"""(
                    mysql:\/\/              # matches 'mysql//'
                    (?:[A-z0-9]+\.?)*       # matches 'server-name'
                    :?                      # matches ':' optional
                    (?:[0-9]{0,10})?        # matches '[port]' optional
                    \/                      # matches '/'
                    [A-z][A-z0-9_]*)        # matches 'dbname'
                    """, i,re.VERBOSE)
                if match:
                    for j in match:
                        connections.append(j)
    connections.sort()
    return connections

for i in search(sys.argv[1]):
    print(i)