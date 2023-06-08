#include "Explosions.h"
#include <stdlib.h>

/**
 * @brief Tworzy i umieszcza nowy element na liscie eksplozji.
 *
 * @param first Podwojny wskaznik na liste eksplozji.
 * @param X Pozycja na osi X.
 * @param Y Pozycja na osi Y.
 * @param i Odleglosc od bomby.
 * @return Stworzona eksplozja.
 */
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
            t = t->next;
            i++;
        }

    }
    return nE;
}
/**
 * @brief Usuwa eksplozje
 *
 * @param exp Podwojny wskaznik na eksplozje
 * @return Czy usunieto eksplozje.
 */
bool Explosion_Remove(struct Explosion** exp)
{
    if (exp != NULL && (*exp) != NULL)
    {
        struct Explosion* prev = (*exp)->prev;
        struct Explosion* next = (*exp)->next;
       
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
            (*exp) = next;
        }
        else {
            (*exp) = NULL;
        }
        return true;
    }
    return false;
}
/**
 * @brief Usuwa liste eksplozji
 *
 * @param explosions Podwojny wskaznik na liste eksplozji
 * @return Czy usunieto liste eksplozji.
 */
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