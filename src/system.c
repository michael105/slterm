// system related. fork, exec, die

#include "includes.h"


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

		setenv("NORM", "\e[0;37;40m", 1);

		setenv("BLACK", "\e[30m", 1);
		setenv("RED", "\e[31m", 1);
		setenv("GREEN", "\e[32m", 1);
		setenv("YELLOW", "\e[33m", 1);
		setenv("BLUE", "\e[34m", 1);
		setenv("MAGENTA", "\e[35m", 1);
		setenv("CYAN", "\e[36m", 1);
		setenv("WHITE", "\e[37m", 1);

		setenv("BROWN", "\e[33m", 1);
		setenv("BGBROWN", "\e[43m", 1);
		setenv("ORANGE", "\e[1;2;33m", 1);
		setenv("ORANGERED", "\e[1;2;31m", 1);
		setenv("GRAY", "\e[1;2;30m", 1);
		setenv("PURPLE", "\e[1;2;35m", 1);
		setenv("MINT", "\e[1;2;32m", 1);
		setenv("TURQUOISE", "\e[1;2;36m", 1);

		setenv("LBLACK", "\e[90m", 1);
		setenv("LRED", "\e[91m", 1);
		setenv("LGREEN", "\e[92m", 1);
		setenv("LYELLOW", "\e[93m", 1);
		setenv("LBLUE", "\e[94m", 1);
		setenv("LMAGENTA", "\e[95m", 1);
		setenv("LCYAN", "\e[96m", 1);
		setenv("LWHITE", "\e[97m", 1);

		setenv("DBLACK", "\e[2;30m", 1);
		setenv("DRED", "\e[2;31m", 1);
		setenv("DGREEN", "\e[2;32m", 1);
		setenv("DYELLOW", "\e[2;33m", 1);
		setenv("DBLUE", "\e[2;34m", 1);
		setenv("DMAGENTA", "\e[2;35m", 1);
		setenv("DCYAN", "\e[2;36m", 1);
		setenv("DWHITE", "\e[2;37m", 1);

		setenv("LDBLACK", "\e[1;2;30m", 1);
		setenv("LDRED", "\e[1;2;31m", 1);
		setenv("LDGREEN", "\e[1;2;32m", 1);
		setenv("LDYELLOW", "\e[1;2;33m", 1);
		setenv("LDBLUE", "\e[1;2;34m", 1);
		setenv("LDMAGENTA", "\e[1;2;35m", 1);
		setenv("LDCYAN", "\e[1;2;36m", 1);
		setenv("LDWHITE", "\e[1;2;37m", 1);


		setenv("BGBLACK", "\e[40m", 1);
		setenv("BGRED", "\e[41m", 1);
		setenv("BGGREEN", "\e[42m", 1);
		setenv("BGYELLOW", "\e[43m", 1);
		setenv("BGBLUE", "\e[44m", 1);
		setenv("BGMAGENTA", "\e[45m", 1);
		setenv("BGCYAN", "\e[46m", 1);
		setenv("BGWHITE", "\e[47m", 1);

		setenv("BGLBLACK", "\e[100m", 1);
		setenv("BGLRED", "\e[101m", 1);
		setenv("BGLGREEN", "\e[102m", 1);
		setenv("BGLYELLOW", "\e[103m", 1);
		setenv("BGLBLUE", "\e[104m", 1);
		setenv("BGLMAGENTA", "\e[105m", 1);
		setenv("BGLCYAN", "\e[106m", 1);
		setenv("BGLWHITE", "\e[107m", 1);


		setenv("BOLD","\e[1m",1);
		setenv("FAINT","\e[2m",1);
		setenv("CURSIVE","\e[3m",1);
		setenv("UNDERLINE","\e[4m",1);
		setenv("BLINK","\e[6m",1);
		setenv("REVERSE","\e[7m",1);
		setenv("STRIKETHROUGH","\e[9m",1);
		setenv("DOUBLEUNDERLINE","\e[21m",1);


		signal(SIGCHLD, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGALRM, SIG_DFL);

		execvp(prog, args);
		// didn't work. 
		// try default, set in config.h.in
		execvp(shell,args);
		_exit(1);
}

void sigchld(int a) {
		int stat;
		pid_t p;

		if ((p = waitpid(shellpid, &stat, WNOHANG)) < 0) {
				die("waiting for pid %hd failed: %s\n", shellpid, strerror(errno));
		}

		if (shellpid != p) {
				return;
		}

		if (WIFEXITED(stat) && WEXITSTATUS(stat)) {
				die("child exited with status %d\n", WEXITSTATUS(stat));
		} else if (WIFSIGNALED(stat)) {
				die("child terminated due to signal %d\n", WTERMSIG(stat));
		}
		exit(0);
}



