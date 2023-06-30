#!/bin/bash




function col() {
		fi=$1; shift
		fa=$[$1*36]; shift
		for fb in $@; do
				printf "\033[0;$fi""m\033[38;5;$[$fa+$fb]""m %03d"  $[$fa+$fb]
		done
}



echo -e "\033[1;30m" Grayscale, \#232-255 "\033[0m"
echo

col 2 0 {232..255}
echo
col 0 0 {232..255}
echo
col 1 0 {232..255}
echo



