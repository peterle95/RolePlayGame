#include <stdio.h>
#include <stdlib.h>

int sum_check(int x, int *subset, int len) 
{
    int sum = 0;
    for (int i = 0; i < len; i++) 
    {
        sum += subset[i];
    }
    return sum == x;
}

void print_sol(int *subset, int len) 
{
    for (int i = 0; i < len; i++) 
    {
        fprintf(stdout, "%d", subset[i]);
        if (i < len - 1) 
        {
            fprintf(stdout, " ");
        }
    }
    fprintf(stdout, "\n");
}

void gen_powerset(int x, int *set, int set_size) 
{
    if (set_size == 0) 
    {
        if (x == 0) fprintf(stdout, "\n");
        return;
    }

    int *subset = malloc(set_size * sizeof(int));
    if (!subset) exit(1);
    
    unsigned int powerset_size = 1 << set_size;
    for (unsigned int counter = 0; counter < powerset_size; counter++) 
    {
        int len = 0;
        for (int j = 0; j < set_size; j++) 
        {
            if (counter & (1 << j)) 
            {
                subset[len++] = set[j];
            }
        }
        if (sum_check(x, subset, len)) 
        {
            print_sol(subset, len);
        }
    }
    free(subset);
}

int main(int argc, char **argv) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s n [elements...]\n", argv[0]);
        return 1;
    }

    int x = atoi(argv[1]);
    int set_size = argc - 2;
    int *set = NULL;

    if (set_size > 0) 
    {
        set = malloc(set_size * sizeof(int));
        if (!set) exit(1);
        for (int i = 0; i < set_size; i++) 
        {
            set[i] = atoi(argv[i + 2]);
        }
    }

    gen_powerset(x, set, set_size);
    free(set);
    return 0;
}
