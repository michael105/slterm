#!/bin/perl
#

foreach my $l (<>){ 
	@a=split(";",$l); 
	$a[1]=~s/[()]//g; 
	printf "%s\t%-20s\t%-12s\t%s\t%s\n", $a[0],$a[1],$a[2],$a[3]; 
};


