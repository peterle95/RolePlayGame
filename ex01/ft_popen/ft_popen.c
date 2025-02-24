#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // For execvp prototype

/*
** ft_popen:
** - For type 'r': creates a pipe where the child process's STDOUT is connected to the pipe.
** - For type 'w': creates a pipe where the child process's STDIN is connected to the pipe.
*/
int ft_popen(const char *file, const char *argv[], char type)
{
    int fd[2];
    int pid;

    if (type != 'r' && type != 'w')
        return (-1);
    if (pipe(fd) == -1)
        return (-1);
    pid = fork();
    if (pid == -1)
    {
        close(fd[0]);
        close(fd[1]);
        return (-1);
    }
    if (pid == 0)
    {
        if (type == 'r')
        {
            // Child: redirect STDOUT to pipe write end.
            if (dup2(fd[1], STDOUT_FILENO) == -1)
                exit(-1);
        }
        else  /* type == 'w' */
        {
            // Child: redirect STDIN to pipe read end.
            if (dup2(fd[0], STDIN_FILENO) == -1)
                exit(-1);
        }
        // Close both ends in the child as they're no longer needed.
        close(fd[0]);
        close(fd[1]);
        // Execute the file.
        if (execvp(file, (char * const *)argv) == -1)
            exit(-1);
    }
    // Parent process: close the unneeded end and return the appropriate descriptor.
    if (type == 'r')
    {
        close(fd[1]);
        return (fd[0]);
    }
    else  /* type == 'w' */
    {
        close(fd[0]);
        return (fd[1]);
    }
}

/*
** ft_putstr:
** A simple function to output a string to STDOUT.
*/
void ft_putstr(const char *s)
{
    size_t i = 0;
    if (!s)
        return;
    while (s[i])
        i++;
    write(1, s, i);
}

/*
** get_next_line:
** A very basic implementation that reads one character at a time from the given file descriptor.
** It stops when it reaches a newline ('\n') or EOF.
*/
char *get_next_line(int fd)
{
    int capacity = 128;
    char *line = malloc(capacity);
    if (!line)
        return NULL;
    int i = 0;
    char c;
    int bytes;

    while ((bytes = read(fd, &c, 1)) > 0)
    {
        line[i++] = c;
        if (c == '\n')
            break;
        if (i == capacity)
        {
            capacity *= 2;
            char *tmp = realloc(line, capacity);
            if (!tmp)
            {
                free(line);
                return NULL;
            }
            line = tmp;
        }
    }
    if (bytes < 0 || (bytes == 0 && i == 0))
    {
        free(line);
        return NULL;
    }
    line[i] = '\0';
    return line;
}

/*
** main:
** Demonstrates the use of ft_popen by executing "ls" and printing its output line by line.
*/
/* int main(void)
{
    int fd = ft_popen("pwd", (const char *[]){"pwd", NULL}, 'r');
    char *line;

    if (fd == -1)
    {
        ft_putstr("ft_popen failed\n");
        return (1);
    }
    while ((line = get_next_line(fd)))
    {
        ft_putstr(line);
        free(line);
    }
    return 0;
} */

int main()
{
    int fd = ft_popen("ls", (const char *[]) {"ls", NULL}, 'r');
    char *line;
    while ((line = get_next_line(fd)))
        ft_putstr(line);
}
