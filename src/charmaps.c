
// convert runes from current charmap to unicode
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

	//if ( attribute & ATTR_BLINK ){
	//	return( rune+(0x1D44e - 0x40) );
	//}

	return(rune);
}

// reverse table, to convert and display utf8 in the selected codepage
// works obviously only once, when the runes are created.
// Later, a redraw or repaste is needed.
void create_unicode_table(){
	memset(uni_to_cp,0,UNITABLE);
	for ( int a = 0; a<128; a++ )
		if ( codepage[selected_codepage][a] < UNITABLE )
			uni_to_cp[codepage[selected_codepage][a]]= a+128;
}

char unicode_to_charmap( uint uc ){
	if ( uc < UNITABLE ){
		if ( uni_to_cp[uc] )
			return( uni_to_cp[uc] );
	} else {
		// search our charmap table
		// currently not needed, all unicode points are below UNITABLE (=0x2600)
	}

	return(0);
}

// callback, called by keystroke
void set_charmap(const Arg *a){
	if ( a->i < sizeof(codepage)/sizeof(uint*)){
		selected_codepage = a->i;
		create_unicode_table();
		
		fill_font_asciitable(&dc.font);
		if ( useboldfont )
			fill_font_asciitable(&dc.bfont);
		if ( useitalicfont )
			fill_font_asciitable(&dc.ifont);
		if ( usebolditalicfont )
			fill_font_asciitable(&dc.ibfont);


		redraw();
	}

}



