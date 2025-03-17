#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int close_pipes(int pipes[][2], int num)
{
	int i = 0;
	int status = 1;

	while (i < num)
	{
		if (close(pipes[i][0]) == -1)
			status = 0;
		if (close(pipes[i][1]) == -1)
			status = 0;
		i++;
	}
	return (status);
}

// Helper to close extra file descriptors, if desired.
void close_unused_fds(int keep_stdin, int keep_stdout) {
	for (int fd = 3; fd < 1024; fd++) {
		if ((keep_stdin && fd == 0) || (keep_stdout && fd == 1) || fd == 2)
			continue;
		close(fd);
	}
}

int picoshell(char **cmds[])
{
	int num_cmds = 0;
	// Count number of commands
	while (cmds[num_cmds] != NULL) {
		num_cmds++;
	}
	
	pid_t *pids = malloc(sizeof(pid_t) * num_cmds);
	if (!pids) {
		return 1;
	}

	int prev_pipe_read = -1;
	
	for (int i = 0; i < num_cmds; i++)
	{
		int curr_pipe[2];
		if (i < num_cmds - 1) {
			if (pipe(curr_pipe) == -1) {
				close(prev_pipe_read);
				free(pids);
				return 1;
			}
		}

		pids[i] = fork();
		if (pids[i] == -1) {
			close(prev_pipe_read);
			if (i < num_cmds - 1) {
				close(curr_pipe[0]);
				close(curr_pipe[1]);
			}
			free(pids);
			return 1;
		}
		if (pids[i] == 0) {
			// Child process
			if (i > 0) {
				if (dup2(prev_pipe_read, 0) == -1) exit(1);
				close(prev_pipe_read);
			}
			if (i < num_cmds - 1) {
				if (dup2(curr_pipe[1], 1) == -1) exit(1);
				close(curr_pipe[1]);
			}
			// Close potential inherited FDs from previous pipes
			if (i < num_cmds - 1) close(curr_pipe[0]);
			
			// Validate command exists and has at least one argument
			if (cmds[i] == NULL || cmds[i][0] == NULL) {
				exit(1);
			}
			
			// Execute command
			execvp(cmds[i][0], cmds[i]);
			exit(1);  // Only reach here if execvp fails
		}
		else {
			// Parent process
			if (i > 0) close(prev_pipe_read);
			if (i < num_cmds - 1) {
				close(curr_pipe[1]);
				prev_pipe_read = curr_pipe[0];
			}
		}
	}
	
	int ret = 0;
	int status;
	for (int i = 0; i < num_cmds; i++)
	{
		if (waitpid(pids[i], &status, 0) == -1)
			return 1;
		if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status))
			ret = 1;
	}
	
	free(pids);
	return ret;
}

int main(int argc, char **argv)
{
	int cmd_size = 1;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "|"))
			cmd_size++;
	}

	char ***cmds = calloc(cmd_size + 1, sizeof(char ***));
	cmds[0] = argv + 1;
	int cmds_i = 1;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "|"))
		{
			cmds[cmds_i] = argv + i + 1;
			argv[i] = NULL;
			cmds_i++;
		}
	}
	int ret = picoshell(cmds);
	free(cmds);
	return ret;
}
