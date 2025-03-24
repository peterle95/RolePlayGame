#ifndef VBC_H
# define VBC_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct node {
    char type;
    int val;
    struct node *l;
    struct node *r;
}   node;

void    unexpected(char c);
node    *parse_expr(char *s);
void    free_list(node *list);
void    op_plus_etoile(node **list);
void    parenthesis(node **list);

#endif