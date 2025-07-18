#ifndef INPUT_H
#define INPUT_H

#define EXTENDED_IDENTIFIER_CHARACTERS "!$%&*+-./:<=>?@^_~"

struct safe_string
{
	int len;
	char *data;
};

int count_copies(char *raw, int index, int max_len);
int is_in(char c, const char * const set);
int count_duplicate_formatting(char *raw, int len, const char *const formatting);
struct safe_string cleanse_formatting(char *raw, int len);
int within_bounds(int check, int a, int b);
int is_legal_identifier(char c);

#endif
