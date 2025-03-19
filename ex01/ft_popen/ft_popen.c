#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // For execvp prototype

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
            if (dup2(fd[1], STDOUT_FILENO) == -1)
                exit(-1);
        }
        else
        {
            if (dup2(fd[0], STDIN_FILENO) == -1)
                exit(-1);
        }
        close(fd[0]);
        close(fd[1]);
        if (execvp(file, (char * const *)argv) == -1)
            exit(-1);
    }
    if (type == 'r')
    {
        close(fd[1]);
        return (fd[0]);
    }
    else
    {
        close(fd[0]);
        return (fd[1]);
    }
}

void ft_putstr(const char *s)
{
    size_t i = 0;
    if (!s)
        return;
    while (s[i])
        i++;
    write(1, s, i);
}

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

int main()
{
    int fd = ft_popen("ls", (const char *[]) {"ls", NULL}, 'r');
    char *line;
    while ((line = get_next_line(fd)))
        ft_putstr(line);
}
