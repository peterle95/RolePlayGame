//This file is given at the exam

#include <stdio.h>
#include <malloc.h>
#include <ctype.h>

typedef struct node {
    enum {
        ADD,
        MULTI,
        VAL
    }   type;
    int val;
    struct node *l;
    struct node *r;
}   node;

node    *new_node(node n)
{
    node *ret = calloc(1, sizeof(n));
    if (!ret)
        return (NULL);
    *ret = n;
    return (ret);
}

void    destroy_tree(node *n)
{
    if (!n)
        return ;
    if (n->type != VAL)
    {
        destroy_tree(n->l);
        destroy_tree(n->r);
    }
    free(n);
}

void    unexpected(char c)
{
    if (c)
        printf("Unexpected token '%c'\n", c);
    else
        printf("Unexpected end of file\n");
}

// Forward declarations
node *parse_factor(char **s); // this
node *parse_expr(char **s);

int accept(char **s, char c)
{
    if (**s == c) // this
    {
        (*s)++;
        return (1);
    }
    return (0);
}

int expect(char **s, char c)
{
    if (accept(s, c))
        return (1);
    unexpected(**s);
    return (0);
}

node *parse_term(char **s) // this
{
    node *ret = parse_factor(s);
    
    if (!ret)
        return NULL;
    
    while (**s == '*')
    {
        (*s)++;
        node *right = parse_factor(s);
        
        if (!right)
        {
            destroy_tree(ret);
            return NULL;
        }
        
        node multi_node = {
            .type = MULTI,
            .l = ret,
            .r = right
        };
        
        ret = new_node(multi_node);
        
        if (!ret)
        {
            destroy_tree(right);
            return NULL;
        }
    }
    
    return ret;
}

node *parse_factor(char **s) // this
{
    if (**s == '(')
    {
        (*s)++;
        node *ret = parse_expr(s);  
              
        if (!ret)
            return NULL;
        
        if (!expect(s, ')'))
        {
            destroy_tree(ret);
            return NULL;
        }
        
        return ret;
    }
    
    if (isdigit(**s))
    {
        node val_node = {
            .type = VAL,
            .val = **s - '0'
        };
        
        (*s)++;
        return new_node(val_node);
    }
    
    unexpected(**s);
    return NULL;
}

node *parse_expr(char **s) // this
{
    node *ret = parse_term(s); 
    
    if (!ret)
        return NULL;
    
    while (**s == '+')
    {
        (*s)++;
        node *right = parse_term(s);
        
        if (!right)
        {
            destroy_tree(ret);
            return NULL;
        }
        
        node add_node = {
            .type = ADD,
            .l = ret,
            .r = right
        };
        
        ret = new_node(add_node);
        
        if (!ret)
        {
            destroy_tree(right);
            return NULL;
        }
    }
        return ret;
}

int eval_tree(node *tree)
{
    switch (tree->type)
    {
        case ADD:
            return (eval_tree(tree->l) + eval_tree(tree->r));
        case MULTI:
            return (eval_tree(tree->l) * eval_tree(tree->r));
        case VAL:
            return (tree->val);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return (1);
    
    char *expr_str = argv[1]; // this
    node *tree = parse_expr(&expr_str); 
    
    if (!tree)
        return (1);
        
    if (*expr_str != '\0') // this
    {
        unexpected(*expr_str);
        destroy_tree(tree);
        return (1);
    }
    
    printf("%d\n", eval_tree(tree));
    destroy_tree(tree);
    return (0); // this
}