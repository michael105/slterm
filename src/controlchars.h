
// handle control chars and sequences, ansi escape sequences.
//

#pragma once


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





