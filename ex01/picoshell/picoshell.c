#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void close_unused_fds(int keep_stdin, int keep_stdout) 
{
	for (int fd = 0; fd < 1024; fd++) {
		if ((fd == 0 && keep_stdin) || (fd == 1 && keep_stdout) || fd == 2)
			continue;
		close(fd);
	}
}

int picoshell(char **cmds[])
{
	int num_cmds = 0;
	while (cmds[num_cmds] != NULL)
	{
		if (cmds[num_cmds][0] == NULL || cmds[num_cmds][0][0] == '\0')
			return 1;
		num_cmds++;
	}
	
	pid_t *pids = malloc(sizeof(pid_t) * num_cmds);
	if (!pids) 
		return 1;
	
	int prev_pipe_read = -1;
	
	for (int i = 0; i < num_cmds; i++)
	{
		int curr_pipe[2];
		if (i < num_cmds - 1) 
		{
			if (pipe(curr_pipe) == -1) 
			{
				close(prev_pipe_read);
				// Wait for any existing children before failing
				for (int j = 0; j < i; j++) {
					waitpid(pids[j], NULL, 0);
				}
				free(pids);
				return 1;
			}
		}
	
		pids[i] = fork();
		if (pids[i] == -1) 
		{
			close(prev_pipe_read);
			if (i < num_cmds - 1) 
			{
				close(curr_pipe[0]);
				close(curr_pipe[1]);
			}
			for (int j = 0; j < i; j++) 
				waitpid(pids[j], NULL, 0);
			free(pids);
			return 1;
		}
		if (pids[i] == 0) 
		{
			if (i > 0) {
				if (dup2(prev_pipe_read, 0) == -1)
					exit(1);
				close(prev_pipe_read);
			}
			if (i < num_cmds - 1) 
			{
				if (dup2(curr_pipe[1], 1) == -1)
					exit(1);
				close(curr_pipe[1]);
			}
			if (i < num_cmds - 1)
				close(curr_pipe[0]);
			
			close_unused_fds((i == 0), (i == num_cmds - 1));
			
			execvp(cmds[i][0], cmds[i]);
			exit(1);
		}
		else 
		{
			if (i > 0)
				close(prev_pipe_read);
			if (i < num_cmds - 1) {
				close(curr_pipe[1]);
				prev_pipe_read = curr_pipe[0];
			}
		}
	}

	int final_status = 0;
	int status;
	/*
	for (int i = 0; i < num_cmds; i++)
	{
		if (waitpid(pids[i], &status, 0) == -1)
			final_status = 1;  // Mark error but continue waiting for others
		else if (WIFEXITED(status))
		{
			int exit_status = WEXITSTATUS(status);
			final_status |= exit_status != 0;
		}
		else 
			final_status = 1;
	}
	*/
  	for (int i = 0; i < num_cmds; i++)
	{
		if (waitpid(pids[i], &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
			final_status = 1;
	}
	
	free(pids);
	return final_status;
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
