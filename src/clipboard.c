// clipboard handling
//

#include "clipboard.h"

XSelection xsel;

// utf8 conversion (again, simplified)
// converts from the current charmap to utf8
uint to_utf8( char* obuf, const char* ibuf ){
	char *ob = obuf;
	while ( *ibuf ){
		//printf("c: %d\n",(uint) (*ibuf));
		if ( *ibuf > 0 ){
			*obuf = *ibuf;
			obuf++;
		} else {
			uint uc = charmap_convert( (uchar)*ibuf, 0 );
			//printf("%d\n",uc);
			int p = 2;

			//if ( !valid_uc_utf8(uc) )
			//	goto ERR_UTF8;
			// invalid values (rfc3629
			// spare that. We do only have our chartable.

			char initb=(char)0xc0;
			if ( uc >= 65536 ){
				obuf[3] = (uc & 0x3f) | 0x80;
				initb >>= (char)1;
				uc >>= 6;
				p++;
			}
			if ( uc >= 2048 ){
				obuf[2] = (uc & 0x3f) | 0x80;
				initb >>= (char)1;
				uc >>= 6;
				p++;
			}
			obuf[1] = (uc & 0x3f) | 0x80;
			obuf[0] = (uc>>6) | initb;
			obuf += p;
		}
		ibuf++;
	}
	*obuf = 0;
	return( obuf-ob );
}

// converts from utf8 to the current charmap, unsupported
// chars are left as utf8
uint from_utf8( char* obuf, const char* buf, uint len ){
	char *ob = obuf;
	int a = 0;
	while ( a<len ){
		int tmp = a;
		// convert utf8
		if ( (a+1<len) && ( (buf[a+1] & 0xc0) == 0x80 ) ){ 
			uint uc = ( (buf[a] & 0x1f) << 6 ) | (buf[a+1] & 0x3f);
			if ( (buf[a] & 0xe0) == 0xc0 ){ // initial Byte 2Byte utf8
				a++;
			} else if ( (a+2<len) && ( (buf[a+2] & 0xc0) == 0x80 ) ){ 
				uc = ( uc << 6 ) | (buf[a+2] & 0x3f);
				if ( (buf[a] & 0xf0) == 0xe0 ){ // initial Byte 3Byte utf8
					a+=2;
				} else if ( (a+3<len) && ( (buf[a+3] & 0xc0) == 0x80 ) ){ 
					if ( (buf[a] & 0xf8) == 0xf0 ){ // 4byte
						uc = ((uc<<6) & 0x1FFFFF ) | ( buf[a+3] & 0x3f );
						a+=3;
					}
				}
			} 
			// need to check, keep utf8, if char is not present
			*obuf = unicode_to_charmap( uc );
			if ( *obuf == 0 ){
				while( tmp <= a ){
					*obuf++ = buf[tmp];
					tmp++;
				}
			} else {
				obuf++;
			}
		} else {
			*obuf++ = buf[a];
		}
		a++;
	}
	*obuf = 0;

	return(obuf - ob);
}

void setsel(char *str, Time t) {
	if (!str)
		return;

	free(xsel.primary);
	xsel.primary = str;

	XSetSelectionOwner(xwin.dpy, XA_PRIMARY, xwin.win, t);
	if (XGetSelectionOwner(xwin.dpy, XA_PRIMARY) != xwin.win)
		selclear();

	clipcopy(NULL);
}


void xsetsel(char *str) { setsel(str, CurrentTime); }


void clipcopy(const Arg *dummy) {
	Atom clipboard;

	free(xsel.clipboard);
	xsel.clipboard = NULL;

	if (xsel.primary != NULL) {
		//printf( "clipcopy: %s\n",xsel.primary);
		//convert to utf8
		xsel.clipboard = xstrdup(xsel.primary);
	// convert to utf8 here? ( case of charmaps changes.. )
		//xsel.clipboard = xmalloc( strlen( xsel.primary) * 4 + 4 );
		//to_utf8( xsel.clipboard, xsel.primary );

		clipboard = XInternAtom(xwin.dpy, "CLIPBOARD", 0);
		XSetSelectionOwner(xwin.dpy, clipboard, xwin.win, CurrentTime);
	}
}

void clippaste(const Arg *dummy) {
	Atom clipboard;

	clipboard = XInternAtom(xwin.dpy, "CLIPBOARD", 0);
	XConvertSelection(xwin.dpy, clipboard, xsel.xtarget, clipboard, xwin.win,
			CurrentTime);
}

void selpaste(const Arg *dummy) {
	XConvertSelection(xwin.dpy, XA_PRIMARY, xsel.xtarget, XA_PRIMARY, xwin.win,
			CurrentTime);
}


void xclipcopy(void) { clipcopy(NULL); }

void selclear_(XEvent *e) { selclear(); }

