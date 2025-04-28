#include "argo.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

int peek(FILE *stream)
{
	if (!stream)
		return EOF;
	int c = getc(stream);
	if (c != EOF)
		ungetc(c, stream);
	return c;
}

void unexpected(FILE *stream) // this
{
	if (!stream)
	{
		printf("unexpected null stream\n");
		return;
	}
	int c = peek(stream);
	if (c != EOF)
		printf("unexpected token '%c'\n", c);
	else
		printf("unexpected end of input\n");
}

int accept(FILE *stream, char c)
{
	if (!stream) // this
		return 0;
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int expect(FILE *stream, char c)
{
	if (!stream) // this
	{
		unexpected(stream);
		return 0;
	}
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

int parse_int(json *dst, FILE *stream) // this
{
	if (!dst || !stream)
		return -1;
	int n = 0;
	int c = peek(stream);
	
	if (!isdigit(c)) 
	{
		unexpected(stream);
		return -1;
	}
	
	c = peek(stream);
	while (isdigit(c)) 
	{
    	getc(stream);
    	n = n * 10 + (c - '0');
    	c = peek(stream);
	}
	
	dst->type = INTEGER;
	dst->integer = n;
	return 1;
}

char *get_str(FILE *stream) // this
{
	if (!stream)
		return NULL;

	size_t capacity = 16;
	size_t i = 0;
	char *res = malloc(capacity);
	if (!res)
		return NULL;

	if (getc(stream) != '"') 
	{
		free(res);
		return NULL;
	}

	while (1) 
	{
		int c = getc(stream);

		if (c == '"')
			break;

		if (c == EOF)
		{
			free(res);
			unexpected(stream);
			return NULL;
		}

		if (c == '\\')
		{
			c = getc(stream);

			if (c == EOF)
			{
				free(res);
				unexpected(stream);
				return NULL;
			}

			switch(c) {
				case '\\':
				case '"':
				case '/':
					break;
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				default:
					printf("Error: Invalid escape sequence '\\%c'\n", c);
					free(res);
					return NULL;
			}
		}

		if (i >= capacity - 1)
		{
			capacity *= 2;

			if (capacity < i + 1) 
			{
				printf("Error: String too large, capacity overflow.\n");
				free(res);
				return NULL;
			}

			char *new_res = realloc(res, capacity);

			if (!new_res)
			{
				free(res);
				printf("realloc failed in get_str\n");
				return NULL;
			}

			res = new_res;
		}
		res[i++] = (char)c;
	}
	res[i] = '\0';
	return res;
}

int parse_map(json *dst, FILE *stream) // this
{
	if (!dst || !stream)
		return -1;

	dst->type = MAP;
	dst->map.size = 0;
	dst->map.data = NULL;

	if (!expect(stream, '{'))
		return -1;

	if (accept(stream, '}'))
		return 1;

	while (1)
	{
		pair *new_data = realloc(dst->map.data, (dst->map.size + 1) * sizeof(pair));

		if (!new_data)
		{
			free_json(*dst);
			dst->map.data = NULL;
			dst->map.size = 0;
			printf("realloc failed in parse_map");
			return -1;
		}
		dst->map.data = new_data;

		pair *current = &dst->map.data[dst->map.size];

		current->key = NULL;
		current->value.type = INTEGER;
		current->value.integer = 0;

		if (peek(stream) != '"')
		{
			unexpected(stream);
			return -1;
		}

		current->key = get_str(stream);
		if (current->key == NULL)
			return -1;

		if (!expect(stream, ':'))
		{
			free(current->key);
			current->key = NULL;
			return -1;
		}

		if (argo(&current->value, stream) == -1)
		{
			free(current->key);
			current->key = NULL;
			return -1;
		}

		dst->map.size++;

		int c = peek(stream);
		if (c == '}')
        {
            getc(stream);
            break;
        }
        else if (c == ',')
            getc(stream);
		else
		{
			unexpected(stream);
			return -1;
		}
	}
	return 1;
}

int parser(json *dst, FILE *stream) // this
{
	if (!dst || !stream)
		return -1;

	int c = peek(stream);
	if (c == EOF)
	{
		unexpected(stream);
		return -1;
	}

	if (isdigit(c))
		return (parse_int(dst, stream));
	else if (c == '"')
	{
		dst->type = STRING;
		if (!(dst->string = get_str(stream))) 
			return -1;
		return 1;
	}
	else if (c == '{')
		return (parse_map(dst, stream));
	else
	{
		unexpected(stream);
		return -1;
	}
	return 1;
}

int argo(json *dst, FILE *stream) // this
{
	if (!stream || !dst)
		return -1;

	*dst = (json){ .type = INTEGER, .integer = 0 };
	if (parser(dst, stream) == -1)
		return -1;
	return 1;
}