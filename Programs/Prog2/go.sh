#!/bin/bash

# Change this if your webserver has a different name/path
WEBSERVER="./webserver"

# Shouldn't have to change this
DOC_ROOT="./wwwroot"

# Or this
WARPROXY="./warproxy.py"

# Do not change this
TIMEOUT="10"

CMD="$WARPROXY -t $TIMEOUT -o csunix.bju.edu -n 10.10.26.49 -p 8080 -- $WEBSERVER -r $DOC_ROOT"
echo "Running: $CMD"
read -e -N1 -iY -p "Is this OK? [Y/n]" OK
if [ "$OK" != "Y" ]; then
    echo "OK, bailing out..."
    exit 1
fi
$CMD
