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
	sa_to.sa_mask.__val[0] = 0;

	if (sigaction(SIGALRM, &sa_to, NULL) == -1)
	{
		if (verbose)
			printf("sigaction failed: %s\n", strsignal(errno)); // Replace perror
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
				// Wait until child is reaped (handle EINTR)
				while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
					;
				ret = 0;
				break;
			}
			else if (errno == EINTR)
			{
				// Retry waitpid if interrupted by other signals
				if (waitpid(pid, &status, WNOHANG) == -1 && errno != EINTR)
					break;
				continue;
			}
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

void abort_func(void) {
    abort();  // Raises SIGABRT
}

void close_call_func(void) {
    sleep(1);  // With 2 second timeout, this should succeed
}

void memory_hog_func(void) {
    void *p;
    while ((p = malloc(1024 * 1024 * 10))) {
        memset(p, 0, 1024 * 1024 * 10);
    }
}

void signal_ignorer_func(void) {
    signal(SIGTERM, SIG_IGN);
    while(1);
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
    
    printf("\n=== Test abort function ===\n");
    sandbox(abort_func, 2, true);
    
    printf("\n=== Test close call function ===\n");
    sandbox(close_call_func, 2, true);
    
    printf("\n=== Test memory hog function ===\n");
    sandbox(memory_hog_func, 2, true); 
    
    printf("\n=== Test signal ignorer function ===\n");
    sandbox(signal_ignorer_func, 2, true);

    return 0;
}
