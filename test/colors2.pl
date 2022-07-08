#!/bin/perl -w
#

my %cls;

sub color{
	my $c = shift;
	my $f = shift;
	printf("\033[38;5;$c"."m%03d ",$c);
	#print "\n" if ( (($c+$f) %12)==0 );
	$cls{$c} = 1 if (( ($c+2) %6) == 0);
}



#for ( 16..45 ){
#	color($_,3);
#}

my $r = 0;
foreach my $a (16,22,28,34,40,
	76,46,82,118,154,148,112,106,100,136,172,130,
	166,202,208,214,220,226,190,184,178,172,142,
	94,58,64,70

) {
	if ( $r==0 ){
		for ( my $c = $a; $c<$a+6; $c++ ){
			color($c,3);
		}
		$r = 1;
	} else {
		for ( my $c = $a+5; $c>=$a; $c-- ){
			color($c,3);
		}
		$r=0;
		print "\n";
	}

}

print "===\n";




for (my $a=16; $a<232; $a+=6 ){
	if (  !exists($cls{$a}) ){
	for ( my $c = $a; $c<$a+6; $c++ ){
		color($c,3);
	}
	print "\n";
}
}

exit;

color($_,3) for ( 202 .. 231  );


print "=====\n";



for ( 52..57, 124..233 ){
	color($_,3);
}

print "=====\n";
for ( 16..233 ){
	color($_,3);
}




