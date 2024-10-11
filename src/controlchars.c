
// handle control chars and sequences, ansi escape sequences.

#include "controlchars.h"

static CSIEscape csiescseq;
static STREscape strescseq;


int handle_controlchars( Rune u, uint decoded_len, char* decoded_char ){

	if (term->esc & ESC_STR) { // within a esc sequence
		if (u == '\a' || u == 030 || u == 032 || u == 033 || ISCONTROLC1(u)) {
			term->esc &= ~(ESC_START | ESC_STR | ESC_DCS);
			if (IS_SET(MODE_SIXEL)) {
				/* TODO: render sixel */
				term->mode &= ~MODE_SIXEL;
				return(1);
			}
			term->esc |= ESC_STR_END;
			goto check_control_code;
		}

		if (IS_SET(MODE_SIXEL)) {
			/* TODO: implement sixel mode */
			return(1);
		}
		if (term->esc & ESC_DCS && strescseq.len == 0 && u == 'q') {
			term->mode |= MODE_SIXEL;
		}

		if (strescseq.len + decoded_len >= strescseq.siz) {
			/*
			 * Here is a bug in terminals. If the user never sends
			 * some code to stop the str or esc command, then st
			 * will stop responding. But this is better than
			 * silently failing with unknown characters. At least
			 * then users will report back.
			 *
			 * In the case users ever get fixed, here is the code:
			 */
			/*
			 * term->esc = 0;
			 * strhandle();
			 */
			//if (strescseq.siz > (SIZE_MAX - UTF_SIZ) / 2) 
			if (strescseq.siz > 8192 ) {
				// misc2024 How could a user fix that, when not being told? Eh??
				// albite nowadays SIZE_MAX is defined here with 0xfffffff*4,
				// why. anyways. The comment above might be about 30 years old.
				// I added the error msg. And changed the maximum size to an
				// IMHO more reasonable value.
				// Users might like to be told about problems 
				fprintf(stderr,"ESC Sequence too long, exceeding 8kB\n"); 
				return(1);
			}
			strescseq.siz *= 2;
			strescseq.buf = xrealloc(strescseq.buf, strescseq.siz);
		}

		memmove(&strescseq.buf[strescseq.len], decoded_char, decoded_len);
		strescseq.len += decoded_len;
		return(1);
	}

check_control_code:
	/*
	 * Actions of control codes must be performed as soon they arrive
	 * because they can be embedded inside a control sequence, and
	 * they must not cause conflicts with sequences.
	 */
	if (ISCONTROL(u)) {
		//	if ( u>=0x80 )
		//		printf("cont r: %x\n",u);

		tcontrolcode(u);
		/*
		 * control codes are not shown ever
		 */
		return(1);
	} else if (term->esc & ESC_START) {
		if (term->esc & ESC_CSI) {
			csiescseq.buf[csiescseq.len++] = u;
			if (BETWEEN(u, 0x40, 0x7E) ||
					csiescseq.len >= sizeof(csiescseq.buf) - 1) {
				term->esc = 0;
				csiparse();
				csihandle();
			}
			return(1);
		} else if (term->esc & ESC_UTF8) {
			tdefutf8(u);
		} else if (term->esc & ESC_ALTCHARSET) {
			tdeftran(u);
		} else if (term->esc & ESC_TEST) {
			tdectest(u);
		} else {
			if (!eschandle(u)) {
				return(1);
			}
			/* sequence already finished */
		}
		term->esc = 0;
		/*
		 * All characters which form part of a sequence are not
		 * printed
		 */
		return(1);
	}


	return(0);
}


void strparse(void) {
	int c;
	char *p = strescseq.buf;

	strescseq.narg = 0;
	strescseq.buf[strescseq.len] = '\0';

	if (*p == '\0') {
		return;
	}

	while (strescseq.narg < STR_ARG_SIZ) {
		strescseq.args[strescseq.narg++] = p;
		while ((c = *p) != ';' && c != '\0') {
			++p;
		}
		if (c == '\0') {
			return;
		}
		*p++ = '\0';
	}
}

