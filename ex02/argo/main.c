#include "argo.h"
#include <errno.h> // For errno

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int	main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	char *filename = argv[1];
	FILE *stream;

	// Handle reading from stdin if filename is "-" or "/dev/stdin"
	// strcmp is safer than direct comparison
	if (strcmp(filename, "-") == 0 || strcmp(filename, "/dev/stdin") == 0) {
		stream = stdin;
	} else {
		stream = fopen(filename, "r");
		if (!stream) {
			perror("Error opening file"); // Print system error message
			fprintf(stderr, "Filename: %s\n", filename);
			return 1;
		}
	}

	// Initialize json struct to a known state
	json file = { .type = INTEGER, .integer = 0 }; // Default state

	if (argo(&file, stream) != 1)
	{
		// argo or parser should have printed an "unexpected token" error
		fprintf(stderr, "Error: Failed to parse JSON input.\n");
		free_json(file); // Attempt cleanup
		if (stream != stdin) fclose(stream);
		return 1;
	}

	// After successful parsing, check for trailing characters
	int c = peek(stream);
	// Allow trailing whitespace
	while (isspace(c)) {
		getc(stream); // Consume whitespace
		c = peek(stream);
	}

	if (c != EOF) {
		fprintf(stderr, "Error: Trailing characters found after JSON data: ");
		unexpected(stream); // Print the unexpected character
		free_json(file);
		if (stream != stdin) fclose(stream);
		return 1;
	}

	// Successfully parsed and no trailing chars, now serialize
	serialize(file);
	printf("\n"); // Add trailing newline as per examples

	// Cleanup
	free_json(file);
	if (stream != stdin) fclose(stream);

	return 0; // Indicate success
}