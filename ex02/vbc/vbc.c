/*This problem is similar to leetcode224.
-----------------------------------------

Each character of argv[1] is contained in an element of a double-chained list.

The op_plus_star function calculates an expression up to the end of the string or ')' by making two passes.
The first calculates multiplication; when '*' is found, the result between the number on the left and the number on the right is stored in the element on the left.
The sign and the right element are removed from the list. The list is chained again.
The second pass calculates the '+'.

parenthesis calculates parentheses;
Scanning the list from left to right, we find the first ')'. Then scroll from right to left to find '('.
We retrieve the result of the expression between these parentheses with op_plus_etoile. The result is stored in the element to the right of '('.
The '(' element and the element to the right of it are deleted.

We return to the beginning. do it again, until no parentheses remain. op_plus_etoile calculates the final result.
---------------------------------------------

The logic of this code is simple(?) but time-consuming. It consumes a lot of resources and time, and is likely to timeout at the exam lol!
But it has been accepted at leetcode

Translated with DeepL.com (free version)*/

#include "vbc.h"

void print_list(node *list)
{
	while (list)
	{
		if (list->type == 'n')
			printf("%d ", list->val);
		else
			printf("%c ", list->type);
		list = list->r;
	}
	printf("\n");
}

void delete_node(node **list, node *to_del)
{
	if (!*list || !to_del)
		return ;

	node *prev = to_del->l;
	node *next = to_del->r;

	if (prev)
		prev->r = next;
	else
		*list = next;
	if (next)
		next->l = prev;
	
	free(to_del);
}

void op_plus_etoile(node **list)//calcul pour une expression sans parenthese
{
	node *scan = *list;
	node *reset = NULL;
	node *next = NULL;

	while (scan && scan->type != ')')
	{
		if (scan->type == '*')
		{
			int res = scan->l->val * scan->r->val;
			reset = scan->l;
			reset->val = res;
			reset->type = 'n';
			next = scan->r->r;//peut etre NULL

			delete_node(list, scan->r);
			delete_node(list, scan);
			scan = next;
		}
		else
			scan = scan->r;
	}
	scan = *list;
	while (scan && scan->type != ')')
	{
		if (scan->type == '+')
		{
			int res = scan->l->val + scan->r->val;
			reset = scan->l;
			reset->val = res;
			reset->type = 'n';
			next = scan->r->r;

			delete_node(list, scan->r);
			delete_node(list, scan);
			scan = next;
		}
		else
			scan = scan->r;
	}
}

void parenthesis(node **list)
{
	node *scan = *list;

	while (scan)
	{
		if (scan->type == ')')
		{
			while (scan && scan->type != '(')
				scan = scan->l;
			op_plus_etoile(&(scan->r));
			delete_node(list, scan->r->r);
			delete_node(list, scan);
			scan = *list;
		}
		else
			scan = scan->r;
	}
}

node *parse_expr(char *s)
{
	node *head = NULL;
	node *prev = NULL;
	
	while (*s) {
		node *new_node = malloc(sizeof(node));
		if (!new_node) exit(1);
		
		if (isdigit(*s)) {
			new_node->type = 'n';
			new_node->val = *s - '0';
		} else if (*s == '+' || *s == '*' || *s == '(' || *s == ')') {
			new_node->type = *s;
			new_node->val = 0;
		} else {
			unexpected(*s);
		}
		
		new_node->l = prev;
		new_node->r = NULL;
		
		if (prev)
			prev->r = new_node;
		else
			head = new_node;
		prev = new_node;
		s++;
	}
	return head;
}

void free_list(node *list)
{
	while (list) {
		node *next = list->r;
		free(list);
		list = next;
	}
}

void unexpected(char c)
{
	if (c == '\0')
		printf("Unexpected end of input\n");
	else
		printf("Unexpected token '%c'\n", c);
	exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return (1);
    node *list = parse_expr(argv[1]);
    if (!list)
        return (1);
    
    parenthesis(&list);
    op_plus_etoile(&list);
    print_list(list);
    free_list(list);
    return (0);
}