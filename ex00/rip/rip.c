#include <unistd.h>
#include <stdlib.h>

void clean_output(char *str)
{
    int i = 0;
    int j = 0;
    
    // Skip leading spaces
    while (str[i] == ' ')
        i++;
        
    // Copy non-space characters and compress multiple spaces into one
    while (str[i])
    {
        if (str[i] != ' ' || (i > 0 && str[i-1] != ' '))
            str[j++] = str[i];
        i++;
    }
    
    // Remove trailing space if exists
    if (j > 0 && str[j-1] == ' ')
        j--;
        
    str[j] = '\0';
}

void    ft_putstr(char *str)
{
    while (*str)
        write(1, str++, 1);
}

int is_valid(char *str)
{
    int count = 0;
    
    while (*str)
    {
        if (*str == '(')
            count++;
        else if (*str == ')')
            count--;
        if (count < 0)
            return 0;
        str++;
    }
    return count == 0;
}

void try_remove(char *str, int pos, int to_remove, char *result)
{
    if (to_remove == 0)
    {
        if (is_valid(result))
        {
            clean_output(result);
            ft_putstr(result);
            write(1, "\n", 1);
        }
        return;
    }
    
    while (str[pos])
    {
        if (str[pos] == '(' || str[pos] == ')')
        {
            char temp = result[pos];
            result[pos] = ' ';
            try_remove(str, pos + 1, to_remove - 1, result);
            result[pos] = temp;
            
            // Skip duplicates
            while (str[pos + 1] && str[pos] == str[pos + 1])
                pos++;
        }
        pos++;
    }
}

int count_unbalanced(char *str)
{
    int open = 0;
    int close = 0;
    int count = 0;
    
    while (*str)
    {
        if (*str == '(')
            open++;
        else if (*str == ')')
        {
            if (open == 0)
                close++;
            else
                open--;
        }
        str++;
    }
    return open + close;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        write(1, "\n", 1);
        return 0;
    }

    char *str = argv[1];
    int len = 0;
    while (str[len])
        len++;
        
    char *result = malloc(len + 1);
    if (!result)
        return 1;
        
    for (int i = 0; i <= len; i++)
        result[i] = str[i];
        
    int min_remove = count_unbalanced(str);
    if (min_remove == 0)
    {
        ft_putstr(str);
        write(1, "\n", 1);
    }
    else if (min_remove > len)
        write(1, "\n", 1);
    else
    {
        try_remove(str, 0, min_remove, result);
    }
    
    free(result);
    return 0;
}
