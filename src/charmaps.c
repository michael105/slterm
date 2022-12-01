

static inline uint convert(uint rune){
	// 1252 / dec-mcs / ansi
	//if ( rune > 0x7f && (rune < 0xA0) )
	//	return( cp1252[rune-0x80] );
	if ( rune > 0x7f && (rune < 0x100) )
		return( codepage[selected_codepage][rune-0x80] );

	return(rune);
}


void set_charmap(const Arg *a){
	if ( a->i < sizeof(codepage)/sizeof(uint*)){
		selected_codepage = a->i;
		redraw();
	}

}



