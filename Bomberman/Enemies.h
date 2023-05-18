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
};

bool Enemy_Add(struct Enemy**, float, float, ALGIF_ANIMATION*);
bool Enemy_Remove(struct Enemy*);
void Enemies_Clear(struct Enemy**);
void Enemies_Draw(struct Enemy*, double, float, float);
void Enemies_Loop(struct Enemy*, double, float, float);
//void Enemies_Spawn(struct Enemy**, int, ALGIF_ANIMATION*);
void Enemy_CollidesWithPlayer(struct Enemy*, float, float, float);