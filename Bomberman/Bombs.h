#pragma once
#include "allegro5/allegro_audio.h"
#include "types.h"
#include "boosts.h"
/**
 * @brief Struktura bomby
 */
struct BombList
{
    float timeLeft; /**< Czas pozostaly do eksplozji. */
    struct Vector2 Position; /**< Pozycja. */
    bool exploded; /**< Czy bomba wybuchla. */
    bool insidePlayer; /**< Czy w bombie jest gracz (po podlozeniu). */
    struct BombList* next;  /**< Nastepny element listy. */
    struct BombList* prev;/**< Poprzedni element listy. */
};

struct BombList* Bomb_CreateList(struct Vector2);
bool Bomb_RemoveList(struct BombList**);
struct BombList* Bomb_InsertInto(struct BombList*, struct Vector2);
unsigned int Bomb_count(struct BombList*);
bool Bomb_ExistsAt(struct BombList*, struct Vector2);
bool Bomb_checkPlayer(struct BombList*, struct Character*);
bool Bomb_Remove(struct BombList**, struct BombList**, struct Character*);
void plantBomb(bool, struct Character*, struct BombList**, struct dstr_block*);
void renderBombs(ALGIF_ANIMATION*, double, float, float, struct Character*, struct BombList*);
void explodeBomb(struct BombList**, struct BombList**, struct dstr_block**, ALLEGRO_SAMPLE*, struct SampleStackElement**, struct Character*, float*, struct Explosion**, struct Enemy**, struct Boost**);
void loopBombs(struct BombList**, struct dstr_block**, ALLEGRO_SAMPLE*, struct SampleStackElement**, float dT, struct Character*, float*, struct Explosion**, struct Enemy**, struct Boost**);
struct BombList* Bomb_Find(struct BombList*, int, int, bool, struct Character*);