void strdump(void) {
	size_t i;
	uint c;

	fprintf(stderr, "ESC%c", strescseq.type);
	for (i = 0; i < strescseq.len; i++) {
		c = strescseq.buf[i] & 0xff;
		if (c == '\0') {
			putc('\n', stderr);
			return;
		} else if (isprint(c)) {
			putc(c, stderr);
		} else if (c == '\n') {
			fprintf(stderr, "(\\n)");
		} else if (c == '\r') {
			fprintf(stderr, "(\\r)");
		} else if (c == 0x1b) {
			fprintf(stderr, "(\\e)");
		} else {
			fprintf(stderr, "(%02x)", c);
		}
	}
	fprintf(stderr, "ESC\\\n");
}

void strreset(void) {
	strescseq = (STREscape){
		.buf = xrealloc(strescseq.buf, STR_BUF_SIZ),
			.siz = STR_BUF_SIZ,
	};
}



void tstrsequence(uchar c) {
	strreset();

	switch (c) {
		case 0x90: /* DCS -- Device Control String */
			c = 'P';
			term->esc |= ESC_DCS;
			break;
		case 0x9f: /* APC -- Application Program Command */
			c = '_';
			break;
		case 0x9e: /* PM -- Privacy Message */
			c = '^';
			break;
		case 0x9d: /* OSC -- Operating System Command */
			c = ']';
			break;
	}
	strescseq.type = c;
	term->esc |= ESC_STR;
}

void csiparse(void) {
	char *p = csiescseq.buf, *np;
	long int v;

	csiescseq.narg = 0;
	if (*p == '?') {
		csiescseq.priv = 1;
		p++;
	}

	csiescseq.buf[csiescseq.len] = '\0';
	while (p < csiescseq.buf + csiescseq.len) {
		np = NULL;
		v = strtol(p, &np, 10);
		if (np == p) {
			v = 0;
		}
		if (v == LONG_MAX || v == LONG_MIN) {
			v = -1;
		}
		csiescseq.arg[csiescseq.narg++] = v;
		p = np;
		if (*p != ';' || csiescseq.narg == ESC_ARG_SIZ) {
			break;
		}
		p++;
	}
	csiescseq.mode[0] = *p++;
	csiescseq.mode[1] = (p < csiescseq.buf + csiescseq.len) ? *p : '\0';
}


void csidump(void) {
	size_t i;
	uint c;

	fprintf(stderr, "ESC[");
	for (i = 0; i < csiescseq.len; i++) {
		c = csiescseq.buf[i] & 0xff;
		if (isprint(c)) {
			putc(c, stderr);
		} else if (c == '\n') {
			fprintf(stderr, "(\\n)");
		} else if (c == '\r') {
			fprintf(stderr, "(\\r)");
		} else if (c == 0x1b) {
			fprintf(stderr, "(\\e)");
		} else {
			fprintf(stderr, "(%02x)", c);
		}
	}
	putc('\n', stderr);
}

void csireset(void) { memset(&csiescseq, 0, sizeof(csiescseq)); }


