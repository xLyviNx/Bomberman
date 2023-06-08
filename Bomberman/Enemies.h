#pragma once
#include <stdbool.h>
#include "algif5-master/src/algif.h"

/**
 * @brief Struktura przeciwnika
 */
struct Enemy
{
    float x;/**< Pozycja na osi X. */
    float y;/**< Pozycja na osi Y. */
    ALGIF_ANIMATION* myAnim; /**< Animacja przeciwnika (GIF/sprite). */
    struct Enemy* next; /**< Nastepny element listy przeciwnikow. */
    struct Enemy* prev; /**< Poprzedni element listy przeciwnikow. */
    float speed; /**< Predkosc przeciwnika. */
    int movedir; /**< Kierunek poruszania sie przeciwnika. */
    float randomTurn; /**< Czas do nastepnego wylosowania kierunku poruszania sie przeciwnika. */
    bool throughWalls; /**< Czy przeciwnik moze poruszac sie przez sciany. */
};

bool Enemy_Add(struct Enemy**, float, float, float, ALGIF_ANIMATION*, bool);
bool Enemy_Remove(struct Enemy*, struct Enemy**);
void Enemies_Clear(struct Enemy**);
void Enemies_Draw(struct Enemy*, double, float, float);
void Enemies_Loop(struct Enemy*, double, float, float, struct Character*, float, struct dstr_block*, struct BombList*,bool);
bool Enemy_CollidesWithPlayer(struct Enemy*, struct Character*, float);
struct Enemy* Enemy_FindAt(struct Enemy*, struct Character*, float, float, struct Enemy*, float);
int Enemies_Count(struct Enemy*);
void Enemy_Move(struct Enemy*, struct Enemy*, float, struct dstr_block*, struct BombList*, struct Character*);
bool CollideWithPlayer(float, float, struct Character*, float);
void Enemy_RandomPosition(struct Enemy*, float*, float*, struct dstr_block*, struct Character*, int, struct BombList*);