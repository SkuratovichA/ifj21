#/bin/bash

# IFJ21 - Priklady pro studenty pro dokresleni zadani - TODO otestovat!!!

#OMPILER="tl build"  # tl build neni treba (je zahrnut v tl run); zde muze byt vas prekladac IFJ21 (napr. ./ifj21)
COMPILER="../../cmake-build-debug/ifj21"  # tl build neni treba (je zahrnut v tl run); zde muze byt vas prekladac IFJ21 (napr. ./ifj21)

INTERPRETER="tl run"  # zde muze byt interpret IFJcode20 (./ic20int)
HELPER="ifj21.tl"

for i in *.tl; do

	if [[ $i == $HELPER ]]; then
		continue
	fi

	IN=`echo $i | sed 's/\(.*\)\.tl/\1.in/g'`
	OUT=`echo $i | sed 's/\(.*\)\.tl/\1.out/g'`
	PRG=$i

	# $COMPILER $i $HELPER > $PRG   # nekontroluji se chyby prekladu (pro chybove testy nutno rozsirit)
	# RETURNED_COMPILER=$?

	echo -n "DIFF: $INTERPRETER $PRG and $OUT: "
	cat $IN | $INTERPRETER $PRG 2> >(sed $'s,.*,\e[31m&\e[m,'>&2) | diff - $OUT > /dev/null 2> /dev/null
	DIFFOK=$?
	if [ "x$DIFFOK" == "x0" ]; then
		echo "OK"
	else
		echo "DIFFERENCE to $OUT"
		cat $IN | $INTERPRETER $PRG 2> >(sed $'s,.*,\e[31m&\e[m,'>&2) | diff - $OUT 
	fi
        echo
done;

# rm *.tmp

exit 0