void csihandle(void) {
	char buf[40];
	int len;

	//printf("scseq.mode: %c\n", csiescseq.mode[0]);//DBG
	switch (csiescseq.mode[0]) {
		default:
unknown:
			fprintf(stderr, "erresc: unknown csi ");
			csidump();
			/* die(""); */
			break;
		case '@': /* ICH -- Insert <n> blank char */
			DEFAULT(csiescseq.arg[0], 1);
			tinsertblank(csiescseq.arg[0]);
			break;
		case 'A': /* CUU -- Cursor <n> Up */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(term->cursor.x, term->cursor.y - csiescseq.arg[0]);
			break;
		case 'B': /* CUD -- Cursor <n> Down */
		case 'e': /* VPR --Cursor <n> Down */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(term->cursor.x, term->cursor.y + csiescseq.arg[0]);
			break;
		case 'i': /* MC -- Media Copy */
			switch (csiescseq.arg[0]) {
				case 0:
					tdump();
					break;
				case 1:
					tdumpline(term->cursor.y);
					break;
				case 2:
					tdumpsel();
					break;
				case 4:
					term->mode &= ~MODE_PRINT;
					break;
				case 5:
					term->mode |= MODE_PRINT;
					break;
			}
			break;
		case 'c': /* DA -- Device Attributes */
			if (csiescseq.arg[0] == 0) {
				ttywrite(vtiden, strlen(vtiden), 0);
			}
			break;
		case 'C': /* CUF -- Cursor <n> Forward */
		case 'a': /* HPR -- Cursor <n> Forward */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(term->cursor.x + csiescseq.arg[0], term->cursor.y);
			break;
		case 'D': /* CUB -- Cursor <n> Backward */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(term->cursor.x - csiescseq.arg[0], term->cursor.y);
			break;
		case 'E': /* CNL -- Cursor <n> Down and first col */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(0, term->cursor.y + csiescseq.arg[0]);
			break;
		case 'F': /* CPL -- Cursor <n> Up and first col */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(0, term->cursor.y - csiescseq.arg[0]);
			break;
		case 'g': /* TBC -- Tabulation clear */
			switch (csiescseq.arg[0]) {
				case 0: /* clear current tab stop */
					term->tabs[term->cursor.x] = 0;
					break;
				case 3: /* clear all the tabs */
					memset(term->tabs, 0, term->colalloc * sizeof(*term->tabs));
					break;
				default:
					goto unknown;
			}
			break;
		case 'G': /* CHA -- Move to <col> */
		case '`': /* HPA */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveto(csiescseq.arg[0] - 1, term->cursor.y);
			break;
		case 'H': /* CUP -- Move to <row> <col> */
		case 'f': /* HVP */
			DEFAULT(csiescseq.arg[0], 1);
			DEFAULT(csiescseq.arg[1], 1);
			tmoveato(csiescseq.arg[1] - 1, csiescseq.arg[0] - 1);
			break;
		case 'I': /* CHT -- Cursor Forward Tabulation <n> tab stops */
			DEFAULT(csiescseq.arg[0], 1);
			tputtab(csiescseq.arg[0]);
			break;
		case 'J': /* ED -- Clear screen */
			switch (csiescseq.arg[0]) {
				case 0: /* below */
					tclearregion(term->cursor.x, term->cursor.y, term->colalloc - 1, term->cursor.y);
					if (term->cursor.y < term->row - 1) {
						tclearregion(0, term->cursor.y + 1, term->colalloc - 1, term->row - 1);
					}
					break;
				case 1: /* above */
					if (term->cursor.y > 1) {
						tclearregion(0, 0, term->col - 1, term->cursor.y - 1);
					}
					tclearregion(0, term->cursor.y, term->cursor.x, term->cursor.y);
					break;
				case 2: /* all */
					tclearregion(0, 0, term->col - 1, term->row - 1);
					break;
				default:
					goto unknown;
			}
			break;
		case 'K': /* EL -- Clear line */
			switch (csiescseq.arg[0]) {
				case 0: /* right */
					tclearregion(term->cursor.x, term->cursor.y, term->colalloc - 1, term->cursor.y);
					break;
				case 1: /* left */
					tclearregion(0, term->cursor.y, term->cursor.x, term->cursor.y);
					break;
				case 2: /* all */
					tclearregion(0, term->cursor.y, term->colalloc - 1, term->cursor.y);
					break;
			}
			break;
		case 'S': /* SU -- Scroll <n> line up */
			DEFAULT(csiescseq.arg[0], 1);
			tscrollup(term->top, csiescseq.arg[0], 0);
			break;
		case 'T': /* SD -- Scroll <n> line down */
			DEFAULT(csiescseq.arg[0], 1);
			tscrolldown(term->top, csiescseq.arg[0], 0);
			break;
		case 'L': /* IL -- Insert <n> blank lines */
			DEFAULT(csiescseq.arg[0], 1);
			tinsertblankline(csiescseq.arg[0]);
			break;
		case 'l': /* RM -- Reset Mode */
			tsetmode(csiescseq.priv, 0, csiescseq.arg, csiescseq.narg);
			break;
		case 'M': /* DL -- Delete <n> lines */
			DEFAULT(csiescseq.arg[0], 1);
			tdeleteline(csiescseq.arg[0]);
			break;
		case 'X': /* ECH -- Erase <n> char */
			DEFAULT(csiescseq.arg[0], 1);
			tclearregion(term->cursor.x, term->cursor.y, term->cursor.x + csiescseq.arg[0] - 1, term->cursor.y);
			break;
		case 'P': /* DCH -- Delete <n> char */
			DEFAULT(csiescseq.arg[0], 1);
			//printf("Delc\n");
			tdeletechar(csiescseq.arg[0]);
			break;
		case 'Z': /* CBT -- Cursor Backward Tabulation <n> tab stops */
			DEFAULT(csiescseq.arg[0], 1);
			tputtab(-csiescseq.arg[0]);
			break;
		case 'd': /* VPA -- Move to <row> */
			DEFAULT(csiescseq.arg[0], 1);
			tmoveato(term->cursor.x, csiescseq.arg[0] - 1);
			break;
		case 'h': /* SM -- Set terminal mode */
			tsetmode(csiescseq.priv, 1, csiescseq.arg, csiescseq.narg);
			break;
		case 'm': /* SGR -- Terminal attribute (color) */
			tsetattr(csiescseq.arg, csiescseq.narg);
			break;
		case 'n': /* DSR – Device Status Report (cursor position) */
			if (csiescseq.arg[0] == 6) {
				len =
					snprintf(buf, sizeof(buf), "\033[%i;%iR", term->cursor.y + 1, term->cursor.x + 1);
				ttywrite(buf, len, 0);
			}
			break;
		case 'r': /* DECSTBM -- Set Scrolling Region */
			if (csiescseq.priv) {
				goto unknown;
			} else {
				DEFAULT(csiescseq.arg[0], 1);
				DEFAULT(csiescseq.arg[1], term->row);
				tsetscroll(csiescseq.arg[0] - 1, csiescseq.arg[1] - 1);
				tmoveato(0, 0);
			}
			break;
		case 's': /* DECSC -- Save cursor position (ANSI.SYS) */
			tcursor(CURSOR_SAVE);
			break;
		case 'u': /* DECRC -- Restore cursor position (ANSI.SYS) */
			tcursor(CURSOR_LOAD);
			break;
		case ' ':
			switch (csiescseq.mode[1]) {
				case 'q': /* DECSCUSR -- Set Cursor Style */
					if (xsetcursor(csiescseq.arg[0])) {
						goto unknown;
					}
					break;
				default:
					goto unknown;
			}
			break;
	}
}


