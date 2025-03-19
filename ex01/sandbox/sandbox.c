#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

int	g_to;

void	handle_timeout(int sig)
{
	if (sig == SIGALRM)
		g_to = 1;
}

int	sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	int pid;
	int status;
	struct sigaction sa_to = {0};
	int ret = -1;

	g_to = 0;
	sa_to.sa_handler = handle_timeout;
	if (sigemptyset(&sa_to.sa_mask) == -1)
	{
		if (verbose)
			perror("sigemptyset failed");
		return (-1);
	}
	if (sigaction(SIGALRM, &sa_to, NULL) == -1)
	{
		if (verbose)
			perror("sigaction failed");
		return (-1);
	}
	
	if ((pid = fork()) == -1)
		return (-1);
	if (pid == 0)
	{
		f();
		exit(0);
	}
	alarm(timeout);
	
	if (waitpid(pid, &status, 0) == -1)
	{
		while (1)
		{
			if (errno == EINTR && g_to)
			{
				if (verbose)
					printf("Bad function: timed out after %u seconds\n", timeout);
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				ret = 0;
				break;
			}
			else if (errno == EINTR)
				continue;
			else
			{
				ret = -1;
				break;
			}
		}
	}
	else
	{
		if (WIFSIGNALED(status))
		{
			if (verbose)
				printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
			ret = 0;
		}
		else if (WIFEXITED(status))
		{
			int ec = WEXITSTATUS(status);
			if (ec != 0 && verbose)
				printf("Bad function: exited with code %d\n", ec);
			ret = (ec == 0) ? 1 : 0;
			if (ec == 0 && verbose)
				printf("Nice function!\n");
		}
		else
			ret = -1;
	}
	alarm(0);
	return (ret);
}

void good_func(void) {
    // This should work
}

void bad_func(void) {
    while(1); // Infinite loop
}

void segfault_func(void) {
    *(int *)0 = 42; // Intentional segmentation fault
}

void exit_nonzero_func(void) {
    exit(42); // Explicit non-zero exit
}

int main(void) {
    printf("=== Test successful function ===\n");
    sandbox(good_func, 2, true);
    
    printf("\n=== Test timeout ===\n");
    sandbox(bad_func, 2, true);
    
    printf("\n=== Test segmentation fault ===\n");
    sandbox(segfault_func, 2, true);
    
    printf("\n=== Test non-zero exit ===\n");
    sandbox(exit_nonzero_func, 2, true);

    return 0;
}
