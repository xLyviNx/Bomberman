#pragma once
#include <stdbool.h>
#include "algif5-master/src/algif.h"
typedef struct Enemy
{
    float x;
    float y;
    ALGIF_ANIMATION* myAnim;
    struct Enemy* next;
    struct Enemy* prev;
} Enemy;

bool Enemy_Add(Enemy**, float, float, ALGIF_ANIMATION*);
bool Enemy_Remove(Enemy*);
bool Enemies_Clear(Enemy**);
void Enemies_Draw(Enemy*, double);
void Enemies_Loop(Enemy*, double);
//void Enemies_Spawn(Enemy**, int, ALGIF_ANIMATION*);
void Enemy_CollidesWithPlayer(Enemy*);