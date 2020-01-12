/* See LICENSE for license details. */
#include <X11/X.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "st.h"
#include "win.h"
#include "selection.h"
#include "debug.h"


#if defined(__linux)
#include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <libutil.h>
#endif

/* macros */
#define ISCONTROLC0(c) (BETWEEN(c, 0, 0x1f) || (c) == '\177')
#define ISCONTROLC1(c) (BETWEEN(c, 0x80, 0x9f))
#define ISCONTROL(c) ((c <= 0x1f) || BETWEEN(c, 0x7f, 0x9f))
//#define ISCONTROL(c)		(ISCONTROLC0(c) || ISCONTROLC1(c)) // \177
//equals 0xf7 misc
//#ifdef UTF8
//#define ISDELIM(u)    (utf8strchr(worddelimiters, u) != NULL)
//#else
//#endif


#define SWAPp(a,b) {a = (void*)((POINTER)a ^ (POINTER)b);\
		b = (void*)((POINTER)a ^ (POINTER)b);\
		a = (void*)((POINTER)a ^ (POINTER)b);}
#define SWAPint(a,b) {a^=b;b^=a;a^=b;}

static void execsh(char *, char **);
static void stty(char **);
static void sigchld(int);
static void ttywriteraw(const utfchar *, size_t);

static void csidump(void);
static void csihandle(void);
static void csiparse(void);
static void csireset(void);
static int eschandle(uchar);
static void strdump(void);
static void strhandle(void);
static void strparse(void);
static void strreset(void);

static void tprinter(char *, size_t);
static void tdumpsel(void);
static void tdumpline(int);
static void tdump(void);
static void tclearregion(int, int, int, int);
static void tcursor(int);
static void tdeletechar(int);
static void tdeleteline(int);
static void tinsertblank(int);
static void tinsertblankline(int);
static void tmoveto(int, int);
static void tmoveato(int, int);
static void tnewline(int);
static void tputtab(int);
static void tputc(Rune);
static void treset(void);
static void tscrollup(int, int, int);
static void tscrolldown(int, int, int);
static void tsetattr(int *, int);
static void tsetchar(Rune, Glyph *, int, int);
static void tsetscroll(int, int);
static void tswapscreen(void);
static void tsetmode(int, int, int *, int);
static int twrite(const utfchar *, int, int);
static void tfulldirt(void);
static void tcontrolcode(uchar);
static void tdectest(utfchar);
static void tdefutf8(utfchar);
static int32_t tdefcolor(int *, int *, int);
static void tdeftran(utfchar);
static void tstrsequence(uchar);


static char *base64dec(const char *);
static char base64dec_getc(const char **);

static ssize_t xwrite(int, const char *, size_t);

/* Globals */
Term term; // misc make local?
static CSIEscape csiescseq;
static STREscape strescseq;
static int iofd = 1; // this might bloat
static int cmdfd;
static pid_t pid;
int borderpx;

ssize_t xwrite(int fd, const char *s, size_t len) {
		size_t aux = len;
		ssize_t r;

		while (len > 0) {
				r = write(fd, s, len);
				if (r < 0) {
						return r;
				}
				len -= r;
				s += r;
		}

		return aux;
}

void *xmalloc(size_t len) {
		void *p;

		if (!(p = malloc(len))) {
				die("malloc: %s\n", strerror(errno));
		}

		return p;
}

void *xrealloc(void *p, size_t len) {
		if ((p = realloc(p, len)) == NULL) {
				die("realloc: %s\n", strerror(errno));
		}

		return p;
}

char *xstrdup(char *s) {
		if ((s = strdup(s)) == NULL) {
				die("strdup: %s\n", strerror(errno));
		}

		return s;
}

static const char base64_digits[] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
		61, 0,  0,  0,  -1, 0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,
		0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0};

char base64dec_getc(const char **src) {
		while (**src && !isprint(**src)) {
				(*src)++;
		}
		return **src ? *((*src)++) : '='; /* emulate padding if string ends */
}

char *base64dec(const char *src) {
		size_t in_len = strlen(src);
		char *result, *dst;

		if (in_len % 4) {
				in_len += 4 - (in_len % 4);
		}
		result = dst = xmalloc(in_len / 4 * 3 + 1);
		while (*src) {
				int a = base64_digits[(unsigned char)base64dec_getc(&src)];
				int b = base64_digits[(unsigned char)base64dec_getc(&src)];
				int c = base64_digits[(unsigned char)base64dec_getc(&src)];
				int d = base64_digits[(unsigned char)base64dec_getc(&src)];

				/* invalid input. 'a' can be -1, e.g. if src is "\n" (c-str) */
				if (a == -1 || b == -1) {
						break;
				}

				*dst++ = (a << 2) | ((b & 0x30) >> 4);
				if (c == -1) {
						break;
				}
				*dst++ = ((b & 0x0f) << 4) | ((c & 0x3c) >> 2);
				if (d == -1) {
						break;
				}
				*dst++ = ((c & 0x03) << 6) | d;
		}
		*dst = '\0';
		return result;
}



int tlinelen(int y) {
		int i = term.col;

		if (TLINE(y)[i - 1].mode & ATTR_WRAP) {
				return i;
		}

		while (i > 0 && TLINE(y)[i - 1].u == ' ') {
				--i;
		}

		return i;
}
void die(const char *errstr, ...) {
		va_list ap;

		va_start(ap, errstr);
		vfprintf(stderr, errstr, ap);
		va_end(ap);
		exit(1);
}

void execsh(char *cmd, char **args) {
		char *sh, *prog;
		const struct passwd *pw;

		errno = 0;
		if ((pw = getpwuid(getuid())) == NULL) {
				if (errno) {
						die("getpwuid: %s\n", strerror(errno));
				} else {
						die("who are you?\n");
				}
		}

		if ((sh = getenv("SHELL")) == NULL) {
				sh = (pw->pw_shell[0]) ? pw->pw_shell : cmd;
		}

		if (args) {
				prog = args[0];
		} else if (utmp) {
				prog = utmp;
		} else {
				prog = sh;
		}
		DEFAULT(args, ((char *[]){prog, NULL}));

		unsetenv("COLUMNS");
		unsetenv("LINES");
		unsetenv("TERMCAP");
		setenv("LOGNAME", pw->pw_name, 1);
		setenv("USER", pw->pw_name, 1);
		setenv("SHELL", sh, 1);
		setenv("HOME", pw->pw_dir, 1);
		setenv("TERM", termname, 1);

		signal(SIGCHLD, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGALRM, SIG_DFL);

		execvp(prog, args);
		_exit(1);
}

void sigchld(int a) {
		int stat;
		pid_t p;

		if ((p = waitpid(pid, &stat, WNOHANG)) < 0) {
				die("waiting for pid %hd failed: %s\n", pid, strerror(errno));
		}

		if (pid != p) {
				return;
		}

		if (WIFEXITED(stat) && WEXITSTATUS(stat)) {
				die("child exited with status %d\n", WEXITSTATUS(stat));
		} else if (WIFSIGNALED(stat)) {
				die("child terminated due to signal %d\n", WTERMSIG(stat));
		}
		exit(0);
}

void stty(char **args) {
		char cmd[_POSIX_ARG_MAX], **p, *q, *s;
		size_t n, siz;

		if ((n = strlen(stty_args)) > sizeof(cmd) - 1) {
				die("incorrect stty parameters\n");
		}
		memcpy(cmd, stty_args, n);
		q = cmd + n;
		siz = sizeof(cmd) - n;
		for (p = args; p && (s = *p); ++p) {
				if ((n = strlen(s)) > siz - 1) {
						die("stty parameter length too long\n");
				}
				*q++ = ' ';
				memcpy(q, s, n);
				q += n;
				siz -= n + 1;
		}
		*q = '\0';
		if (system(cmd) != 0) {
				perror("Couldn't call stty");
		}
}

