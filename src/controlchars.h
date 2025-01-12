
// handle control chars and sequences, ansi escape sequences.
//

#pragma once

#define ISCONTROLC0(c) (BETWEEN(c, 0, 0x1f) || (c) == 0x7f) // 0x7f = delete

//#define ISCONTROLC1(c) (BETWEEN(c, 0x80, 0x9f))
#define ISCONTROLC1(c) (0)

#define ISCONTROL(c) ((c <= 0x1f) || (c==0x7f))
//#define ISCONTROL(c) ((c <= 0x1f) || BETWEEN(c, 0x7f, 0x9f))
//#define ISCONTROL(c) (0)




#define ESC_BUF_SIZ (256 * UTF_SIZ) // UTF_SIZ is 4 with utf8, 1 without
#define ESC_ARG_SIZ 16
#define STR_BUF_SIZ ESC_BUF_SIZ
#define STR_ARG_SIZ ESC_ARG_SIZ


enum escape_state {
  ESC_START = 1,
  ESC_CSI = 2,
  ESC_STR = 4, /* OSC, PM, APC */
  ESC_ALTCHARSET = 8,
  ESC_STR_END = 16, /* a final string was encountered */
  ESC_TEST = 32,    /* Enter in test mode */
  ESC_UTF8 = 64,
  ESC_DCS = 128,
};



/* CSI Escape sequence structs */
/* ESC '[' [[ [<priv>] <arg> [;]] <mode> [<mode>]] */
typedef struct {
  char buf[ESC_BUF_SIZ]; /* raw string */
  size_t len;            /* raw string length */
  char priv;
  int arg[ESC_ARG_SIZ];
  int narg; /* nb of args */
  char mode[2];
} CSIEscape;

/* STR Escape sequence structs */
/* ESC type [[ [<priv>] <arg> [;]] <mode>] ESC '\' */
typedef struct {
  char type;  /* ESC type ... */
  char *buf;  /* allocated raw string */
  size_t siz; /* allocation size */
  size_t len; /* raw string length */
  char *args[STR_ARG_SIZ];
  int narg; /* nb of args */
} STREscape;



#ifdef UTF8
int handle_controlchars( Rune u, uint decoded_len, char* decoded_char );
#else
int _handle_controlchars( Rune u );
#define handle_controlchars( _rune, ... ) _handle_controlchars( _rune )
#endif


static void tdefutf8(utfchar);
static void tdeftran(utfchar);
static void tsetmode(int, int, int *, int);

static void tstrsequence(uchar);
static void tcontrolcode(uchar);


static void csidump(void);
static void csihandle(void);
static void csiparse(void);
static void csireset(void);
static int eschandle(uchar);

static void strdump(void);
static void strparse(void);
static void strreset(void);