void selrequest(XEvent *e) {
	XSelectionRequestEvent *xsre;
	XSelectionEvent xev;
	Atom xa_targets, string, clipboard;
	char *seltext;

	xsre = (XSelectionRequestEvent *)e;
	xev.type = SelectionNotify;
	xev.requestor = xsre->requestor;
	xev.selection = xsre->selection;
	xev.target = xsre->target;
	xev.time = xsre->time;
	if (xsre->property == None)
		xsre->property = xsre->target;

	/* reject */
	xev.property = None;

	xa_targets = XInternAtom(xwin.dpy, "TARGETS", 0);
	if (xsre->target == xa_targets) {
		/* respond with the supported type */
		string = xsel.xtarget;
		XChangeProperty(xsre->display, xsre->requestor, xsre->property, XA_ATOM, 32,
				PropModeReplace, (uchar *)&string, 1);
		xev.property = xsre->property;
	} else if (xsre->target == xsel.xtarget || xsre->target == XA_STRING) {
		/*
		 * xith XA_STRING non ascii characters may be incorrect in the
		 * requestor. It is not our problem, use utf8.
		 */
		clipboard = XInternAtom(xwin.dpy, "CLIPBOARD", 0);
		if (xsre->selection == XA_PRIMARY) {
			seltext = xsel.primary;
		} else if (xsre->selection == clipboard) {
			seltext = xsel.clipboard;
		} else {
			fprintf(stderr, "Unhandled clipboard selection 0x%lx\n", xsre->selection);
			return;
		}
		if (seltext != NULL) {

#ifdef UTF8_CLIPBOARD
			int len = strlen(seltext);
			{
				char buf[len*4+4];
				int blen = to_utf8( buf, seltext );
			//	printf("seltext: %s\n",seltext);
			//	printf("seltext utf8: %s\n",buf);
			XChangeProperty(xsre->display, xsre->requestor, xsre->property,
					xsre->target, 8, PropModeReplace, 
					 (uchar*)buf, blen );
					//(uchar *)seltext, strlen(seltext));
			xev.property = xsre->property;
			}
#else
			XChangeProperty(xsre->display, xsre->requestor, xsre->property,
					xsre->target, 8, PropModeReplace, 
					(uchar *)seltext, strlen(seltext));
			xev.property = xsre->property;
#endif

		}
	}

	/* all done, send a notification to the listener */
	if (!XSendEvent(xsre->display, xsre->requestor, 1, 0, (XEvent *)&xev))
		fprintf(stderr, "Error sending SelectionNotify event\n");
}




void selnotify(XEvent *e) {
	ulong nitems, ofs, rem;
	int format;
	uchar *data, *last, *repl;
	Atom type, incratom, property = None;

	incratom = XInternAtom(xwin.dpy, "INCR", 0);

	ofs = 0;
	if (e->type == SelectionNotify)
		property = e->xselection.property;
	else if (e->type == PropertyNotify)
		property = e->xproperty.atom;

	if (property == None)
		return;

	do {
		if (XGetWindowProperty(xwin.dpy, xwin.win, property, ofs, BUFSIZ / 4, False,
					AnyPropertyType, &type, &format, &nitems, &rem,
					&data)) {
			fprintf(stderr, "Clipboard allocation failed\n");
			return;
		}

		if (e->type == PropertyNotify && nitems == 0 && rem == 0) {
			/*
			 * If there is some PropertyNotify with no data, then
			 * this is the signal of the selection owner that all
			 * data has been transferred. We won't need to receive
			 * PropertyNotify events anymore.
			 */
			MODBIT(xwin.attrs.event_mask, 0, PropertyChangeMask);
			XChangeWindowAttributes(xwin.dpy, xwin.win, CWEventMask, &xwin.attrs);
		}

		if (type == incratom) {
			/*
			 * Activate the PropertyNotify events so we receive
			 * when the selection owner does send us the next
			 * chunk of data.
			 */
			MODBIT(xwin.attrs.event_mask, 1, PropertyChangeMask);
			XChangeWindowAttributes(xwin.dpy, xwin.win, CWEventMask, &xwin.attrs);

			/*
			 * Deleting the property is the transfer start signal.
			 */
			XDeleteProperty(xwin.dpy, xwin.win, (int)property);
			continue;
		}

		/*
		 * As seen in getsel:
		 * Line endings are inconsistent in the terminal and GUI world
		 * copy and pasting. When receiving some selection data,
		 * replace all '\n' with '\r'.
		 * FIXME: Fix the computer world.
		 */
		repl = data;
		last = data + nitems * format / 8;
		while ((repl = memchr(repl, '\n', last - repl))) {
			*repl++ = '\r';
		}

		if (IS_SET(MODE_BRCKTPASTE) && ofs == 0)
			ttywrite("\033[200~", 6, 0);

#ifdef UTF8_CLIPBOARD
		//printf("Paste: %s  \nsize: %d\n",data,last-data);
		//convert from utf8 / different charmap here
			{
				char buf[last-data];
				int len = from_utf8( buf, data, last-data );
				ttywrite((char *)buf, len, 1);
			}
#else
		ttywrite((char *)data, nitems * format / 8, 1);
#endif
		if (IS_SET(MODE_BRCKTPASTE) && rem == 0)
			ttywrite("\033[201~", 6, 0);
		XFree(data);
		/* number of 32-bit chunks returned */
		ofs += nitems * format / 32;
	} while (rem > 0);

	/*
	 * Deleting the property again tells the selection owner to send the
	 * next data chunk in the property.
	 */
	XDeleteProperty(xwin.dpy, xwin.win, (int)property);
}


void printsel(const Arg *arg) { tdumpsel(); }

void tdumpsel(void) {
	char *ptr;

	if ((ptr = getsel())) {
		tprinter(ptr, strlen(ptr));
		free(ptr);
	}
}


