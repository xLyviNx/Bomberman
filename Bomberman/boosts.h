#pragma once
#include <stdbool.h>
#include <allegro5/allegro5.h>

/**
 * @brief Struktura reprezentujaca ulepszenie.
 */
struct Boost
{
    float x; /**< Wspolrzedna x ulepszenia. */
    float y; /**< Wspolrzedna y ulepszenia. */
    unsigned short Type; /**< Typ ulepszenia. */
    struct Boost* next; /**< Wskaznik na nastepne ulepszenie. */
    struct Boost* prev; /**< Wskaznik na poprzednie ulepszenie. */
};



void Boost_Add(struct Boost**, unsigned short, float, float);
bool Boost_Remove(struct Boost*, struct Boost**);
void Boost_Loop(struct Boost**, struct Character*, float, float, ALLEGRO_BITMAP*, ALLEGRO_BITMAP*, ALLEGRO_BITMAP*);
bool Boost_hasPlayerIn(struct Boost*, struct Character*);
void Boosts_Clear(struct Boost**);

