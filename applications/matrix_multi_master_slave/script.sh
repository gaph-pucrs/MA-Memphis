#!/bin/bash

ESCRAVOS=$1
LINHAS=$2
TEMPO=$3
TESTCASE=$4
SCENARIO=$5

for i in `seq 2 $ESCRAVOS`
do
	cp slave1.c slave$i.c
done

for i in `seq $ESCRAVOS`
do
	sed -i 's/Echo(\"Fim.*/Echo(\"Fim da aplicação slave'$i'\")\;/' slave$i.c
	sed -i 's/Echo(\"Inicio.*/Echo(\"Inicio da aplicação slave'$i'\")\;/' slave$i.c
	
done

sed -i 's/^#define numTasks.*/#define numTasks /' multME.h

./script2.sh multME.h "#define numTasks" "$ESCRAVOS"

sed -i 's/^int task.*/int task[numTasks] = {/' multME.h

./script2.sh multME.h "int task" "slave1"
chmod +x script2.sh

for i in `seq 2 $ESCRAVOS`
do
	./script2.sh multME.h "int task" ", slave$i"
done

./script2.sh multME.h "int task" "};"

sed -i 's/^#define N.*/#define N/' multME.h

./script2.sh multME.h "#define N" " $LINHAS"

cd ../..

#source source-here.sh

#cd hemps8.5/testcases

#hemps-run $APLICACAO.yaml $3

#cd ..

cd sandbox-memphis
memphis-gen $TESTCASE
memphis-app $TESTCASE mult_master_slave
memphis-run $TESTCASE $SCENARIO $TEMPO

cd ..
cd applications/mult_master_slave
for i in `seq 2 $ESCRAVOS`
do
	rm -rf slave$i.c
done


