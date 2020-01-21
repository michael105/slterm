#!/bin/zsh

echo TERM: $TERM COLORTERM: $COLORTERM

for a in {30..49}; 
do (for b in {0..8}; 
	do echo -n "\033[0;$b;03;$a""m" " $b;$a "
		done ;) 
	echo "\033[0;37m";  
done


echo "\033[1;4m";
echo Attributes
echo -n "\033[0m";

for a in {0..15};do printf "\033[0;$a""m\033[38;5;4m %03d" $a; done
echo
for a in {16..31};do printf "\033[0;$a""m\033[38;5;4m %03d" $a; done
echo



echo "\033[1;4;37m";
echo 256 Color mode, \#0-15 "\033[0;1m (indexed palette)"
echo -n "\033[0m";

for i in 0 8; do
		for a in 2 0 1; do
				printf "\033[37;$a""m $a""m:" 
				for b in {0..7};do 
						printf "\033[38;5;$[$b+$i]""m %03d" $[$b+$i]
				done
				echo
		done
done

echo "\033[1;4m";
echo Inverted
echo -n "\033[0m";



for i in 0 8; do
		for a in 2 0 1; do
				printf "\033[37;$a""m $a""m:" 
				for b in {0..7};do 
						printf "\033[38;5;$[$b+$i]m"  
						# first color; then ivert
						printf "\033[7""m %03d" $[$b+$i]
				done
				echo
		done
done


function col() {
		fi=$1; shift
		fa=$[$1*36]; shift
		for fb in $@; do
				printf "\033[0;$fi""m\033[38;5;$[$fa+$fb]""m %03d"  $[$fa+$fb]
		done
}


function colbg() {
		fi=$1; shift
		fa=$[$1*36]; shift
		for fb in $@; do
				printf "\033[0;$fi""m\033[48;5;$[$fa+$fb]""m %03d"  $[$fa+$fb]
		done
}






echo "\033[0;1;37;4m";
echo Colors \#16-231
echo -n "\033[0m";




for a in {1..15}; do 
		(for b in {0..15}; do 
				(( color = a * 16 + b )) 
			#	printf "\033[0;2m\033[38;5;$color""m %03d"  $color
				printf "\033[0m\033[38;5;$color""m %03d"  $color
			#	printf "\033[0;1m\033[38;5;$color""m %03d"  $color
		done
		) 
		echo "\033[0;30m"  
done



echo "\033[1;4;37m";
echo 256 Grayscale, \#232-255 "\033[0;1m"
echo -n "\033[0m";

col 2 0 {232..255}
echo
col 0 0 {232..255}
echo
col 1 0 {232..255}
echo




echo "\033[0;1;37;4m";
echo Colors \#16-231 - gradient
echo -n "\033[0m";

#for i in 2 0 1; do

		i=2
for a in {0..5}; do 
		col $i $a {16..21}
		col $i $a {27..22}
		col $i $a {28..33}
		col $i $a {39..34}
		col $i $a {46..51}
		col $i $a {45..41}
		echo
done


i=0;
for a in {5..0}; do 
		col $i $a {16..21}
		col $i $a {27..22}
		col $i $a {28..33}
		col $i $a {39..34}
		col $i $a {46..51}
		col $i $a {45..41}
		echo
done

i=1

for a in {0..5}; do 
		col $i $a {16..21}
		col $i $a {27..22}
		col $i $a {28..33}
		col $i $a {39..34}
		col $i $a {46..51}
		col $i $a {45..41}
		echo
done

echo


#col 0 0 {16..21}
#col 0 1 {16..21}





echo "\033[0;1;37;4m";
echo Colors \#16-231 - gradient background
echo -n "\033[0m";

#for i in 2 0 1; do

		i=2
for a in {0..5}; do 
		colbg $i $a {16..21}
		colbg $i $a {27..22}
		colbg $i $a {28..33}
		colbg $i $a {39..34}
		colbg $i $a {46..51}
		colbg $i $a {45..41}
		echo
done


i=0;
for a in {5..0}; do 
		colbg $i $a {16..21}
		colbg $i $a {27..22}
		colbg $i $a {28..33}
		colbg $i $a {39..34}
		colbg $i $a {46..51}
		colbg $i $a {45..41}
		echo
done

i=1

for a in {0..5}; do 
		colbg $i $a {16..21}
		colbg $i $a {27..22}
		colbg $i $a {28..33}
		colbg $i $a {39..34}
		colbg $i $a {46..51}
		colbg $i $a {45..41}
		echo
done

echo



exit

echo

for a in {0..16} #
do 
		(for b in {0..15}
		do 
				(( color = a * 16 + b )) 
				printf "\033[48;5;$color""m %03d"  $color
		done
		) 
		echo "\033[0;30m"  
done






