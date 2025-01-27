/* retmarks.
 stored in term->retmarks[N]
the store is used circular, 
term->current_retmark points to the currently unused place,
the last retmark can be accessed with (current_retmark - 1) &( RETMARKCOUNT-1)
The position is given as term->histi (histindex)
*/



void retmark_scrolledup(){ // scan upwards for the next retmark, update 
	for ( int t = term->current_retmark - (term->scrolled_retmark?term->scrolled_retmark:1); 
			t!=term->current_retmark; 
			t-- ){
		t &= ( RETMARKCOUNT-1 ); 
		DBG("mark upw: %d   %d\n",t, term->retmarks[t] );
		if ( (term->retmarks[t]==0) || (term->histi - term->retmarks[t] >= term->scr) ){
			term->scrolled_retmark = (term->current_retmark - t) & ( RETMARKCOUNT-1);
			return;
		}
	}
}



void retmark_scrolleddown(){ // scan downwards for the next retmark, update 
	for ( int t = (term->current_retmark - (term->scrolled_retmark?term->scrolled_retmark:1 )) & (RETMARKCOUNT-1); t!=term->current_retmark; 
			t = (t+1) & ( RETMARKCOUNT-1 ) ){
		if ( (term->histi - term->retmarks[t] < term->scr) ){
			term->scrolled_retmark = ( term->current_retmark - t + 1 ) & ( RETMARKCOUNT-1);
			//DBG("mark: %d   %d\n",t, term->retmarks[t] );
			return;
		}
	}
	// at the bottom
	term->scrolled_retmark = 1;
}


void set_retmark() {
	if (term==p_alt) return;
	// check, if (e.g. vi) we are above the last retmark
	if ( (term->retmarks[ (term->current_retmark - 1) & (RETMARKCOUNT-1) ] < 
				(term->histi + term->cursor.y ) )  || 
		( term->circledhist && (term->retmarks[ (term->current_retmark - 1) & (RETMARKCOUNT-1) ] > 
			 (term->histi + term->cursor.y + HISTSIZE/2 )  ) ) ){ 
		// second case: circled buffer, first case: try to detect screen based programs
		// e.g. vim
		term->retmarks[ term->current_retmark ] = term->histi + term->cursor.y;
		term->current_retmark = (term->current_retmark + 1) & (RETMARKCOUNT-1);
		term->scrolled_retmark = 0;
	}
	//DBG("Setretmark: n:%d histi:%d scr:%d  cursor: %d\n", term->current_retmark, term->histi, term->scr, term->cursor.y );
}

void retmark(const Arg* a){
	if (term==p_alt) return; // not usable in alt screen
									 //DBG("Retmark: n:%d scrm:%d histi:%d scr:%d   scroll_mark %d  current_mark %d\n", term->rows, term->retmarks[0],term->histi, term->scr, term->scroll_retmark, term->current_retmark );

									 // rewrite that. (count curentretmark from 0 to UINT_MAX. Limit bits when
									 // accessing the array. ->scrolled_retmark can be set absolute.

	if ( a->i == -1 ){ // tab right in lessmode -> scrolling down

		// todo: rewrite. also for a circled buffer
		int b = 1;
		// todo: reverse scanning.
		for ( int t = (term->current_retmark +1 ) & (RETMARKCOUNT-1); t!=term->current_retmark; 
				t = (t+1) & ( RETMARKCOUNT-1 ) ){
			//if ( (term->retmarks[t] < term->histi - term->scr) ){
			if ( (term->histi - term->retmarks[t] < term->scr) ){
				term->scr=(term->histi - term->retmarks[t]);
				term->scrolled_retmark = ( term->current_retmark - t ) & ( RETMARKCOUNT-1);
				b = 0;
				//DBG("mark: %d   %d\n",t, term->retmarks[t] );
				break;
			}
		}
		if ( b ){
			term->scrolled_retmark = 0;
			scrolltobottom();
		}


		} else if ( a->i >= 1 ){ // number, scroll to retmark number x
			int t = (term->current_retmark - a->i ) & (RETMARKCOUNT-1); 
			term->scr=(term->histi-term->retmarks[t]);
			term->scrolled_retmark = ( term->current_retmark - t ) & ( RETMARKCOUNT-1);

		} else if ( a->i == -2 ){ // = key '0'
			term->scrolled_retmark = 0;
			scrolltobottom();
		} else { // scroll backward / Up
			if ( term->histi<term->rows){ // at the top
				scrolltotop();
				lessmode_toggle( ARGPi(LESSMODE_ON) );	
				return;
			}

#if 0
			// todo: scroll to next retmark
			int t = term->scrolled_retmark;
			if ( t ){
				//t= (t-1) & (RETMARKCOUNT-1);
				t--;
				term->scrolled_retmark = t;
				term->scr=(term->histi - term->retmarks[(term->current_retmark - t)&(RETMARKCOUNT-1) ]);

			}
#else
			for ( int t = (term->current_retmark -1 ) & (RETMARKCOUNT-1); t!=term->current_retmark; 
					t = (t-1) & ( RETMARKCOUNT-1 ) ){
				DBG("mark: %d   %d\n",t, term->retmarks[t] );
				if ( (term->retmarks[t]==0) || (term->histi - term->retmarks[t] > term->scr) ){
					term->scr=(term->histi - term->retmarks[t]);
					term->scrolled_retmark = (term->current_retmark - t) & ( RETMARKCOUNT-1);
					break;
				}
			} 
#endif

	}
	DBG("scr: %d\n", term->scr );
	if ( term->scr<0 ){
		// TODO: circledhist
		if ( term->circledhist )
			term->scr&=(HISTSIZE-1);
		else
			term->scr=0;

		DBG("scr: %d\n", term->scr );
	};
	//DBG("Retmark OUT: n:%d scrm:%d histi:%d scr:%d   scroll_mark %d  current_mark %d\n", term->rows, term->retmarks[0],term->histi, term->scr, term->scroll_retmark, term->current_retmark );

	selscroll(0, term->scr);
	tfulldirt();
	//updatestatus();
	lessmode_toggle( ARGPi(LESSMODE_ON) );
}


