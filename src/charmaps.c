

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



	return(rune);
}


// callback, called by keystroke
void set_charmap(const Arg *a){
	if ( a->i < sizeof(codepage)/sizeof(uint*)){
		selected_codepage = a->i;
		redraw();
	}

}



