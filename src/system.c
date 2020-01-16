// system related. fork, exec, die

#include "includes.h"


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