int ttynew(char *line, char *cmd, char *out, char **args) {
		int m, s;

		if (out) {
				term.mode |= MODE_PRINT;
				iofd = (!strcmp(out, "-")) ? 1 : open(out, O_WRONLY | O_CREAT, 0666);
				if (iofd < 0) {
						fprintf(stderr, "Error opening %s:%s\n", out, strerror(errno));
				}
		}

		if (line) {
				if ((cmdfd = open(line, O_RDWR)) < 0) {
						die("open line '%s' failed: %s\n", line, strerror(errno));
				}
				dup2(cmdfd, 0);
				stty(args);
				return cmdfd;
		}

		/* seems to work fine on linux, openbsd and freebsd */
		if (openpty(&m, &s, NULL, NULL, NULL) < 0) {
				die("openpty failed: %s\n", strerror(errno));
		}

		switch (pid = fork()) {
				case -1:
						die("fork failed: %s\n", strerror(errno));
						break;
				case 0:
						close(iofd);
						setsid(); /* create a new process group */
						dup2(s, 0);
						dup2(s, 1);
						dup2(s, 2);
						if (ioctl(s, TIOCSCTTY, NULL) < 0) {
								die("ioctl TIOCSCTTY failed: %s\n", strerror(errno));
						}
						close(s);
						close(m);
#ifdef __OpenBSD__
						if (pledge("stdio getpw proc exec", NULL) == -1) {
								die("pledge\n");
						}
#endif
						execsh(cmd, args);
						break;
				default:
#ifdef __OpenBSD__
						if (pledge("stdio rpath tty proc", NULL) == -1) {
								die("pledge\n");
						}
#endif
						close(s);
						cmdfd = m;
						signal(SIGCHLD, sigchld);
						break;
		}
		return cmdfd;
}

size_t ttyread(void) {
#ifdef UTF8
		static char buf[BUFSIZ];
#else
		static unsigned char buf[BUFSIZ];
#endif
		static int buflen = 0;
		int written;
		int ret;

		/* append read bytes to unprocessed bytes */
		if ((ret = read(cmdfd, buf + buflen, LEN(buf) - buflen)) < 0) {
				die("couldn't read from shell: %s\n", strerror(errno));
		}
		buflen += ret;

		dbg("ttyread, ret: %d, buflen: %d, buf[0] %c\n", ret, buflen, buf[0]);

		written = twrite(buf, buflen, 0);
		buflen -= written;
		/* keep any uncomplete utf8 char for the next call */
		if (buflen > 0) {
				memmove(buf, buf + written, buflen);
		}

		return ret;
}

void ttywrite(const utfchar *s, size_t n, int may_echo) {
		const utfchar *next;
		Arg arg = (Arg){.i = term.scr};

		kscrolldown(&arg);

		dbg("ttywrite %d %x %c\n", n, s[0], s[0]);
		if (may_echo && IS_SET(MODE_ECHO)) {
				dbg("t1\n");
				twrite(s, n, 1);
		}

		if (!IS_SET(MODE_CRLF)) {
				dbg("t2\n");
				ttywriteraw(s, n);
				return;
		}

		dbg("ttywrite2 %d %x %c\n", n, s[0], s[0]);
		/* This is similar to how the kernel handles ONLCR for ttys */
		while (n > 0) {
				dbg("ttywrite3 %d %x %c\n", n, s[0], s[0]);
				if (*s == '\r') {
						next = s + 1;
						ttywriteraw((uchar*)"\r\n", 2);
				} else {
						next = memchr(s, '\r', n);
						DEFAULT(next, s + n);
						ttywriteraw(s, next - s);
				}
				n -= next - s;
				s = next;
		}
}

void ttywriteraw(const utfchar *s, size_t n) {
		fd_set wfd, rfd;
		ssize_t r;
		size_t lim = 256;

		dbg("ttywriteraw n: %d s[0]: %c\n", n, s[0]);

		/*
		 * Remember that we are using a pty, which might be a modem line.
		 * Writing too much will clog the line. That's why we are doing this
		 * dance.
		 * FIXME: Migrate the world to Plan 9.
		 */
		while (n > 0) {
				FD_ZERO(&wfd);
				FD_ZERO(&rfd);
				FD_SET(cmdfd, &wfd);
				FD_SET(cmdfd, &rfd);

				dbg("ttywriteraw while n: %d s[0]: %c\n", n, s[0]);
				/* Check if we can write. */
				if (pselect(cmdfd + 1, &rfd, &wfd, NULL, NULL, NULL) < 0) {
						if (errno == EINTR) {
								continue;
						}
						die("select failed: %s\n", strerror(errno));
				}
				if (FD_ISSET(cmdfd, &wfd)) {
						/*
						 * Only write the bytes written by ttywrite() or the
						 * default of 256. This seems to be a reasonable value
						 * for a serial line. Bigger values might clog the I/O.
						 */
						if ((r = write(cmdfd, s, (n < lim) ? n : lim)) < 0) {
								goto write_error;
						}
						dbg("ttywriteraw  n: %d s[0]: %c lim: %d  r: %d\n", n, s[0], lim, r);
						if (r < n) {
								/*
								 * We weren't able to write out everything.
								 * This means the buffer is getting full
								 * again. Empty it.
								 */
								if (n < lim) {
										lim = ttyread();
								}
								n -= r;
								s += r;
						} else {
								/* All bytes have been written. */
								break;
						}
				}
				if (FD_ISSET(cmdfd, &rfd)) {
						lim = ttyread();
				}
		}
		return;

write_error:
		die("write error on tty: %s\n", strerror(errno));
}

void ttyresize(int tw, int th) {
		struct winsize w;

		w.ws_row = term.row;
		w.ws_col = term.col;
		w.ws_xpixel = tw;
		w.ws_ypixel = th;
		if (ioctl(cmdfd, TIOCSWINSZ, &w) < 0) {
				fprintf(stderr, "Couldn't set window size: %s\n", strerror(errno));
		}
}

void ttyhangup() {
		/* Send SIGHUP to shell */
		kill(pid, SIGHUP);
}

int tattrset(int attr) {
		int i, j;

		for (i = 0; i < term.row - 1; i++) {
				for (j = 0; j < term.col - 1; j++) {
						if (term.line[i][j].mode & attr) {
								return 1;
						}
				}
		}

		return 0;
}

void tsetdirt(int top, int bot) {
		int i;

		LIMIT(top, 0, term.row - 1);
		LIMIT(bot, 0, term.row - 1);

		for (i = top; i <= bot; i++) {
				term.dirty[i] = 1;
		}
}

void tsetdirtattr(int attr) {
		int i, j;

		for (i = 0; i < term.row - 1; i++) {
				for (j = 0; j < term.col - 1; j++) {
						if (term.line[i][j].mode & attr) {
								tsetdirt(i, i);
								break;
						}
				}
		}
}

void tfulldirt(void) { tsetdirt(0, term.row - 1); }

void tcursor(int mode) {
		static TCursor c[2];
		int alt = IS_SET(MODE_ALTSCREEN);

		if (mode == CURSOR_SAVE) {
				c[alt] = term.c;
		} else if (mode == CURSOR_LOAD) {
				term.c = c[alt];
				tmoveto(c[alt].x, c[alt].y);
		}
}

