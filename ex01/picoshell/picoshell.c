#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int  close_pipes(int pipes[][2], int num)
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

int picoshell(char **cmds[])
{
	int i = 0;
	int num_of_processes = 0;
	int pid;
	int pids[1024]; // Array to store child process IDs for ordered waiting

	while (cmds[num_of_processes])
		num_of_processes++;
	int pipes[num_of_processes - 1][2];
	while (i < num_of_processes - 1)
	{
		if (pipe(pipes[i]) == -1)
			return (1);
		i++;
	}
	i = 0;
	while (i < num_of_processes)
	{
		if ((pid = fork()) == -1)
			return (1);
		if (pid == 0)
		{
			// First set up redirections
			if (i > 0)
			{
				if (dup2(pipes[i - 1][0], 0) == -1)
					exit(1);
			}
			if (i < num_of_processes - 1)
			{
				if (dup2(pipes[i][1], 1) == -1)
					exit(1);
			}
			
			// THEN close all pipes after setting up redirections
			if (!close_pipes(pipes, num_of_processes - 1))
				exit(1);

			if (execvp(cmds[i][0], cmds[i]) == -1)
				exit (1);
		}
		pids[i] = pid; // Store child PID for ordered waiting
		i++;
	}
	if (!close_pipes(pipes, num_of_processes - 1))
		return (1);
	
	int ret = 0;
	int status;
	
	// Wait for child processes in order
	for (i = 0; i < num_of_processes; i++)
	{
		if (waitpid(pids[i], &status, 0) == -1)
			return (1);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			ret = 1;
		else if (!WIFEXITED(status))
			ret = 1;
	}
	return (ret);
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
