#!/bin/sh

echo -ne "\n\033[1;4;37m";
echo -e 256 Color mode, \#0-7 "\033[0;1m (indexed palette)" "\033[0;30;40m "
echo

for a in 2 '2;1' 0 1; do
	printf '\033[37;0m%3sm: ' $a
	for b in `seq 30 37`;do 
		printf "\033[$a;$b"'m%03d ' $b
	done
	printf "\033[0m\n"
done

echo

