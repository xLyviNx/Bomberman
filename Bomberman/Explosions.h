#pragma once
#include <stdio.h>
#include <stdbool.h>

struct Explosion
{
    float timeLeft;
    int gridX;
    int gridY;
    int i;
    struct Explosion* next;
    struct Explosion* prev;
};

struct Explosion* Explosion_Insert(struct Explosion**, int, int, int);
bool Explosion_Remove(struct Explosion**);
bool Explosion_RemoveList(struct Explosion**);
