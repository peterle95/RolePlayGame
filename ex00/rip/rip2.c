#include <unistd.h>

int is_balanced(char *str) 
{
    int count = 0;
    for (int i = 0; str[i]; i++) 
    {
        if (str[i] == '(')
            count++;
        else if (str[i] == ')')
            count--;
        if (count < 0)
            return 0;
    }
    return count == 0;
}

void print_solution(char *str) 
{
    while (*str) write(1, str++, 1);
    write(1, " \n", 2);
}

void remove_parentheses(char *str, int pos, char *current, int removed) 
{
    if (str[pos] == '\0') 
    {
        if (is_balanced(current))
            print_solution(current);
        return;
    }
    current[pos] = str[pos];
    remove_parentheses(str, pos + 1, current, removed);
    if (removed > 0 && (str[pos] == '(' || str[pos] == ')')) 
    {
        current[pos] = ' ';
        remove_parentheses(str, pos + 1, current, removed - 1);
    }
}

int count_unbalanced(char *str) 
{
    int open = 0;
    int unbalanced = 0;
    
    for (int i = 0; str[i]; i++) 
    {
        if (str[i] == '(')
            open++;
        else if (str[i] == ')') 
        {
            if (open == 0)
                unbalanced++;
            else
                open--;
        }
    }
    return unbalanced + open;
}

int main(int argc, char **argv) 
{
    if (argc != 2) {
        write(1, "\n", 1);
        return 1;
    }

    char *input = argv[1];
    int len = 0;
    int has_open = 0;
    int has_close = 0;
    while (input[len] != '\0') // or argv[1][len]
    {
        if (input[len] != '(' && input[len] != ')') // or argv[1][len]
        {
            write(1, "\n", 1);
            return 1;
        }
        if (input[len] == '(') // or argv[1][len]
            has_open = 1;
        if (input[len] == ')') // or argv[1][len]
            has_close = 1;
        len++;
    }
    
    if (!has_open || !has_close)
        return (write(1, "\n", 1), 1);
        
    char current[len + 1];
    for (int i = 0; i <= len; i++)
        current[i] = input[i];
        
    int min_remove = count_unbalanced(input);
    if (!min_remove)
        return (print_solution(input), 0);
        
    remove_parentheses(input, 0, current, min_remove);
    return 0;
}
