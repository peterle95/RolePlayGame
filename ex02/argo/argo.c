#include "argo.h"

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

int	parse_int(json *dst, FILE *stream)
{
	int n = 0;
	int c = peek(stream);
	
	if (!isdigit(c)) {
		unexpected(stream);
		return -1;
	}
	
	while (isdigit(c)) {
		getc(stream); // consume digit
		n = n * 10 + (c - '0');
		c = peek(stream);
	}
	
	dst->type = INTEGER;
	dst->integer = n;
	return 1;
}

char *get_str(FILE *stream)
{
	size_t capacity = 16;
	size_t i = 0;
	char *res = malloc(capacity);
	if (!res) return NULL;
	
	int c = getc(stream); // Consume opening quote
	if (c != '"') {
		free(res);
		return NULL;
	}

	while (1) {
		c = getc(stream);
		if (c == '"') break;
		if (c == EOF) {
			free(res);
			unexpected(stream);
			return NULL;
		}
		if (c == '\\') {
			c = getc(stream);
			if (c == EOF) {
				free(res);
				unexpected(stream);
				return NULL;
			}
		}
		if (i >= capacity-1) {
			capacity *= 2;
			char *new_res = realloc(res, capacity);
			if (!new_res) {
				free(res);
				return NULL;
			}
			res = new_res;
		}
		res[i++] = c;
	}
	res[i] = '\0';
	return res;
}

int parse_map(json *dst, FILE *stream)
{
	dst->type = MAP;
	dst->map.size = 0;
	dst->map.data = NULL;
	
	if (!expect(stream, '{')) return -1;

	char c = getc(stream);

	if (peek(stream) == '}')
		return 1;

	while (1)
	{
		if (peek(stream) != '"') {
			unexpected(stream);
			return -1;
		}
		pair *new_data = realloc(dst->map.data, (dst->map.size+1)*sizeof(pair));
		if (new_data == NULL) {
			return -1;
		}
		dst->map.data = new_data;
		pair *current = &dst->map.data[dst->map.size];
		current->key = get_str(stream);
		if (current->key == NULL)
			return -1;
		dst->map.size++;
		if (expect(stream, ':') == 0)
			return -1;
		if (argo(&current->value, stream) == -1)
			return -1;
		c = peek(stream);
		if (c == '}')
		{
			accept(stream ,c);
			break ;
		}
		if (c == ',')
			accept(stream, ',');
		else
		{
			unexpected(stream);
			return -1;
		}
	}
	return 1;
}

int parser(json *dst, FILE *stream)
{
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

//free_json() added in the main
int argo(json *dst, FILE *stream)
{
	if (parser(dst, stream) == -1)
		return -1;
	return 1;
}