void strhandle(void) {
	char *p = NULL, *dec;
	int j, narg, par;

	term->esc &= ~(ESC_STR_END | ESC_STR);
	strparse();
	par = (narg = strescseq.narg) ? atoi(strescseq.args[0]) : 0;

	switch (strescseq.type) {
		case ']': /* OSC -- Operating System Command */
			switch (par) {
				case 0:
				case 1:
				case 2:
					if (narg > 1) {
						xsettitle(strescseq.args[1]);
					}
					return;
				case 52:
					if (narg > 2) {
						dec = base64dec(strescseq.args[2]);
						if (dec) {
							xsetsel(dec);
							xclipcopy();
						} else {
							fprintf(stderr, "erresc: invalid base64\n");
						}
					}
					return;
				case 4: /* color set */
					if (narg < 3) {
						break;
					}
					p = strescseq.args[2];
					/* FALLTHROUGH */
				case 104: /* color reset, here p = NULL */
					j = (narg > 1) ? atoi(strescseq.args[1]) : -1;
					if (xsetcolorname(j, p)) {
						if (par == 104 && narg <= 1) {
							return; /* color reset without parameter */
						}
						fprintf(stderr, "erresc: invalid color j=%d, p=%s\n", j,
								p ? p : "(null)");
					} else {
						/*
						 * TODO if defaultbg color is changed, borders
						 * are dirty
						 */
						redraw();
					}
					return;
			}
			break;
		case 'k': /* old title set compatibility */
			xsettitle(strescseq.args[0]);
			return;
		case 'P': /* DCS -- Device Control String */
			term->mode |= ESC_DCS;
		case '_': /* APC -- Application Program Command */
		case '^': /* PM -- Privacy Message */
			return;
	}

	fprintf(stderr, "erresc: unknown str ");
	strdump();
}


