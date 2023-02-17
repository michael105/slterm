

static inline uint charmap_convert(uint rune,uint attribute){
	// 1252 / dec-mcs / ansi
	//if ( rune > 0x7f && (rune < 0xA0) )
	//	return( cp1252[rune-0x80] );
	if ( rune > 0x7f && (rune < 0x100) )
		return( codepage[selected_codepage][rune-0x80] );
	
	// greek (1st try - need beta code translation table)
	//if ( attribute & ATTR_REVERSE ){
	//	return( rune+(0x390 - 0x40) );
	//}

	if ( attribute & ATTR_BLINK ){
		return( rune+(0x1D44e - 0x40) );
	}



	return(rune);
}

// reverse table, to convert and display utf8 in the selected codepage
// works obviously only once, when the runes are created.
// Later, a redraw or repaste is needed.
void create_reversetable(){
	for ( int a = 0; a<128; a++ )
		rev_cp[a] = codepage[selected_codepage][a];
}


// callback, called by keystroke
void set_charmap(const Arg *a){
	if ( a->i < sizeof(codepage)/sizeof(uint*)){
		selected_codepage = a->i;
		create_reversetable();
		redraw();
	}

}



