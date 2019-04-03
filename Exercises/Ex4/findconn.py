import sys
import re
import ctypes

rdblah = ctypes.cdll.LoadLibrary("./readblah.so")
rdblah.open_blah.restype = ctypes.c_void_p

def search(path : str) -> list:
    connections = []
    if('.blah' in path):
        f = rdblah.open_blah(str.encode('utf-8'))
        print(f)
        # blahString = rdblah.read_blah(f)
        rdblah.close_blah(f)
        print("HI")
    else:
        f = open(path, "r")
    for i in f:
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