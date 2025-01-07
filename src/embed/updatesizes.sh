for i in *ttf 
do 
	echo $i 
	sed -E "s/("$i"_len = ).*/\1"`stat $i -c %s`";/" embed_font.h
done
