#include "argo.h"

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

	// Expect opening quote
	if (getc(stream) != '"') 
	{
		// Put back the character we read if it wasn't a quote
		// Note: This part is tricky, ideally expect() should be used before calling get_str
		// For simplicity here, assume caller ensured the first char is '"'
		// Or, modify the caller logic. Let's assume the call is correct for now.
		// If the first character wasn't '"', it's an error state.
		free(res);
		// We don't have the unexpected char, so can't report it easily here.
		// Return NULL, the caller should handle the error.
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
			switch(c) {
				case '\\': // Fallthrough
				case '"':  // Fallthrough
				case '/':  // Keep c as is ('\\', '"', '/')
					break;
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				// case 'u': // \uXXXX handling is more complex, skip for now
				//     break;
				default: // Invalid escape sequence
					fprintf(stderr, "Error: Invalid escape sequence '\\%c'\n", c);
					free(res);
					// We consumed the invalid char, so unexpected() might not see it
					return NULL;
			}
		}
		// Resize buffer if needed
		if (i >= capacity - 1)
		{
			capacity *= 2;
			// Check for potential overflow if capacity becomes huge
			if (capacity < i + 1) {
				fprintf(stderr, "Error: String too large, capacity overflow.\n");
				free(res);
				return NULL;
			}
			char *new_res = realloc(res, capacity);
			if (!new_res)
			{
				free(res);
				perror("realloc failed in get_str");
				return NULL;
			}
			res = new_res;
		}
		res[i++] = (char)c; // Store the character (original or escaped)
	}
	res[i] = '\0'; // Null-terminate the string
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

	// Handle empty map {}
	if (accept(stream, '}'))
		return 1;

	while (1)
	{
		// Allocate space for the new pair *before* parsing key/value
		pair *new_data = realloc(dst->map.data, (dst->map.size + 1) * sizeof(pair));
		if (!new_data)
		{
			// realloc failed, cleanup already parsed pairs if any
			// The current dst->map.data is still valid if realloc failed
			free_json(*dst); // Use free_json to clean up what we have
			dst->map.data = NULL; // Ensure data pointer is NULL after freeing
			dst->map.size = 0;
			perror("realloc failed in parse_map");
			return -1;
		}
		dst->map.data = new_data;

		// Get pointer to the potential new pair slot
		pair *current = &dst->map.data[dst->map.size];
		// Initialize fields to safe values
		current->key = NULL;
		current->value = (json){ .type = INTEGER, .integer = 0 }; // Default init

		// 1. Parse Key (must be a string)
		if (peek(stream) != '"')
		{
			unexpected(stream);
			// No key was allocated yet, just return
			// Note: The realloc happened, but we haven't increased size.
			// free_json called by caller will handle existing pairs.
			return -1;
		}
		current->key = get_str(stream);
		if (current->key == NULL)
		{
			// get_str failed (might have printed error)
			// No need to free current->key as it's NULL
			return -1; // Let caller handle cleanup via free_json
		}

		// 2. Parse Separator ':'
		if (!expect(stream, ':'))
		{
			// Failed to find ':', need to free the key we just allocated
			free(current->key);
			current->key = NULL; // Prevent double free by caller's free_json
			return -1; // Let caller handle cleanup
		}

		// 3. Parse Value
		if (argo(&current->value, stream) == -1)
		{
			// Failed to parse value, need to free the key
			free(current->key);
			current->key = NULL;
			// The current->value might be partially filled, free_json in caller might fail
			// It's safer if argo ensures dst is cleaned/reset on failure. Assume it does for now.
			return -1; // Let caller handle cleanup
		}

		// Key-Value pair successfully parsed, *now* increment size
		dst->map.size++;

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
			// Continue loop for the next pair
		}
		else
		{
			// Unexpected character after value
			unexpected(stream);
			// Need to cleanup the last successfully parsed pair (key+value)
			// This is tricky because free_json expects a full map.
			// Instead of cleaning here, rely on caller calling free_json.
			// The state dst has size incremented, includes the last pair.
			return -1;
		}
	}
	return 1; // Map parsed successfully
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
		if (!(dst->string = get_str(stream))) return -1;
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

int argo(json *dst, FILE *stream) // this
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