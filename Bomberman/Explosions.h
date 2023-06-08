#pragma once
#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Struktura eksplozji
 */
struct Explosion
{
    float timeLeft; /**< Czas pozostaly do znikniecia. */
    int gridX; /**< Pozycja na osi X. */
    int gridY; /**< Pozycja na osi Y. */
    int i; /**< Odleglosc od bomby. */
    struct Explosion* next; /**< Nastepny element listy. */
    struct Explosion* prev; /**< Poprzedni element listy. */
};

struct Explosion* Explosion_Insert(struct Explosion**, int, int, int);
bool Explosion_Remove(struct Explosion**);
bool Explosion_RemoveList(struct Explosion**);
void renderExplosions(struct Explosion*, float, float);
void loopExplosions(struct Explosion**, bool*, float);