#!/bin/bash

LINHA=$(egrep -in "$2" $1 | head -n1 | cut -d: -f1)
CONTEUDO=$(head -n$LINHA $1 | tail -n1)
sed -i "${LINHA}i${CONTEUDO}$3" $1
#sed '/rt/d' teste.txt
sed -i "$(($LINHA+1))d" $1
