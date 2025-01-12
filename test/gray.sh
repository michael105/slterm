#!/bin/bash




function col() {
		fi=$1; shift
		fa=$[$1*36]; shift
		for fb in $@; do
				printf "\033[0;$fi""m\033[38;5;$[$fa+$fb]""m %03d"  $[$fa+$fb]
		done
}

function gray(){
echo -ne "\033[1;30m$*\033[0m"
}

gray "Grayscale, #232-255 "
echo
echo

gray "1:   "; col 1 0 {255..232}; echo
gray "0:   "; col 0 0 {255..232}; echo
gray "1;2: "; col "1;2" 0 {255..232}; echo
gray "2:   "; col 2 0 {255..232}; echo

echo