void treset(void) {
		uint i;

		term.c = (TCursor){{.mode = ATTR_NULL, .fg = defaultfg, .bg = defaultbg,.u=' '},
				.x = 0,
				.y = 0,
				.state = CURSOR_DEFAULT};

		memset(term.tabs, 0, term.col * sizeof(*term.tabs));
		for (i = tabspaces; i < term.col; i += tabspaces) {
				term.tabs[i] = 1;
		}
		term.top = 0;
		term.bot = term.row - 1;
#ifdef UTF8
		term.mode = MODE_WRAP | MODE_UTF8;
#else
		term.mode = MODE_WRAP; //|MODE_UTF8;
#endif
		memset(term.trantbl, CS_USA, sizeof(term.trantbl));
		term.charset = 0;

		for (i = 0; i < 2; i++) {
				tmoveto(0, 0);
				tcursor(CURSOR_SAVE);
				tclearregion(0, 0, term.col - 1, term.row - 1);
				tswapscreen();
		}
}

void tnew(int col, int row) {
		dbg2("tnew *******************************************************\n");
		term = (Term){.c = {.attr = {.fg = defaultfg, .bg = defaultbg}}};
		term.hist[0][0] = xmalloc( col * sizeof(Glyph));
		term.hist[1][0] = 0; //xmalloc( col * sizeof(Glyph));

		term.guard=0xf0f0f0f0;
		tresize(col, row);
		treset();
}

int tisaltscr(void) { return IS_SET(MODE_ALTSCREEN); }

void tswapscreen(void) {
		SWAPp( term.line, term.alt );
		term.mode ^= MODE_ALTSCREEN;
		tfulldirt();
}

void kscrolldown(const Arg *a) {
		int n = a->i;

		dbg2("kscrolldown, n: %d, guard: %x\n",n, term.guard);
		if (n < 0) {
				n = term.row + n;
		}

		if (n > term.scr) {
				n = term.scr;
		}
		dbg2("kscrolldown2, n: %d\n",n);

		if (term.scr > 0) {
				term.scr -= n;
				selscroll(0, -n);
				tfulldirt();
		}
}

void kscrollup(const Arg *a) {
		int n = a->i;

		dbg2("kscrollup, n: %d, term.histi: %d, term.row: %d \n",n, term.histi, term.row);
		if (n < 0) {
				n = term.row + n;
		}
		dbg2("kscrollup2, n: %d\n",n);

		if ( term.scr <= HISTSIZE-n ) {
				term.scr += n;

				if ( (term.circledhist==0) && (term.scr>term.histi ) )
						term.scr=term.histi;

				selscroll(0, n);
				tfulldirt();
		}
}

void tscrolldown(int orig, int n, int copyhist) {
		int i;

		dbg2("===== tscrolldown, orig, n, copyhist: %d %d %d\n",orig,n, copyhist);
		LIMIT(n, 0, term.bot - orig + 1);

		if (copyhist) {
				term.histi = ((term.histi - 1 ) ^ HISTSIZE) & (HISTSIZE-1); 
				//term.histi = (term.histi - 1 + HISTSIZE) % HISTSIZE; //??? uh. negative number, I guess
				SWAPp( term.hist[term.cthist][term.histi], term.line[term.bot] );
		}

		tsetdirt(orig, term.bot - n);
		tclearregion(0, term.bot - n + 1, term.col - 1, term.bot);

		for (i = term.bot; i >= orig + n; i--) {
				SWAPp( term.line[i], term.line[i-n] );
		}

		selscroll(orig, n);
}

