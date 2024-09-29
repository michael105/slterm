#!/bin/sh


for i in `seq 0 18`
do
	echo -e "\e[0m" Attr: $i "\e[$i"m Some text
done


