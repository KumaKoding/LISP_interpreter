#include <stdio.h>
#include <stdlib.h>

#include "vector.h"
#include "lex.h"

#define true 1
#define false 0

typedef int bool;

#define ADD "add"
#define SUB "sub"
#define MUL "mul"
#define DIV "div"

bool match_keyword(vector *str, const char *keyword, int kw_size)
{
    if(str->len == kw_size - 1)
    {
        for(int i = 0; i < str->len; i++)
        {
            if(str->data[i] != keyword[i])
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

int destruct_tokens(struct token *head)
{
    if(head == NULL) { return -1; }

    struct token *old_node = head;
    struct token *new_node = NULL;

    do
    {
        new_node = old_node->next_token;

        free(old_node->str);
        free(old_node);

        old_node = new_node;
    } 
    while(old_node->next_token != NULL);

    return 0;
}

struct token* lex(vector *input)
{
    struct token *start = malloc(sizeof(struct token));
    start->type = Start;
    start->str = malloc(1);
    start->len = 1;

    start->next_token = malloc(sizeof(struct token));
    struct token *curr = start->next_token;

    for(int i = 0; i < input->len; i++) 
    {
        // printf("(%c, %d)", input->data[i], i);

        if(input->data[i] == '(')
        {
            curr->type = O_paren;

            curr->str = malloc(1);
            curr->str[0] = '(';

            curr->len = 1;
        }
        if(input->data[i] == ')')
        {
            curr->type = C_paren;

            curr->str = malloc(1);
            curr->str[0] = ')';

            curr->len = 1;
        }
        if(input->data[i] == ' ')
        {
            curr->type = Space;

            curr->str = malloc(1);
            curr->str[0] = ' ';

            curr->len = 1;
        }
        if(input->data[i] >= '0' && input->data[i] <= '9')
        {
            curr->type = Number;

            curr->str = malloc(1);
            curr->str[0] = input->data[i];

            curr->len = 1;
        }
        if(input->data[i] >= 'a' && input->data[i] <= 'z')
        {
            vector *keyword = init_vector();
            int j = 0;

            while(input->data[i + j] >= 'a' && input->data[i + j] <= 'z')
            {
                v_append(keyword, input->data[i + j]);
                j++;
            }

            i += j - 1;

            if(match_keyword(keyword, ADD, sizeof(ADD)))
            {
                curr->type = Add;
            } 
            if(match_keyword(keyword, SUB, sizeof(SUB)))
            {
                curr->type = Sub;
            }
            if(match_keyword(keyword, MUL, sizeof(MUL)))
            {
                curr->type = Mul;
            }
            if(match_keyword(keyword, DIV, sizeof(DIV)))
            {
                curr->type = Div;
            }
            else
            {
                curr->type = Unknown;
            }

            curr->str = malloc(keyword->len);
            curr->len = keyword->len;

            for(int i = 0; i < keyword->len; i++)
            {
                curr->str[i] = keyword->data[i];
            }

            v_destruct(keyword);
        }
        if(input->data[i] == '\n')
        {
            printf("done\n\n");

            curr->type = End;
            curr->next_token = NULL;
            
            curr->str = malloc(1);
            curr->len = 1;

            break;
        }
        
        curr->next_token = malloc(sizeof(struct token));
        curr = curr->next_token;
    }

    return start;
}
