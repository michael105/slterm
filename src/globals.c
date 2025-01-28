// globals.

// need to check, in which files they are used.


// pid of executed shell 
pid_t shellpid;

Term *term=0;
Term *p_term=0;
Term *p_help=0;
Term *p_help_storedterm=0;
Term *p_alt=0;


// term.c
int enterlessmode;


XWindow xwin;
TermWindow twin;




// in selection.c
//Selection sel;
//XSelection xsel;


// overwrite the (only) dependency of fontconfig to lbintl, and therefore iconv.
// The internationalization is used solely for the font info name.
// adding nearly 1MB to the memory usage
const char *libintl_dgettext( const char* domain, const char* text){
	return(text);
}



#ifndef WITHBZIP2
// bzip2 dummies
// only needed to load bzip2 compressed fonts. (seems so..)
int BZ2_bzDecompressEnd(void*v, int a,int b){ return 0; };
int BZ2_bzDecompress( void*v ){ return 0; };
int BZ2_bzDecompressInit( void*v, int a,int b ){ die("bzip2 not linked. remove dummie functions, and relink\n"); return 0; };
#endif