// misc: handle control chars
// bin mode should go here.
void tcontrolcode(uchar ascii) {
	//printf("tcontrolcode: %c\n", ascii); //DBG
	switch (ascii) {
		case '\t': /* HT */
			tputtab(1);
			return;
		case '\b': /* BS */
			tmoveto(term->cursor.x - 1, term->cursor.y);
			return;
		case '\r': /* CR */
			tmoveto(0, term->cursor.y);
			return;
		case '\f': /* LF */
		case '\v': /* VT */
		case '\n': /* LF */
			/* go to first col if the mode is set */
			tnewline(IS_SET(MODE_CRLF));
			return;
		case '\a': /* BEL */
			if (term->esc & ESC_STR_END) {
				/* backwards compatibility to xterm */
				strhandle();
			} else {
				xbell();
			}
			break;
		case '\033': /* ESC */
			csireset();
			term->esc &= ~(ESC_CSI | ESC_ALTCHARSET | ESC_TEST);
			term->esc |= ESC_START;
			return;
		case '\016': /* SO (LS1 -- Locking shift 1) */
		case '\017': /* SI (LS0 -- Locking shift 0) */
			term->charset = 1 - (ascii - '\016'); // todo: handle charset
			return;
		case '\032': /* SUB */
			tsetchar('?', &term->cursor.attr, term->cursor.x, term->cursor.y);
		case '\030': /* CAN */
			csireset();
			break;
		case '\005': /* ENQ (IGNORED) */
		case '\000': /* NUL (IGNORED) */
		case '\021': /* XON (IGNORED) */
		case '\023': /* XOFF (IGNORED) */
		case 0177:   /* DEL (IGNORED) */
			return;
		case 0x80: /* TODO: PAD */
		case 0x81: /* TODO: HOP */
		case 0x82: /* TODO: BPH */
		case 0x83: /* TODO: NBH */
		case 0x84: /* TODO: IND */
			break;
		case 0x85:     /* NEL -- Next line */
			tnewline(1); /* always go to first col */
			break;
		case 0x86: /* TODO: SSA */
		case 0x87: /* TODO: ESA */
			break;
		case 0x88: /* HTS -- Horizontal tab stop */
			term->tabs[term->cursor.x] = 1;
			break;
		case 0x89: /* TODO: HTJ */
		case 0x8a: /* TODO: VTS */
		case 0x8b: /* TODO: PLD */
		case 0x8c: /* TODO: PLU */
		case 0x8d: /* TODO: RI */
		case 0x8e: /* TODO: SS2 */
		case 0x8f: /* TODO: SS3 */
		case 0x91: /* TODO: PU1 */
		case 0x92: /* TODO: PU2 */
		case 0x93: /* TODO: STS */
		case 0x94: /* TODO: CCH */
		case 0x95: /* TODO: MW */
		case 0x96: /* TODO: SPA */
		case 0x97: /* TODO: EPA */
		case 0x98: /* TODO: SOS */
		case 0x99: /* TODO: SGCI */
			break;
		case 0x9a: /* DECID -- Identify Terminal */
			ttywrite(vtiden, strlen(vtiden), 0);
			break;
		case 0x9b: /* TODO: CSI */
		case 0x9c: /* TODO: ST */
			break;
		case 0x90: /* DCS -- Device Control String */
		case 0x9d: /* OSC -- Operating System Command */
		case 0x9e: /* PM -- Privacy Message */
		case 0x9f: /* APC -- Application Program Command */
			tstrsequence(ascii);
			return;
	}
	/* only CAN, SUB, \a and C1 chars interrupt a sequence */
	term->esc &= ~(ESC_STR_END | ESC_STR);
}

