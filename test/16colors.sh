#!/bin/sh

echo -ne "\n\033[1;4;37m";
echo -e 256 Color mode, \#0-15 "\033[0;1m (indexed palette)" "\033[0;30;40m "
echo

for i in 0 8; do
	for a in 2 0 1; do
		printf "\033[37;$a""m $a""m:" 
		for b in 0 1 2 3 4 5 6 7;do 
			printf "\033[38;5;"$(($b+$i))"m %03d" $(( $b+$i ))
		done
		echo
	done
done

echo