void tscrollup(int orig, int n, int copyhist) {
		int i;

		dbg2("==== tscrollup, guard: %x orig %d, n %d, copyhist: %d\n",term.guard, orig,n, copyhist);
		LIMIT(n, 0, term.bot - orig + 1);

		if (copyhist) {
				dbg2("term.histi: %d\n", term.histi);
				term.histi = ((term.histi + 1) ^ HISTSIZE ) & (HISTSIZE-1);
				dbg2("term.histi: %d, \n", term.histi);
				if ( term.histi == 0 )
						term.circledhist=1;


				if ( term.hist[term.cthist][term.histi] ){
						dbg2("SWAP cthist %d, histi %d, orig %d\n", term.cthist, term.histi, orig);
						SWAPp( term.hist[term.cthist][term.histi], term.line[orig] );
				}	 else {
						dbg2("New line, cthist %d, term.histi: %d, term.col: %d\n", term.cthist, term.histi, term.col);
						term.hist[term.cthist][term.histi] = term.line[orig];
						term.line[orig] = xmalloc( term.col * sizeof(Glyph));
				}
				// (candidate for swap or, malloc hist here) done.
				// "compression" might take place here, as well.
				// sort of count*glyph for adjacent equal glyphs.
				// Maybe another text attribute. Then, the next glyph
				// as union int gives the count. Giving for, e.g. an empty line
				// with 200 cols a compression ratio of 200/2 (misc)
		}

		if (term.scr > 0 && term.scr < HISTSIZE) {
				term.scr = MIN(term.scr + n, HISTSIZE - 1);
		}

		tclearregion(0, orig, term.col - 1, orig + n - 1);
		tsetdirt(orig + n, term.bot);

		for (i = orig; i <= term.bot - n; i++) {
				SWAPp(term.line[i],term.line[i+n]);
		}

		selscroll(orig, -n);
}
void tnewline(int first_col) {
		int y = term.c.y;

		if (y == term.bot) {
				tscrollup(term.top, 1, 1);
		} else {
				y++;
		}
		tmoveto(first_col ? 0 : term.c.x, y);
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

/* for absolute user moves, when decom is set */
void tmoveato(int x, int y) {
		tmoveto(x, y + ((term.c.state & CURSOR_ORIGIN) ? term.top : 0));
}

void tmoveto(int x, int y) {
		int miny, maxy;

		if (term.c.state & CURSOR_ORIGIN) {
				miny = term.top;
				maxy = term.bot;
		} else {
				miny = 0;
				maxy = term.row - 1;
		}
		term.c.state &= ~CURSOR_WRAPNEXT;
		term.c.x = LIMIT(x, 0, term.col - 1);
		term.c.y = LIMIT(y, miny, maxy);
}

void tsetchar(Rune u, Glyph *attr, int x, int y) {
#ifdef UTF8
		static char *vt100_0[62] = {
				/* 0x41 - 0x7e */
				"↑", "↓", "→", "←", "█", "▚", "☃",      /* A - G */
				0,   0,   0,   0,   0,   0,   0,   0,   /* H - O */
				0,   0,   0,   0,   0,   0,   0,   0,   /* P - W */
				0,   0,   0,   0,   0,   0,   0,   " ", /* X - _ */
				"◆", "▒", "␉", "␌", "␍", "␊", "°", "±", /* ` - g */
				"␤", "␋", "┘", "┐", "┌", "└", "┼", "⎺", /* h - o */
				"⎻", "─", "⎼", "⎽", "├", "┤", "┴", "┬", /* p - w */
				"│", "≤", "≥", "π", "≠", "£", "·",      /* x - ~ */
		};

		/*
		 * The table is proudly stolen from rxvt.
		 */
		if (term.trantbl[term.charset] == CS_GRAPHIC0 &&
						BETWEEN(u, 0x41, 0x7e) && vt100_0[u - 0x41])
				utf8decode(vt100_0[u - 0x41], &u, UTF_SIZ);

		if (term.line[y][x].mode & ATTR_WIDE) {
				dbg2("Attr_wide: %c\n",term.line[y][x].u);
				if (x + 1 < term.col) {
						term.line[y][x + 1].u = ' ';
						term.line[y][x + 1].mode &= ~ATTR_WDUMMY;
				}
		} else if (term.line[y][x].mode & ATTR_WDUMMY) {
				dbg2("Attr_dummy: %c\n",term.line[y][x].u);
				term.line[y][x - 1].u = ' ';
				term.line[y][x - 1].mode &= ~ATTR_WIDE;
		}
#endif

		term.dirty[y] = 1;
		term.line[y][x] = *attr; //misc optimize here
		term.line[y][x].u = u;
}

void memset32( uint32_t* i, uint32_t value, int count ){
		for ( int a=0; a<count; a++ )
				i[a] = value;
}

void tclearregion(int x1, int y1, int x2, int y2) {
		int x, y;
		Glyph *gp;

		if (x1 > x2) {
				SWAPint(x1,x2);
		}
		if (y1 > y2) {
				SWAPint(y1,y2);
		}

		LIMIT(x1, 0, term.col - 1);
		LIMIT(x2, 0, term.col - 1);
		LIMIT(y1, 0, term.row - 1);
		LIMIT(y2, 0, term.row - 1);

		selclear(); // only call once.
		//term.c.attr.u=' ';

		for (y = y1; y <= y2; y++) { 
				term.dirty[y] = 1;
#ifndef UTF8
				dbg("y: %d, x1: %d, x2: %d\n", y, x1, x2);
				memset32( &term.line[y][x1].intG, term.c.attr.intG, (x2-x1)+1 ); // memset64 or comp
#else
				for (x = x1; x <= x2; x++) {//misc copy longs (64bit)or,better: memset. mmx/sse?
						//if (selected(x, y)) { // room for optimization. only ask once, when no selection
						//		selclear();
						//}
						gp = &term.line[y][x];
						gp->fg = term.c.attr.fg;
						gp->bg = term.c.attr.bg;
						gp->mode = 0;
						gp->u = ' ';
				}
#endif
		}
}

void tdeletechar(int n) {
		int dst, src, size;
		Glyph *line;

		LIMIT(n, 0, term.col - term.c.x);

		dst = term.c.x;
		src = term.c.x + n;
		size = term.col - src;
		line = term.line[term.c.y];

		memmove(&line[dst], &line[src], size * sizeof(Glyph));
		tclearregion(term.col - n, term.c.y, term.col - 1, term.c.y);
}

void tinsertblank(int n) {
		int dst, src, size;
		Glyph *line;

		LIMIT(n, 0, term.col - term.c.x);

		dst = term.c.x + n;
		src = term.c.x;
		size = term.col - dst;
		line = term.line[term.c.y];

		memmove(&line[dst], &line[src], size * sizeof(Glyph));
		tclearregion(src, term.c.y, dst - 1, term.c.y);
}

void tinsertblankline(int n) {
		if (BETWEEN(term.c.y, term.top, term.bot)) {
				tscrolldown(term.c.y, n, 0);
		}
}

void tdeleteline(int n) {
		if (BETWEEN(term.c.y, term.top, term.bot)) {
				tscrollup(term.c.y, n, 0);
		}
}

int32_t tdefcolor(int *attr, int *npar, int l) {
		int32_t idx = -1;
		uint r, g, b;

		switch (attr[*npar + 1]) {
				case 2: /* direct color in RGB space */
						if (*npar + 4 >= l) {
								fprintf(stderr, "erresc(38): Incorrect number of parameters (%d)\n",
												*npar);
								break;
						}
						r = attr[*npar + 2];
						g = attr[*npar + 3];
						b = attr[*npar + 4];
						*npar += 4;
						if (!BETWEEN(r, 0, 255) || !BETWEEN(g, 0, 255) || !BETWEEN(b, 0, 255)) {
								fprintf(stderr, "erresc: bad rgb color (%u,%u,%u)\n", r, g, b);
						} else {
								idx = TRUECOLOR(r, g, b);
						}
						break;
				case 5: /* indexed color */
						if (*npar + 2 >= l) {
								fprintf(stderr, "erresc(38): Incorrect number of parameters (%d)\n",
												*npar);
								break;
						}
						*npar += 2;
						if (!BETWEEN(attr[*npar], 0, 255)) {
								fprintf(stderr, "erresc: bad fgcolor %d\n", attr[*npar]);
						} else {
								idx = attr[*npar];
						}
						break;
				case 0: /* implemented defined (only foreground) */
				case 1: /* transparent */
				case 3: /* direct color in CMY space */
				case 4: /* direct color in CMYK space */
				default:
						fprintf(stderr, "erresc(38): gfx attr %d unknown\n", attr[*npar]);
						break;
		}

		return idx;
}

void tsetattr(int *attr, int l) {
		int i;
		int32_t idx;

		for (i = 0; i < l; i++) {
				switch (attr[i]) {
						case 0:
								term.c.attr.mode &=
										~(ATTR_BOLD | ATTR_FAINT | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_BLINK |
														ATTR_REVERSE | ATTR_INVISIBLE | ATTR_STRUCK);
								term.c.attr.fg = defaultfg;
								term.c.attr.bg = defaultbg;
								break;
						case 1:
								term.c.attr.mode |= ATTR_BOLD;
								break;
						case 2:
								term.c.attr.mode |= ATTR_FAINT;
								break;
						case 3:
								term.c.attr.mode |= ATTR_ITALIC;
								break;
						case 4:
								term.c.attr.mode |= ATTR_UNDERLINE;
								break;
						case 5: /* slow blink */
								/* FALLTHROUGH */
						case 6: /* rapid blink */
								term.c.attr.mode |= ATTR_BLINK;
								break;
						case 7:
								term.c.attr.mode |= ATTR_REVERSE;
								break;
						case 8:
								term.c.attr.mode |= ATTR_INVISIBLE;
								break;
						case 9:
								term.c.attr.mode |= ATTR_STRUCK;
								break;
						case 22:
								term.c.attr.mode &= ~(ATTR_BOLD | ATTR_FAINT);
								break;
						case 23:
								term.c.attr.mode &= ~ATTR_ITALIC;
								break;
						case 24:
								term.c.attr.mode &= ~ATTR_UNDERLINE;
								break;
						case 25:
								term.c.attr.mode &= ~ATTR_BLINK;
								break;
						case 27:
								term.c.attr.mode &= ~ATTR_REVERSE;
								break;
						case 28:
								term.c.attr.mode &= ~ATTR_INVISIBLE;
								break;
						case 29:
								term.c.attr.mode &= ~ATTR_STRUCK;
								break;
						case 38:
								if ((idx = tdefcolor(attr, &i, l)) >= 0) {
										term.c.attr.fg = idx;
								}
								break;
						case 39:
								term.c.attr.fg = defaultfg;
								break;
						case 48:
								if ((idx = tdefcolor(attr, &i, l)) >= 0) {
										term.c.attr.bg = idx;
								}
								break;
						case 49:
								term.c.attr.bg = defaultbg;
								break;
						default:
								if (BETWEEN(attr[i], 30, 37)) {
										term.c.attr.fg = attr[i] - 30;
								} else if (BETWEEN(attr[i], 40, 47)) {
										term.c.attr.bg = attr[i] - 40;
								} else if (BETWEEN(attr[i], 90, 97)) {
										term.c.attr.fg = attr[i] - 90 + 8;
								} else if (BETWEEN(attr[i], 100, 107)) {
										term.c.attr.bg = attr[i] - 100 + 8;
								} else {
										fprintf(stderr, "erresc(default): gfx attr %d unknown\n", attr[i]);
										csidump();
								}
								break;
				}
		}
}

void tsetscroll(int t, int b) {
		LIMIT(t, 0, term.row - 1);
		LIMIT(b, 0, term.row - 1);
		if (t > b) {
				//		SWAPint( t,b );
				term.top = b;
				term.bot = t;
		} else {
				term.top = t;
				term.bot = b;
		}
}

void tsetmode(int priv, int set, int *args, int narg) {
		int alt, *lim;

		for (lim = args + narg; args < lim; ++args) {
				if (priv) {
						switch (*args) {
								case 1: /* DECCKM -- Cursor key */
										xsetmode(set, MODE_APPCURSOR);
										break;
								case 5: /* DECSCNM -- Reverse video */
										xsetmode(set, MODE_REVERSE);
										break;
								case 6: /* DECOM -- Origin */
										MODBIT(term.c.state, set, CURSOR_ORIGIN);
										tmoveato(0, 0);
										break;
								case 7: /* DECAWM -- Auto wrap */
										MODBIT(term.mode, set, MODE_WRAP);
										break;
								case 0:  /* Error (IGNORED) */
								case 2:  /* DECANM -- ANSI/VT52 (IGNORED) */
								case 3:  /* DECCOLM -- Column  (IGNORED) */
								case 4:  /* DECSCLM -- Scroll (IGNORED) */
								case 8:  /* DECARM -- Auto repeat (IGNORED) */
								case 18: /* DECPFF -- Printer feed (IGNORED) */
								case 19: /* DECPEX -- Printer extent (IGNORED) */
								case 42: /* DECNRCM -- National characters (IGNORED) */
								case 12: /* att610 -- Start blinking cursor (IGNORED) */
										break;
								case 25: /* DECTCEM -- Text Cursor Enable Mode */
										xsetmode(!set, MODE_HIDE);
										break;
								case 9: /* X10 mouse compatibility mode */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEX10);
										break;
								case 1000: /* 1000: report button press */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEBTN);
										break;
								case 1002: /* 1002: report motion on button press */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEMOTION);
										break;
								case 1003: /* 1003: enable all mouse motions */
										xsetpointermotion(set);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEMANY);
										break;
								case 1004: /* 1004: send focus events to tty */
										xsetmode(set, MODE_FOCUS);
										break;
								case 1006: /* 1006: extended reporting mode */
										xsetmode(set, MODE_MOUSESGR);
										break;
								case 1034:
										xsetmode(set, MODE_8BIT);
										break;
								case 1049: /* swap screen & set/restore cursor as xterm */
										if (!allowaltscreen) {
												break;
										}
										tcursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
										/* FALLTHROUGH */
								case 47: /* swap screen */
								case 1047:
										if (!allowaltscreen) {
												break;
										}
										alt = IS_SET(MODE_ALTSCREEN);
										if (alt) {
												tclearregion(0, 0, term.col - 1, term.row - 1);
										}
										if (set ^ alt) { /* set is always 1 or 0 */
												tswapscreen();
										}
										if (*args != 1049) {
												break;
										}
										/* FALLTHROUGH */
								case 1048:
										tcursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
										break;
								case 2004: /* 2004: bracketed paste mode */
										xsetmode(set, MODE_BRCKTPASTE);
										break;
										/* Not implemented mouse modes. See comments there. */
								case 1001: /* mouse highlight mode; can hang the
															terminal by design when implemented. */
								case 1005: /* UTF-8 mouse mode; will confuse
															applications not supporting UTF-8
															and luit. */
								case 1015: /* urxvt mangled mouse mode; incompatible
															and can be mistaken for other control
															codes. */
										break;
								default:
										fprintf(stderr, "erresc: unknown private set/reset mode %d\n", *args);
										break;
						}
				} else {
						switch (*args) {
								case 0: /* Error (IGNORED) */
										break;
								case 2:
										xsetmode(set, MODE_KBDLOCK);
										break;
								case 4: /* IRM -- Insertion-replacement */
										MODBIT(term.mode, set, MODE_INSERT);
										break;
								case 12: /* SRM -- Send/Receive */
										MODBIT(term.mode, !set, MODE_ECHO);
										break;
								case 20: /* LNM -- Linefeed/new line */
										MODBIT(term.mode, set, MODE_CRLF);
										break;
								default:
										fprintf(stderr, "erresc: unknown set/reset mode %d\n", *args);
										break;
						}
				}
		}
}