/*
 * returns 1 when the sequence is finished and it hasn't to read
 * more characters for this sequence, otherwise 0
 */
int eschandle(uchar ascii) {
	//printf("eschandle: %c\n", ascii); //DBG
	switch (ascii) {
		case '[':
			term->esc |= ESC_CSI;
			return 0;
		case '#':
			term->esc |= ESC_TEST;
			return 0;
		case '%':
			term->esc |= ESC_UTF8;
			return 0;
		case 'P': /* DCS -- Device Control String */
		case '_': /* APC -- Application Program Command */
		case '^': /* PM -- Privacy Message */
		case ']': /* OSC -- Operating System Command */
		case 'k': /* old title set compatibility */
			tstrsequence(ascii);
			return 0;
		case 'n': /* LS2 -- Locking shift 2 */
		case 'o': /* LS3 -- Locking shift 3 */
			term->charset = 2 + (ascii - 'n');
			break;
		case '(': /* GZD4 -- set primary charset G0 */
		case ')': /* G1D4 -- set secondary charset G1 */
		case '*': /* G2D4 -- set tertiary charset G2 */
		case '+': /* G3D4 -- set quaternary charset G3 */
			term->icharset = ascii - '(';
			term->esc |= ESC_ALTCHARSET;
			return 0;
		case 'D': /* IND -- Linefeed */
			if (term->cursor.y == term->bot) {
				tscrollup(term->top, 1, 1);
			} else {
				tmoveto(term->cursor.x, term->cursor.y + 1);
			}
			break;
		case 'E':      /* NEL -- Next line */
			tnewline(1); /* always go to first col */
			break;
		case 'H': /* HTS -- Horizontal tab stop */
			term->tabs[term->cursor.x] = 1;
			break;
		case 'M': /* RI -- Reverse index */
			if (term->cursor.y == term->top) {
				tscrolldown(term->top, 1, 1);
			} else {
				tmoveto(term->cursor.x, term->cursor.y - 1);
			}
			break;
		case 'Z': /* DECID -- Identify Terminal */
			ttywrite(vtiden, strlen(vtiden), 0);
			break;
		case 'c': /* RIS -- Reset to initial state */
			treset();
			resettitle();
			xloadcolors();
			break;
		case '=': /* DECPAM -- Application keypad */
			xsetmode(1, MODE_APPKEYPAD);
			break;
		case '>': /* DECPNM -- Normal keypad */
			xsetmode(0, MODE_APPKEYPAD);
			break;
		case '7': /* DECSC -- Save Cursor */
			tcursor(CURSOR_SAVE);
			break;
		case '8': /* DECRC -- Restore Cursor */
			tcursor(CURSOR_LOAD);
			break;
		case '\\': /* ST -- String Terminator */
			if (term->esc & ESC_STR_END) {
				strhandle();
			}
			break;
		default:
			fprintf(stderr, "erresc: unknown sequence ESC 0x%02X '%c'\n", (uchar)ascii,
					isprint(ascii) ? ascii : '.');
			break;
	}
	return 1;
}


