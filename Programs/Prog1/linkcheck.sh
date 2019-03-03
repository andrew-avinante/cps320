#Andrew Avinante aavin894 Program 1
#This program checks for broken links

#!/bin/bash

echo
echo Welcome to LinkCheck, by Andrew Avinante \(aavin894\)
echo

#Checks if input is supplied
if [ $# -eq 0 ]
then
    echo "Enter the URL to check: "
    read URL
else
    URL=$1
fi

echo Downloading $URL
echo

#Checks if URL is real
if wget -q "$URL" -O output.html
then
    echo Here are the links I found:
    echo

    #Loops over input from this horrific RegEx
    for i in $(grep -oh \<[\ ]*[Aa][\ ]*[-=\'\":/.A-Za-z\ _0-9?%\&\;]*[Hh][Rr][Ee][Ff][\ ]*[=][\ ]*[\"\']*[-:/.A-Za-z\ _0-9?%\&\;]*[\"\']*[\ ]*[-=\'\":/.A-Za-z\ _0-9?%\&\;]*\> ./output.html | #Finds anchor tag
    grep -oh [Hh][Tt][Tt][Pp][Ss]*[-:/.A-Za-z\ _0-9?%\&\;]*)    #Extracts the URL
    do
        if wget -q $i
        then
            echo $i - ok
        else
            echo $i - **BROKEN**
        fi
    done
else
    echo "Unable to check URL because it could not be downloaded."
fi
