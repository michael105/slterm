perl -e 'for(32..127,160..255){printf " %3d: %s",$_,chr($_); (($_ | 0x7) == $_) and print "\n";}'
