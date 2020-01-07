perl -e 'for(32..127,160..255){printf " %s",chr($_); (($_ | 0xf)
== $_) and print "\n";}'
