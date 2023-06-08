#include "Explosions.h"
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
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
/**
 * @brief Renderuje (wyswietla) istniejace eksplozje.
 *
 * @param expl Wskaznik na liste eksplozji.
 * @param cam_x_offset Offset kamery na osi X.
 * @param cam_y_offset Offset kamery na osi Y.
 */
void renderExplosions(struct Explosion* expl, float cam_x_offset, float cam_y_offset)
{
    if (expl != NULL)
    {
        struct Explosion* exp = expl;
        int i = 0;
        while (expl != NULL && exp != NULL)
        {
            if (exp == NULL) {
                break;
            }
            
            float size = 128 * exp->timeLeft;
            al_draw_filled_circle(exp->gridX - cam_x_offset, exp->gridY - cam_y_offset, size, al_map_rgb(180, 140, 0));
            if (exp != NULL) {
                exp = exp->next;
            }
            else break;
        }
    }
}
/**
 * @brief Funkcja obslugujaca wszystkie istniejace eksplozje wywolywana co klatke.
 *
 * @param expl Podwojny wskaznik na liste eksplozji.
 * @param removed Wskaznik na wartosc logiczna (czy zniszczono jakas bombe, zwracanie przez wskaznik).
 * @param deltaTime Czas od ostatniej klatki.
 */
void loopExplosions(struct Explosion** expl, bool* removed, float deltaTime)
{
    if (expl != NULL)
    {
        struct Explosion* exp = (*expl);
        while (exp != NULL)
        {
            exp->timeLeft -= deltaTime;
            if (exp->timeLeft <= 0)
            {
                (*removed) = Explosion_Remove(&exp);
                (*expl) = exp;
                return;
            }
            if (exp->next != NULL) {
                exp = exp->next;
            }
            else break;
        }
    }
}
