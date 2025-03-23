#include <unistd.h>
#include <stdlib.h>

// devi pushare solo questa funzione
int ft_popen(const char *file, const char *argv[], char type)
{
    int fd[2];
    int pid;

    if (!file || !argv || (type != 'r' && type != 'w'))
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

// SUBJECT MAIN
/* int main() 
{
    int fd = ft_popen("ls", (const char *[]) {"ls", NULL}, 'r');
    char *line;
    while ((line = get_next_line(fd)))
        ft_putstr(line);
} */

//IMPLEMENTATION WHERE FDs ARE NOT LEAKED
int main()
{
    int fd = ft_popen("ls", (const char *[]) {"ls", NULL}, 'r');
    if (fd == -1) {
        write(2, "Error in ft_popen\n", 18);
        return 1;
    }          
    char *line;
    while ((line = get_next_line(fd)))
    {
        ft_putstr(line);
        free(line);
    }
    close(fd);
    return 0;
}

//CHAIN COMMANDS
/* int main()
{
    int fd1 = ft_popen("ls", (const char *[]){"ls", NULL}, 'r');
    char *content = get_next_line(fd1);
    close(fd1);
    
    int fd2 = ft_popen("wc", (const char *[]){"wc", NULL}, 'w');
    write(fd2, content, strlen(content));
    close(fd2);
    free(content);
    return 0;
} */

//STREAM DATA TO ANOTHER PROGRAM
/* int main()
{
    // Write to a command
    int fd = ft_popen("grep hello", (const char *[]){"grep", "hello", NULL}, 'w');
    write(fd, "hello world\n", 12);
    close(fd);
    return 0;
} */