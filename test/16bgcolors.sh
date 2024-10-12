#!/bin/sh

echo
echo -e "\033[1;37mBackground colors\e[0m"

echo
#echo -n '     '
for i in `seq 0 7`; do echo -ne "\e[48;5;$i""m    $i   \e[30m"; done
echo -e "\e[0m"

for i in `seq 8 15`; do printf "\e[30;48;5;$i""m   %2d   " $i; done
echo -e "\e[0m"
echo



