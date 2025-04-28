#include "argo.h"
#include <stdio.h> // For printf, NULL, EOF
#include <stdlib.h> // For malloc, realloc, free, size_t
#include <string.h> // For strcmp (if allowed)
#include <ctype.h> // For isdigit, isspace (if allowed)
#include <stdint.h>

int peek(FILE *stream)
{
	if (!stream)
		return EOF;
	int c = getc(stream);
	if (c != EOF)
		ungetc(c, stream);
	return c;
}

void unexpected(FILE *stream)
{
	if (!stream)
	{
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
	if (!stream)
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
	if (!stream)
	{
		unexpected(stream);
		return 0;
	}
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

int parse_int(json *dst, FILE *stream)
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

char *get_str(FILE *stream)
{
	if (!stream)
		return NULL;
	size_t capacity = 16;
	size_t i = 0;
	char *res = malloc(capacity);
	if (!res)
	{
		return NULL;
	}

	while (1) {
		int c = getc(stream);
		if (c == '"') // End of string
			break;
		if (c == EOF)
		{
			free(res);
			unexpected(stream); // Report unexpected EOF
			return NULL;
		}
		if (c == '\\') // Handle escape sequences
		{
			c = getc(stream);
			if (c == EOF)
			{
				free(res);
				unexpected(stream);
				return NULL;
			}
			if (c != '\\' && c != '"')
			{
				free(res);
				ungetc(c, stream);
				unexpected(stream);
				return NULL;
			}
		}

		// Resize buffer if needed
		if (i >= capacity - 1)
		{
			if (capacity > SIZE_MAX / 2) {
				free(res);
				return NULL;
			}
			capacity *= 2;
			char *new_res = realloc(res, capacity);
			if (!new_res)
			{
				free(res);
				return NULL;
			}
			res = new_res;
		}
		res[i++] = (char)c; // Store the character (original or escaped)
	}
	res[i] = '\0'; // Null-terminate the string
	return res;
}

int parse_map(json *dst, FILE *stream)
{
	if (!dst || !stream)
		return -1;

	dst->type = MAP;
	dst->map.size = 0;
	dst->map.data = NULL;

	if (!expect(stream, '{'))
		return -1;

	// Handle empty map {}
	if (accept(stream, '}'))
		return 1;

	while (1)
	{
		// Store old state in case realloc fails
		pair *old_data = dst->map.data;
		size_t old_size = dst->map.size;

		// Allocate space for the new pair *before* parsing key/value
		pair *new_data = realloc(old_data, (old_size + 1) * sizeof(pair));
		if (!new_data)
		{
			// realloc failed. old_data is still valid.
			// Restore state for free_json
			dst->map.data = old_data;
			dst->map.size = old_size;
			free_json(*dst); // Free the previously allocated valid pairs
			// Reset dst to safe state after freeing
			dst->map.data = NULL;
			dst->map.size = 0;
			return -1;
		}
		dst->map.data = new_data;

		// Get pointer to the new pair slot (index is the old size)
		pair *current = &dst->map.data[old_size];
		// Initialize fields to safe values
		current->key = NULL;
		current->value = (json){ .type = INTEGER, .integer = 0 }; // Default init

		// 1. Parse Key (must be a string)
		if (!expect(stream, '"')) {
			dst->map.size = old_size; // Keep size consistent for free_json
			return -1;
		}
		current->key = get_str(stream);
		if (current->key == NULL)
		{
			return -1;
		}

		// 2. Parse Separator ':'
		if (!expect(stream, ':'))
		{
			// Failed to find ':', need to free the key we just allocated
			free(current->key);
			current->key = NULL;
			return -1;
		}

		// 3. Parse Value
		if (argo(&current->value, stream) == -1)
		{
			// Failed to parse value, need to free the key
			free(current->key);
			current->key = NULL;
			return -1;
		}

		// Key-Value pair successfully parsed, *now* increment size
		dst->map.size++; // Use the actual new size

		// 4. Check for map end '}' or next element ','
		int c = peek(stream);
		if (c == '}')
		{
			(void)getc(stream); // Consume '}'
			break; // Map finished successfully
		}

		if (c == ',')
		{
			(void)getc(stream); // Consume ','
			if (peek(stream) == '}') {
				unexpected(stream); // Cannot have '}' immediately after ','
				return -1;
			}
		}
		else
		{
			// Unexpected character after value
			unexpected(stream);
			return -1;
		}
	}
	return 1; // Map parsed successfully
}

int parser(json *dst, FILE *stream)
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
		if (!expect(stream, '"')) {
			return -1;
		}
		dst->type = STRING;
		dst->string = get_str(stream);
		if (!dst->string) {
			return -1;
		}
		return 1;
	}
	else if (c == '{')
		return (parse_map(dst, stream));
	else
	{
		unexpected(stream);
		return -1;
	}
	return (1);
}

int argo(json *dst, FILE *stream)
{
	if (!stream || !dst)
		return -1;
	// Initialize dst to a safe default state before parsing
	*dst = (json){ .type = INTEGER, .integer = 0 };
	if (parser(dst, stream) == -1)
	{
		// If parser fails, it might have partially filled dst.
		// free_json might be needed, but let the caller handle it.
		return -1;
	}
	return 1;
}