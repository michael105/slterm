#!/bin/perl -w



use Data::Dumper::Simple;


my @a;

$col = 0;
$r = 0;

foreach my $b ( 16,52,88,124,160,196 ){
	for ( my $c = $b; $c<$b+6; $c++ ){
		$a[$c-$b+$r][$col] = $c;
	}
	$col ++;
}


$col=0;
$r += 6;
for( my $b = 27; $b<=207;$b+=36 ){
	for ( my $c = $b; $c>=$b-5; $c-- ){
		$a[$b-$c+$r][$col] = $c;
	}
	$col ++;
}

$col=0;
$r += 6;

for( my $b = 28; $b<=208;$b+=36 ){
	for ( my $c = $b; $c<$b+6; $c++ ){
		$a[$c-$b+$r][$col] = $c;
	}
	$col ++;
}

$col=0;
$r += 6;
for( my $b = 39; $b<=219;$b+=36 ){
	for ( my $c = $b; $c>=$b-5; $c-- ){
		$a[$b-$c+$r][$col] = $c;
	}
	$col ++;
}


$col=0;
$r += 6;

for( my $b = 40; $b<=220;$b+=36 ){
	for ( my $c = $b; $c<$b+6; $c++ ){
		$a[$c-$b+$r][$col] = $c;
	}
	$col ++;
}


$col=0;
$r += 6;
for( my $b = 51; $b<=231;$b+=36 ){
	for ( my $c = $b; $c>=$b-5; $c-- ){
		$a[$b-$c+$r][$col] = $c;
	}
	$col ++;
}


if ( 1 ){
foreach my $row(@a){
	foreach my $c( @{$row} ){
		printf("\033[38;5;$c"."m%03d ",$c);
	}
	print "\n";
}

exit;
}


if ( 1 ){
foreach my $row(@a){
	my @ap = [];
	foreach my $c( (@{$row}) ){
		if ( $c >=16 && $c <= 51 ){
			push @ap,$c;
			printf("\033[0;2m\033[38;5;$c"."m%03d ",$c);
			printf("\033[0;0m\033[38;5;$c"."m%03d ",$c);
			printf("\033[0;1m\033[38;5;$c"."m%03d ",$c);

			next;
		}

		printf("\033[0;1m\033[38;5;$c"."m%03d ",$c);
	}

	foreach my $c( reverse(@{$row}) ){
		if ( $c >=16 && $c <= 51 ){
			push @ap,$c;
			next;
		}

		printf("\033[0;0m\033[38;5;$c"."m%03d ",$c);
	}
	foreach my $c( (@{$row}) ){
		if ( $c >=16 && $c <= 51 ){
			push @ap,$c;
			#printf("\033[0;0m\033[38;5;$c"."m%03d ",$c);
		#printf("\033[0;1m\033[38;5;$c"."m%03d ",$c);
		#printf("\033[0;2m\033[38;5;$c"."m%03d ",$c);


			next;
		}
		printf("\033[0;2m\033[38;5;$c"."m%03d ",$c);
	}

	#	foreach my $c( @{$row} ){
	#	printf("\033[0;2m\033[38;5;$c"."m%03d ",$c);
	#}
	print "\n";
}
exit;
}






$ln = 0;
foreach my $row(@a){
	if ( $ln == 0 ){
	foreach my $c( @{$row} ){
		printf("\033[38;5;$c"."m%03d ",$c);
	}
	$ln=1;
} else {
	foreach my $c( reverse(@{$row}) ){
		printf("\033[38;5;$c"."m%03d ",$c);
	}
	$ln=0;
	print "\n";
}


	#print Dumper($row);

}
