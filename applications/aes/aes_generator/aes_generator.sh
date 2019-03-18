#!/bin/bash

MSG_LENGHT=$1
MAX_SLAVES=$2
NUMBER_OF_SLAVES=$3
DEBUG="debug_0"

echo "Creating a AES application..."
echo "--MSG_LENGHT: ${MSG_LENGHT}"
echo "--MAX_SLAVES: $2"
echo "--NUMBER_OF_SLAVES: $3"
echo "--DEBUG: ${4^^}"

if [ $# -gt 3 ]; then
	DEBUG=$4
fi

rm -rf ../aes_*.c

echo " "
echo "Creating main..."

string="int Slave[MAX_SLAVES] = {"
aux=""
for i in `seq 1 $MAX_SLAVES`
do
	if [ $i -eq $MAX_SLAVES ]; then
		aux="aes_slave${i}"
		string+=${aux}
    else
		aux="aes_slave${i},"
		string+=${aux}
    fi
done
aux="};"
string+=${aux}

cp aes_master.c ../aes_master.c
sed -i -e "s/int Slave\[MAX_SLAVES\] = {};/$string/" -e "s/#define MSG_LENGHT/#define MSG_LENGHT $MSG_LENGHT/" -e "s/#define MAX_SLAVES/#define MAX_SLAVES $MAX_SLAVES/" -e "s/#define NUMBER_OF_SLAVES/#define NUMBER_OF_SLAVES $NUMBER_OF_SLAVES/" ../aes_master.c

echo "aes_master.c created!"
echo " "

echo "Creating slaves..."
for i in `seq 1 $MAX_SLAVES`
do
    cp aes_sl.c "../aes_slave${i}.c"
    echo "aes_slave${i}.c created!"
done

echo "Creating files \".h\"..."
cp *.h ./..

if [ "${4^^}" = "DEBUG_1" ]; then
	sed -i -e "s/\/\/#define debug_comunication_on/#define debug_comunication_on/" ../aes_master.h
	sed -i -e "s/\/\/#define debug_comunication_on/#define debug_comunication_on/" ../aes.h
elif [ "${4^^}" = "DEBUG_2" ]; then
	sed -i -e "s/\/\/#define debug_aes_on/#define debug_aes_on/" ../aes.h
elif [ "${4^^}" = "DEBUG_3" ]; then
	sed -i -e "s/\/\/#define debug_aes_on/#define debug_aes_on/" ../aes.h
	sed -i -e "s/\/\/#define debug_comunication_on/#define debug_comunication_on/" ../aes.h
	sed -i -e "s/\/\/#define debug_comunication_on/#define debug_comunication_on/" ../aes_master.h
fi

cp aes.cfg ./..
