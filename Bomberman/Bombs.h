#pragma once
#include "allegro5/allegro_audio.h"
struct BombList
{
    float timeLeft;
    struct Vector2 Position;
    bool isRemote;
    bool exploded;
    bool insidePlayer;
    struct BombList* next;
    struct BombList* prev;
};

struct BombList* Bomb_CreateList(struct Vector2, bool);
bool Bomb_RemoveList(struct BombList**);
struct BombList* Bomb_InsertInto(struct BombList*, struct Vector2, bool);
unsigned int Bomb_count(struct BombList*);
bool Bomb_ExistsAt(struct BombList*, struct Vector2);
bool Bomb_checkPlayer(struct BombList*, struct Character*);
bool Bomb_Remove(struct BombList**, struct BombList**, struct Character*);
void plantBomb(bool, struct Character*, struct BombList**);
void renderBombs(ALGIF_ANIMATION*, double, float, float, struct Character*, struct BombList*);
void explodeBomb(struct BombList**, struct BombList**, struct dstr_block**, ALLEGRO_SAMPLE*, struct SampleStackElement**, struct Character*, float*, struct Explosion**);
void loopBombs(struct BombList**, struct dstr_block**, ALLEGRO_SAMPLE*, struct SampleStackElement**, float dT, struct Character*, float*, struct Explosion**);
struct BombList* Bomb_Find(struct BombList*, int, int, bool, struct Character*);
