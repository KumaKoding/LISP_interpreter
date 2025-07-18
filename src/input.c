#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"

#define IGNORE_CHARACTERS "\n\t\r"

int count_copies(char *raw, int index, int max_len)
{
	char target = raw[index];

	int ctr = 0;

	while(raw[index + ctr + 1] == target && index + ctr + 1 <= max_len)
	{
		ctr++;
	}

	return ctr;
}

int is_in(char c, const char * const set)
{
	for(int i = 0; i < strlen(set); i++)
	{
		if(c == set[i])
		{
			return 1;
		}
	}

	return 0;
}

int count_duplicate_formatting(char *raw, int len, const char *const formatting)
{
	int subtract_len = 0;
	int c = 0;

	while(c < len)
	{
		if(is_in(raw[c], formatting))
		{
			int duplicates = count_copies(raw, c, len);

			subtract_len += duplicates;
			c += duplicates;
		}

		c++;
	}

	return subtract_len;
}

struct safe_string cleanse_formatting(char *raw, int len)
{
	struct safe_string clean_string;

	clean_string.len = len - count_duplicate_formatting(raw, len, IGNORE_CHARACTERS);
	clean_string.data = malloc(sizeof(char) * clean_string.len); 

	int raw_c = 0;
	int new_c = 0;

	while(new_c < clean_string.len)
	{
		if(is_in(raw[raw_c], IGNORE_CHARACTERS))
		{
			clean_string.data[new_c] = ' ';
			raw_c += count_copies(raw, raw_c, len);
		}
		else 
		{
			clean_string.data[new_c] = raw[raw_c];
		}

		raw_c++;
		new_c++;
	}

	return clean_string;
}

int within_bounds(int check, int a, int b)
{
	if(check >= a && check <= b)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

int is_legal_identifier(char c)
{
	if(within_bounds(c, 'a', 'z'))
	{
		return 1;
	}

	if(within_bounds(c, 'A', 'Z'))
	{
		return 1;
	}

	if(is_in(c, EXTENDED_IDENTIFIER_CHARACTERS))
	{
		return 1;
	}

	return 0;
}
