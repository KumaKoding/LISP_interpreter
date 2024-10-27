#include <stdio.h>
#include <stdlib.h>

#include "types.h"

PairTable *pt_init()
{
    PairTable *pt = malloc(sizeof(PairTable));

    pt->max = PTABLE_INIT_SIZE;
    pt->len = 0;

    pt->pairs = calloc(PTABLE_INIT_SIZE, sizeof(Pair*));

    return pt;
}

int pt_hash(Vector *key, int table_size)
{
    int h = 0;

    for(int i = 0; i < key->len; i++)
    {
        h += key->data[i]; // - (int)'a' creates negative hash
    }

    return h % table_size;
}

void insert_last(Pair **head_pair, Pair *append_pair)
{
    if(*head_pair == NULL)
    {
        *head_pair = append_pair;

        (*head_pair)->next_item = NULL;
        (*head_pair)->n_next = 0;

        return;
    }

    Pair **curr = head_pair;
    int n_next = (*curr)->n_next;

    for(int i = 0; i <= n_next; i++)
    {
        (*curr)->n_next++;
        curr = &(*curr)->next_item;
    }

    *curr = append_pair;
}

void disperse_insert(Pair **pairs, int p_len, Pair *pair)
{
    int n_next = pair->n_next;
    Pair *curr = pair;

    for(int i = 0; i <= n_next; i++)
    {
        Pair *next = curr->next_item;
        int new_hash = pt_hash(curr->key, p_len);

        curr->n_next = 0;
        curr->next_item = NULL;
        insert_last(&pairs[new_hash], curr);

        curr = next;
    }
}

void pt_insert(PairTable *pt, Vector *key, Expr *instruction)
{
    if(pt_find(pt, key))
    {
        printf("ERROR: entry already defined: ");
        v_println(key);

        return;
    }

    Pair *new_pair = malloc(sizeof(Pair));
    new_pair->instructions = instruction;
    new_pair->key = key;
    new_pair->next_item = NULL;
    new_pair->n_next = 0;

    pt->len++;

    // printf("%d %d\n", pt->len, pt->max);

    if(pt->len < pt->max)
    {
        int hash = pt_hash(key, pt->max);

        insert_last(&pt->pairs[hash], new_pair);

        return;
    }

    Pair **new_pairs = calloc(pt->max * 2, sizeof(Pair*));

    for(int i = 0; i < pt->max; i++)
    {
        if(pt->pairs[i] != NULL)
        {
            disperse_insert(new_pairs, pt->max * 2, pt->pairs[i]);
        }
    }

    pt->max *= 2;

    int hash = pt_hash(new_pair->key, pt->max);

    insert_last(&new_pairs[hash], new_pair);

    free(pt->pairs);

    pt->pairs = new_pairs;
}

Pair *pt_find(PairTable *pt, Vector *key)
{
    Pair *head = pt->pairs[pt_hash(key, pt->max)];

    if(!head)
    {
        return NULL;
    }

    int n_next = head->n_next;

    for(int i = 0; i <= n_next; i++)
    {
        if(vec_cmp_vec(key, head->key))
        {
            return head;
        }

        head = head->next_item;
    }

    return NULL;
}

void pt_delete(PairTable *pt, Vector *key)
{
    Pair *head = pt->pairs[pt_hash(key, pt->max)];
    Pair *targ = pt_find(pt, key);

    printf("%p\n", targ);
    v_println(targ->key);
    v_println(head->key);

    if(targ)
    {
        int head_n_next = head->n_next;
        Pair *prev = NULL;

        printf("%d %d %d\n", head_n_next, targ->n_next, (head_n_next - targ->n_next));

        for(int i = 0; i < (head_n_next - targ->n_next); i++)
        {
            prev = head;
            head->n_next--;
            head = head->next_item;
        }

        if((head_n_next - targ->n_next) > 0)
        {
            prev->next_item = head->next_item;
        
            v_destruct(head->key);
            e_destruct(head->instructions); 
            free(head);
        }
        else if(head_n_next == 0)
        {
            pt->pairs[pt_hash(key, pt->max)] = NULL;
            
            v_destruct(head->key);
            e_destruct(head->instructions); 
            free(head);

        }
        else if((head_n_next - targ->n_next) == 0)
        {
            pt->pairs[pt_hash(key, pt->max)] = head->next_item;

            v_destruct(head->key);
            e_destruct(head->instructions);
            free(head);
        }

        pt->len--;
    }
}

void pt_destruct(PairTable *pt)
{
    for(int i = 0; i < pt->max; i++)
    {
        Pair *curr = pt->pairs[i];

        if(curr)
        {
            for(int n = 0; n < pt->pairs[i]->n_next - 1; n++)
            {
                Pair *prev = curr;
                curr = curr->next_item;

                v_destruct(prev->key);
                e_destruct(prev->instructions);
                free(prev);
            }
        }
    }
}

void pt_print(PairTable *pt)
{
    int counter = 0;

    for(int i = 0; i < pt->max; i++)
    {
        printf("%4d", i);

        if(pt->pairs[i] != NULL)
        {
            Pair *curr = pt->pairs[i];

            for(int j = 0; j <= pt->pairs[i]->n_next; j++)
            {
                printf(" ");
                v_print(curr->key);

                curr = curr->next_item;

                counter++;
            }
        }

        printf("\n");
    }

    printf("elems: %d\n", counter);
}
