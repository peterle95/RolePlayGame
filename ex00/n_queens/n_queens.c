#include <unistd.h>
#include <stdlib.h>

// Function to print a number
void    ft_putnbr(int n)
{
    char c;
    
    if (n >= 10)
        ft_putnbr(n / 10);
    c = n % 10 + '0';
    write(1, &c, 1);
}

// Function to print the solution
void    print_solution(int *board, int n)
{
    int i = 0;
    
    while (i < n)
    {
        ft_putnbr(board[i]);
        if (i < n - 1)
            write(1, " ", 1);
        i++;
    }
    write(1, "\n", 1);
}

// Function to check if a queen can be placed on board[row][col]
int     is_safe(int *board, int row, int col, int n)
{
    int i;
    int j;
    
    // Check this row on left side
    for (i = 0; i < col; i++)
        if (board[i] == row)
            return (0);
            
    // Check upper diagonal on left side
    i = col - 1;
    j = row - 1;
    while (i >= 0 && j >= 0)
    {
        if (board[i] == j)
            return (0);
        i--;
        j--;
    }
            
    // Check lower diagonal on left side
    i = col - 1;
    j = row + 1;
    while (i >= 0 && j < n)
    {
        if (board[i] == j)
            return (0);
        i--;
        j++;
    }
            
    return (1);
}

// Recursive function to solve N Queens
void    solve_nqueens(int *board, int col, int n)
{
    int row;
    
    // Base case: If all queens are placed, print the solution
    if (col == n)
    {
        print_solution(board, n);
        return;
    }
    
    // Try placing queen in all rows of this column
    for (row = 0; row < n; row++)
    {
        if (is_safe(board, row, col, n))
        {
            board[col] = row;
            solve_nqueens(board, col + 1, n);
        }
    }
}

int     main(int argc, char **argv)
{
    int n;
    // int *board;
    int board[15];  // Static array with maximum reasonable size

    if (argc != 2)
        return (1);
        
    n = atoi(argv[1]);
    /* if (n <= 0)
        return (1);
        
    board = (int *)malloc(sizeof(int) * n);
    if (!board)
        return (1); */
    if (n <= 0 || n > 15)  // Add upper bound check
        return (1);

    solve_nqueens(board, 0, n);
    // free(board);
    return (0);
}
