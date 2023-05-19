#include "Explosions.h"
#include <stdlib.h>
struct Explosion* Explosion_Insert(struct Explosion** first, int X, int Y, int i)
{
    printf("Inserting Explosion\n");
    struct Explosion* nE = (struct Explosion*)malloc(sizeof(struct Explosion));
    if (nE != NULL)
    {
        nE->gridX = X;
        nE->gridY = Y;
        nE->next = NULL;
        nE->prev = NULL;
        nE->timeLeft = 0.2;
        nE->i = i;
        if ((*first) != NULL)
        {
            struct Explosion* end = *first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            end->next = nE;
            nE->prev = end;
        }
        else
        {
            (*first) = nE;
        }
        struct Explosion* t = (*first);
        int i = 0;
        while (t != NULL)
        {
            //printf("- %d: prev: %p,  next: %p\n", i, t->prev, t->next);
            t = t->next;
            i++;
        }

    }
    return nE;
}
bool Explosion_Remove(struct Explosion** exp)
{
    if (exp != NULL && (*exp) != NULL)
    {
        struct Explosion* prev = (*exp)->prev;
        struct Explosion* next = (*exp)->next;
        if (next != NULL) {
            //printf("next: %d\n", next->gridX);
        }
        if (prev != NULL) {
            //printf("prev: %d\n", prev->gridX);
        }
        free((*exp));
        if (prev != NULL)
        {
            prev->next = next;
            if (next != NULL) {
                next->prev = prev;
            }
            (*exp) = prev;
        }
        else if (next != NULL)
        {
            next->prev = NULL;
            //printf("Swapping with %p\n", next);
            (*exp) = next;
        }
        else {
            (*exp) = NULL;
        }
        //printf("Now: %p\n", (*exp));
        return true;
    }
    return false;
}
bool Explosion_RemoveList(struct Explosion** explosions)
{
    while (*explosions != NULL)
    {
        struct Explosion* next = (*explosions)->next;
        free(*explosions);
        *explosions = next;
    }
    return *explosions == NULL;
}