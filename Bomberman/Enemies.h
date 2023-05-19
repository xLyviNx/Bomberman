#pragma once
#include <stdbool.h>
#include "algif5-master/src/algif.h"
struct Enemy
{
    float x;
    float y;
    ALGIF_ANIMATION* myAnim;
    struct Enemy* next;
    struct Enemy* prev;
    float speed;
    int movedir;
    float randomTurn;
    bool throughWalls;
};

bool Enemy_Add(struct Enemy**, float, float, float, ALGIF_ANIMATION*, bool);
bool Enemy_Remove(struct Enemy*, struct Enemy**);
void Enemies_Clear(struct Enemy**);
void Enemies_Draw(struct Enemy*, double, float, float);
void Enemies_Loop(struct Enemy*, double, float, float, struct Character*, float, struct dstr_block*, struct BombList*,bool);
//void Enemies_Spawn(struct Enemy**, int, ALGIF_ANIMATION*);
bool Enemy_CollidesWithPlayer(struct Enemy*, struct Character*, float);
struct Enemy* Enemy_FindAt(struct Enemy*, struct Character*, float, float, struct Enemy*, float);
int Enemies_Count(struct Enemy*);
void Enemy_Move(struct Enemy*, struct Enemy*, float, struct dstr_block*, struct BombList*, struct Character*);
bool CollideWithPlayer(float, float, struct Character*, float);
void Enemy_RandomPosition(struct Enemy*, float*, float*, struct dstr_block*, struct Character*, int, struct BombList*);

//bool Enemy_CheckForBlocks(float, float, struct dstr_block*, bool*, struct Character*);