void csihandle(void) {
		char buf[40];
		int len;

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
						tmoveto(term.c.x, term.c.y - csiescseq.arg[0]);
						break;
				case 'B': /* CUD -- Cursor <n> Down */
				case 'e': /* VPR --Cursor <n> Down */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x, term.c.y + csiescseq.arg[0]);
						break;
				case 'i': /* MC -- Media Copy */
						switch (csiescseq.arg[0]) {
								case 0:
										tdump();
										break;
								case 1:
										tdumpline(term.c.y);
										break;
								case 2:
										tdumpsel();
										break;
								case 4:
										term.mode &= ~MODE_PRINT;
										break;
								case 5:
										term.mode |= MODE_PRINT;
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
						tmoveto(term.c.x + csiescseq.arg[0], term.c.y);
						break;
				case 'D': /* CUB -- Cursor <n> Backward */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x - csiescseq.arg[0], term.c.y);
						break;
				case 'E': /* CNL -- Cursor <n> Down and first col */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(0, term.c.y + csiescseq.arg[0]);
						break;
				case 'F': /* CPL -- Cursor <n> Up and first col */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(0, term.c.y - csiescseq.arg[0]);
						break;
				case 'g': /* TBC -- Tabulation clear */
						switch (csiescseq.arg[0]) {
								case 0: /* clear current tab stop */
										term.tabs[term.c.x] = 0;
										break;
								case 3: /* clear all the tabs */
										memset(term.tabs, 0, term.col * sizeof(*term.tabs));
										break;
								default:
										goto unknown;
						}
						break;
				case 'G': /* CHA -- Move to <col> */
				case '`': /* HPA */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(csiescseq.arg[0] - 1, term.c.y);
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
										tclearregion(term.c.x, term.c.y, term.col - 1, term.c.y);
										if (term.c.y < term.row - 1) {
												tclearregion(0, term.c.y + 1, term.col - 1, term.row - 1);
										}
										break;
								case 1: /* above */
										if (term.c.y > 1) {
												tclearregion(0, 0, term.col - 1, term.c.y - 1);
										}
										tclearregion(0, term.c.y, term.c.x, term.c.y);
										break;
								case 2: /* all */
										tclearregion(0, 0, term.col - 1, term.row - 1);
										break;
								default:
										goto unknown;
						}
						break;
				case 'K': /* EL -- Clear line */
						switch (csiescseq.arg[0]) {
								case 0: /* right */
										tclearregion(term.c.x, term.c.y, term.col - 1, term.c.y);
										break;
								case 1: /* left */
										tclearregion(0, term.c.y, term.c.x, term.c.y);
										break;
								case 2: /* all */
										tclearregion(0, term.c.y, term.col - 1, term.c.y);
										break;
						}
						break;
				case 'S': /* SU -- Scroll <n> line up */
						DEFAULT(csiescseq.arg[0], 1);
						tscrollup(term.top, csiescseq.arg[0], 0);
						break;
				case 'T': /* SD -- Scroll <n> line down */
						DEFAULT(csiescseq.arg[0], 1);
						tscrolldown(term.top, csiescseq.arg[0], 0);
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
						tclearregion(term.c.x, term.c.y, term.c.x + csiescseq.arg[0] - 1, term.c.y);
						break;
				case 'P': /* DCH -- Delete <n> char */
						DEFAULT(csiescseq.arg[0], 1);
						tdeletechar(csiescseq.arg[0]);
						break;
				case 'Z': /* CBT -- Cursor Backward Tabulation <n> tab stops */
						DEFAULT(csiescseq.arg[0], 1);
						tputtab(-csiescseq.arg[0]);
						break;
				case 'd': /* VPA -- Move to <row> */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveato(term.c.x, csiescseq.arg[0] - 1);
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
										snprintf(buf, sizeof(buf), "\033[%i;%iR", term.c.y + 1, term.c.x + 1);
								ttywrite(buf, len, 0);
						}
						break;
				case 'r': /* DECSTBM -- Set Scrolling Region */
						if (csiescseq.priv) {
								goto unknown;
						} else {
								DEFAULT(csiescseq.arg[0], 1);
								DEFAULT(csiescseq.arg[1], term.row);
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

void strhandle(void) {
		char *p = NULL, *dec;
		int j, narg, par;

		term.esc &= ~(ESC_STR_END | ESC_STR);
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
						term.mode |= ESC_DCS;
				case '_': /* APC -- Application Program Command */
				case '^': /* PM -- Privacy Message */
						return;
		}

		fprintf(stderr, "erresc: unknown str ");
		strdump();
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

void sendbreak(const Arg *arg) {
		if (tcsendbreak(cmdfd, 0)) {
				perror("Error sending break");
		}
}

void tprinter(char *s, size_t len) {
		if (iofd != -1 && xwrite(iofd, s, len) < 0) {
				perror("Error writing to output file");
				close(iofd);
				iofd = -1;
		}
}

void toggleprinter(const Arg *arg) { term.mode ^= MODE_PRINT; }

void printscreen(const Arg *arg) { tdump(); }

void printsel(const Arg *arg) { tdumpsel(); }

void tdumpsel(void) {
		char *ptr;

		if ((ptr = getsel())) {
				tprinter(ptr, strlen(ptr));
				free(ptr);
		}
}

void tdumpline(int n) {
		char buf[UTF_SIZ];
		Glyph *bp, *end;

		bp = &term.line[n][0];
		end = &bp[MIN(tlinelen(n), term.col) - 1];
		if (bp != end || bp->u != ' ') {
				for (; bp <= end; ++bp) {
						tprinter(buf, utf8encode(bp->u, buf));
				}
		}
		tprinter("\n", 1);
}

void tdump(void) {
		int i;

		for (i = 0; i < term.row; ++i) {
				tdumpline(i);
		}
}

void tputtab(int n) {
		uint x = term.c.x;

		if (n > 0) {
				while (x < term.col && n--) {
						for (++x; x < term.col && !term.tabs[x]; ++x) {
								/* nothing */
						}
				}
		} else if (n < 0) {
				while (x > 0 && n++) {
						for (--x; x > 0 && !term.tabs[x]; --x) {
								/* nothing */
						}
				}
		}
		term.c.x = LIMIT(x, 0, term.col - 1);
}

void tdefutf8(utfchar ascii) {
#ifdef UTF8
		if (ascii == 'G')
				term.mode |= MODE_UTF8;
		else
#endif
				if (ascii == '@') {
						term.mode &= ~MODE_UTF8;
				}
}

void tdeftran(utfchar ascii) {
		static char cs[] = "0B";
		static int vcs[] = {CS_GRAPHIC0, CS_USA};
		char *p;

		if ((p = strchr(cs, ascii)) == NULL) {
				fprintf(stderr, "esc unhandled charset: ESC ( %c\n", ascii);
		} else {
				term.trantbl[term.icharset] = vcs[p - cs];
		}
}

void tdectest(utfchar c) {
		int x, y;

		if (c == '8') { /* DEC screen alignment test. */
				for (x = 0; x < term.col; ++x) {
						for (y = 0; y < term.row; ++y) {
								tsetchar('E', &term.c.attr, x, y);
						}
				}
		}
}

void tstrsequence(uchar c) {
		strreset();

		switch (c) {
				case 0x90: /* DCS -- Device Control String */
						c = 'P';
						term.esc |= ESC_DCS;
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
		term.esc |= ESC_STR;
}

void tcontrolcode(uchar ascii) {
		switch (ascii) {
				case '\t': /* HT */
						tputtab(1);
						return;
				case '\b': /* BS */
						tmoveto(term.c.x - 1, term.c.y);
						return;
				case '\r': /* CR */
						tmoveto(0, term.c.y);
						return;
				case '\f': /* LF */
				case '\v': /* VT */
				case '\n': /* LF */
						/* go to first col if the mode is set */
						tnewline(IS_SET(MODE_CRLF));
						return;
				case '\a': /* BEL */
						if (term.esc & ESC_STR_END) {
								/* backwards compatibility to xterm */
								strhandle();
						} else {
								xbell();
						}
						break;
				case '\033': /* ESC */
						csireset();
						term.esc &= ~(ESC_CSI | ESC_ALTCHARSET | ESC_TEST);
						term.esc |= ESC_START;
						return;
				case '\016': /* SO (LS1 -- Locking shift 1) */
				case '\017': /* SI (LS0 -- Locking shift 0) */
						term.charset = 1 - (ascii - '\016');
						return;
				case '\032': /* SUB */
						tsetchar('?', &term.c.attr, term.c.x, term.c.y);
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
						term.tabs[term.c.x] = 1;
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
		term.esc &= ~(ESC_STR_END | ESC_STR);
}

/*
 * returns 1 when the sequence is finished and it hasn't to read
 * more characters for this sequence, otherwise 0
 */
int eschandle(uchar ascii) {
		switch (ascii) {
				case '[':
						term.esc |= ESC_CSI;
						return 0;
				case '#':
						term.esc |= ESC_TEST;
						return 0;
				case '%':
						term.esc |= ESC_UTF8;
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
						term.charset = 2 + (ascii - 'n');
						break;
				case '(': /* GZD4 -- set primary charset G0 */
				case ')': /* G1D4 -- set secondary charset G1 */
				case '*': /* G2D4 -- set tertiary charset G2 */
				case '+': /* G3D4 -- set quaternary charset G3 */
						term.icharset = ascii - '(';
						term.esc |= ESC_ALTCHARSET;
						return 0;
				case 'D': /* IND -- Linefeed */
						if (term.c.y == term.bot) {
								tscrollup(term.top, 1, 1);
						} else {
								tmoveto(term.c.x, term.c.y + 1);
						}
						break;
				case 'E':      /* NEL -- Next line */
						tnewline(1); /* always go to first col */
						break;
				case 'H': /* HTS -- Horizontal tab stop */
						term.tabs[term.c.x] = 1;
						break;
				case 'M': /* RI -- Reverse index */
						if (term.c.y == term.top) {
								tscrolldown(term.top, 1, 1);
						} else {
								tmoveto(term.c.x, term.c.y - 1);
						}
						break;
				case 'Z': /* DECID -- Identify Terminal */
						ttywrite(vtiden, strlen(vtiden), 0);
						break;
				case 'c': /* RIS -- Reset to initial state */
						treset();
						resettitle();
						xloadcols();
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
						if (term.esc & ESC_STR_END) {
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

void tputc(Rune u) {
		char c[UTF_SIZ];
		int control;
		int width, len;
		Glyph *gp;

		control = ISCONTROL(u);
#ifdef UTF8
		if (!IS_SET(MODE_UTF8) && !IS_SET(MODE_SIXEL)) {
#endif
				c[0] = u;
				width = len = 1;
#ifdef UTF8
		} else {
				len = utf8encode(u, c);
				if (!control && (width = wcwidth(u)) == -1) {
						memcpy(c, "\357\277\275", 4); // UTF_INVALID
						width = 1;
				}
		}
#endif

		if (IS_SET(MODE_PRINT)) {
				tprinter(c, len);
		}

		/*
		 * STR sequence must be checked before anything else
		 * because it uses all following characters until it
		 * receives a ESC, a SUB, a ST or any other C1 control
		 * character.
		 */
		if (term.esc & ESC_STR) {
				if (u == '\a' || u == 030 || u == 032 || u == 033 || ISCONTROLC1(u)) {
						term.esc &= ~(ESC_START | ESC_STR | ESC_DCS);
						if (IS_SET(MODE_SIXEL)) {
								/* TODO: render sixel */
								term.mode &= ~MODE_SIXEL;
								return;
						}
						term.esc |= ESC_STR_END;
						goto check_control_code;
				}

				if (IS_SET(MODE_SIXEL)) {
						/* TODO: implement sixel mode */
						return;
				}
				if (term.esc & ESC_DCS && strescseq.len == 0 && u == 'q') {
						term.mode |= MODE_SIXEL;
				}

				if (strescseq.len + len >= strescseq.siz) {
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
						 * term.esc = 0;
						 * strhandle();
						 */
						if (strescseq.siz > (SIZE_MAX - UTF_SIZ) / 2) {
								return;
						}
						strescseq.siz *= 2;
						strescseq.buf = xrealloc(strescseq.buf, strescseq.siz);
				}

				memmove(&strescseq.buf[strescseq.len], c, len);
				strescseq.len += len;
				return;
		}

check_control_code:
		/*
		 * Actions of control codes must be performed as soon they arrive
		 * because they can be embedded inside a control sequence, and
		 * they must not cause conflicts with sequences.
		 */
		if (control) {
				tcontrolcode(u);
				/*
				 * control codes are not shown ever
				 */
				return;
		} else if (term.esc & ESC_START) {
				if (term.esc & ESC_CSI) {
						csiescseq.buf[csiescseq.len++] = u;
						if (BETWEEN(u, 0x40, 0x7E) ||
										csiescseq.len >= sizeof(csiescseq.buf) - 1) {
								term.esc = 0;
								csiparse();
								csihandle();
						}
						return;
				} else if (term.esc & ESC_UTF8) {
						tdefutf8(u);
				} else if (term.esc & ESC_ALTCHARSET) {
						tdeftran(u);
				} else if (term.esc & ESC_TEST) {
						tdectest(u);
				} else {
						if (!eschandle(u)) {
								return;
						}
						/* sequence already finished */
				}
				term.esc = 0;
				/*
				 * All characters which form part of a sequence are not
				 * printed
				 */
				return;
		}
		if (sel.ob.x != -1 && BETWEEN(term.c.y, sel.ob.y, sel.oe.y)) {
				selclear();
		}

		gp = &term.line[term.c.y][term.c.x];
		if (IS_SET(MODE_WRAP) && (term.c.state & CURSOR_WRAPNEXT)) {
				gp->mode |= ATTR_WRAP; //misc wrapping here
				tnewline(1);
				gp = &term.line[term.c.y][term.c.x];
		}

		if (IS_SET(MODE_INSERT) && term.c.x + width < term.col) {
				memmove(gp + width, gp, (term.col - term.c.x - width) * sizeof(Glyph));
		}

		if (term.c.x + width > term.col) {
				tnewline(1);
				gp = &term.line[term.c.y][term.c.x];
		}

		tsetchar(u, &term.c.attr, term.c.x, term.c.y);

#ifdef UTF8
		if (width == 2) {
				dbg2("tputchar width2: %x %c", gp[0].u, gp[0].u );
				gp->mode |= ATTR_WIDE;
				if (term.c.x + 1 < term.col) {
						gp[1].u = '\0';
						gp[1].mode = ATTR_WDUMMY;
				}
		}
#endif

		if (term.c.x + width < term.col) {
				tmoveto(term.c.x + width, term.c.y);
		} else {
				term.c.state |= CURSOR_WRAPNEXT;
		}
}
#if 0
void histputc(utfchar c){
		gp = &term.line[term.c.y][term.c.x];
		if (IS_SET(MODE_WRAP) && (term.c.state & CURSOR_WRAPNEXT)) {
				gp->mode |= ATTR_WRAP;
				tnewline(1);
				gp = &term.line[term.c.y][term.c.x];
		}

		if (IS_SET(MODE_INSERT) && term.c.x + width < term.col) {
				memmove(gp + width, gp, (term.col - term.c.x - width) * sizeof(Glyph));
		}

		if (term.c.x + width > term.col) {
				tnewline(1);
				gp = &term.line[term.c.y][term.c.x];
		}

		tsetchar(u, &term.c.attr, term.c.x, term.c.y);


#ifdef UTF8
		if (width == 2) {
				dbg2("tputchar width2: %x %c", gp[0].u, gp[0].u );
				gp->mode |= ATTR_WIDE;
				if (term.c.x + 1 < term.col) {
						gp[1].u = '\0';
						gp[1].mode = ATTR_WDUMMY;
				}
		}
#endif

		if (term.c.x + width < term.col) {
				tmoveto(term.c.x + width, term.c.y);
		} else {
				term.c.state |= CURSOR_WRAPNEXT;
		}
}
#endif


int twrite(const utfchar *buf, int buflen, int show_ctrl) {
		int charsize;
		Rune u;
		int n;

		dbg("twrite0 buflen: %d buf[0]: %c  show_ctrl: %d\n", buflen, buf[0], show_ctrl);

#ifdef UTF8
		for (n = 0; n < buflen; n += charsize) { // misc dfq
				if (IS_SET(MODE_UTF8) && !IS_SET(MODE_SIXEL)) {
						// process a complete utf8 char
						charsize = utf8decode(buf + n, &u, buflen - n);
						if (charsize == 0)
								break;
				} else  {
						u = buf[n] & 0xFF;
						charsize = 1;
				}
#else
		for (n = 0; n < buflen; n++ ) { // misc dfq
					u = buf[n];
#endif
				dbg("twrite1 %d %c\n", u, u);
				if (show_ctrl && ISCONTROL(u)) {
						dbg("twrite ISCONTROL %d %c\n", u, u);
						if (u & 0x80) {
								u &= 0x7f;
								tputc('^');
								tputc('[');
						} else if (u != '\n' && u != '\r' && u != '\t') {
								u ^= 0x40;
								tputc('^');
						}
				}
				tputc(u);
		}
		return n;
}

void tresize(int col, int row) {
		int i, j;
		int minrow = MIN(row, term.row);
		int mincol = MIN(col, term.col);
		int *bp;
		TCursor c;

		if (row < term.row || col < term.col) {
				toggle_winmode(trt_kbdselect(XK_Escape, NULL, 0));
		}

		if (col < 1 || row < 1) {
				fprintf(stderr, "tresize: error resizing to %dx%d\n", col, row);
				return;
		}

		/*
		 * slide screen to keep cursor where we expect it -
		 * tscrollup would work here, but we can optimize to
		 * memmove because we're freeing the earlier lines
		 */
		for (i = 0; i <= term.c.y - row; i++) {
				free(term.line[i]);
				free(term.alt[i]);
		}
		/* ensure that both src and dst are not NULL */
		if (i > 0) {
				memmove(term.line, term.line + i, row * sizeof(Line));
				memmove(term.alt, term.alt + i, row * sizeof(Line));
		}
		for (i += row; i < term.row; i++) {
				free(term.line[i]);
				free(term.alt[i]);
		}

		/* resize to new height */
		term.line = xrealloc(term.line, row * sizeof(Line));
		term.alt = xrealloc(term.alt, row * sizeof(Line));
		term.dirty = xrealloc(term.dirty, row * sizeof(*term.dirty));
		term.tabs = xrealloc(term.tabs, col * sizeof(*term.tabs));

		int oldline = 0;
		int newline = 0;
		int oldcol = 0;
		int newcol = 0;
		int newhist = !(term.cthist);
		int oldhist = term.cthist;
		// delay here. Collect resize events
		if ( term.circledhist  ){
				oldline = (term.histi+1 > HISTSIZE ) ? 0 : (term.histi+1);
		}
		term.c.attr.u = ' '; 
		dbg2("oldline: %d  term.histi: %d  term.col: %d col: %d\n",oldline,term.histi, term.col, col);
#if 0

		if ( oldline != term.histi ){
				term.hist[newhist][newline] = xmalloc( col * sizeof(Glyph));
				memset32( &term.hist[newhist][newline][mincol].intG, term.c.attr.intG, col-mincol );
		}

		while (oldline!=term.histi) { // Didn't reach the end of the old history yet
				dbg3( "oldhist: %d term.col %d newhist %d oldline: %d oldcol: %d newline: %d newcol: %d\n", oldhist, term.col,newhist, oldline, oldcol, newline, newcol );
				while( oldcol < term.col && !( ( oldcol>0 ) && (term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP )) ){
						dbg3( "term.col: %d L2: oldline: %d oldcol: %d newline: %d newcol: %d\n",term.col, oldline, oldcol, newline, newcol );
						//dbg3( "intG oldhist: %d - %d\n", term.hist[oldhist][oldline][oldcol].intG, term.hist[oldhist][oldline][oldcol].u );
						if ( term.hist[oldhist][oldline][oldcol].mode & ATTR_WRAP ){
								dbg2("WRAP\n");
						}
						term.hist[newhist][newline][newcol].intG = term.hist[oldhist][oldline][oldcol].intG;
						//term.hist[newhist][newline][newcol].mode
						oldcol++;
						newcol++;
						if ( ( newcol == col) || ( term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP ) ){ // end of line
								dbg3("Eol. newcol: %d  oldcol:%d\n",newcol,oldcol);
								term.hist[newhist][newline][newcol-1].mode |= ATTR_WRAP;
								newline++;
								newline &= ((1<<HISTSIZEBITS)-1);
								newcol=0;
								term.hist[newhist][newline] = xmalloc( col * sizeof(Glyph));
								dbg3("newline: %d\n",newline);
								memset32( &term.hist[newhist][newline][mincol].intG, term.c.attr.intG, col-mincol );
						}
						if ( oldcol < term.col && !( ( oldcol>0 ) && (term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP )) ){
								dbg3( "YY: newline: %d newcol: %d\n", newline, newcol );
								free( term.hist[oldhist][oldline] );
								oldcol = 0;
								term.hist[oldhist][oldline] = 0;
								oldline++;
								oldline &= ((1<<HISTSIZEBITS)-1); // modulo

						}

				}
		}
		term.cthist = newhist;
		dbg2("copied hist. oldhist: %d  term.cthist: %d\n", oldhist, term.cthist );
#else
		int t = term.histi;
		if ( term.circledhist  ){
				t = HISTSIZE;
		}

		for (i = 0; i < t; i++) { // 
				term.hist[(term.cthist)][i] = xrealloc(term.hist[term.cthist][i], col * sizeof(Glyph));
#ifndef UTF8
				memset32( &term.hist[term.cthist][i][mincol].intG, term.c.attr.intG, col-mincol );
				//for (j = mincol; j < col; j++) {
				//		term.hist[term.cthist][i][j].intG = term.c.attr.intG;
				//}
#else
				for (j = mincol; j < col; j++) {
						//term.hist[term.cthist][i][j].intG = term.c.attr.intG;
						term.hist[term.cthist][i][j] = term.c.attr;
						term.hist[term.cthist][i][j].u = ' '; 
						//append empty chars, if more cols than before
				}
#endif
		}

#endif
		/* resize each row to new width, zero-pad if needed */
		for (i = 0; i < minrow; i++) {
				//dbg3("i: %d, %d", i, minrow );
				term.line[i] = xrealloc(term.line[i], col * sizeof(Glyph));
				//dbg3("i2\n");
				term.alt[i] = xrealloc(term.alt[i], col * sizeof(Glyph));
		}

		dbg3("i4\n");
		/* allocate any new rows */
		for (/* i = minrow */; i < row; i++) {
				term.line[i] = xmalloc(col * sizeof(Glyph));
				term.alt[i] = xmalloc(col * sizeof(Glyph));
		}
		if (col > term.col) {
				bp = term.tabs + term.col;

				memset(bp, 0, sizeof(*term.tabs) * (col - term.col));
				while (--bp > term.tabs && !*bp) {
						/* nothing */
				}
				for (bp += tabspaces; bp < term.tabs + col; bp += tabspaces) {
						*bp = 1;
				}
		}
		/* update terminal size */
		term.col = col;
		term.row = row;
		/* reset scrolling region */
		tsetscroll(0, row - 1);
		/* make use of the LIMIT in tmoveto */
		tmoveto(term.c.x, term.c.y);
		/* Clearing both screens (it makes dirty all lines) */
		c = term.c;
		for (i = 0; i < 2; i++) {
				if (mincol < col && 0 < minrow) {
						tclearregion(mincol, 0, col - 1, minrow - 1);
				}
				if (0 < col && minrow < row) {
						tclearregion(0, minrow, col - 1, row - 1);
				}
				tswapscreen();
				tcursor(CURSOR_LOAD);
		}
		term.c = c;
}

void resettitle(void) { xsettitle(NULL); }

void drawregion(int x1, int y1, int x2, int y2) {
		int y;
		for (y = y1; y < y2; y++) {
				if (!term.dirty[y]) {
						continue;
				}
				term.dirty[y] = 0;
				xdrawline(TLINE(y), x1, y, x2);
		}
}

void draw(void) {
		int cx = term.c.x;

		if (!xstartdraw()) {
				return;
		}

		/* adjust cursor position */
		LIMIT(term.ocx, 0, term.col - 1);
		LIMIT(term.ocy, 0, term.row - 1);

#ifdef UTF8
		if (term.line[term.ocy][term.ocx].mode & ATTR_WDUMMY) {
				term.ocx--;
		}
		if (term.line[term.c.y][cx].mode & ATTR_WDUMMY) {
				cx--;
		}
#endif 

		drawregion(0, 0, term.col, term.row);
		if (term.scr == 0) {
				xdrawcursor(cx, term.c.y, term.line[term.c.y][cx], term.ocx, term.ocy,
								term.line[term.ocy][term.ocx]);
		}
		term.ocx = cx, term.ocy = term.c.y;
		xfinishdraw();
		xximspot(term.ocx, term.ocy);
}

void redraw(void) {
		tfulldirt();
		draw();
}

void set_notifmode(int type, KeySym ksym) {
		static char *lib[] = {" MOVE ", " SEL  "};
		static Glyph *g, *deb, *fin;
		static int col, bot;

		if (ksym == -1) {
				free(g);
				col = term.col, bot = term.bot;
				g = xmalloc(col * sizeof(Glyph));
				memcpy(g, term.line[bot], col * sizeof(Glyph));
		} else if (ksym == -2) {
				memcpy(term.line[bot], g, col * sizeof(Glyph));
		}

		if (type < 2) {
				char *z = lib[type];
				for (deb = &term.line[bot][col - 6], fin = &term.line[bot][col]; deb < fin;
								z++, deb++) {
						deb->mode = ATTR_REVERSE, deb->u = *z, deb->fg = defaultfg,
								deb->bg = defaultbg;
				}
		} else if (type < 5) {
				memcpy(term.line[bot], g, col * sizeof(Glyph));
		} else {
				for (deb = &term.line[bot][0], fin = &term.line[bot][col]; deb < fin;
								deb++) {
						deb->mode = ATTR_REVERSE, deb->u = ' ', deb->fg = defaultfg,
								deb->bg = defaultbg;
				}
				term.line[bot][0].u = ksym;
		}

		term.dirty[bot] = 1;
		drawregion(0, bot, col, bot + 1);
}

