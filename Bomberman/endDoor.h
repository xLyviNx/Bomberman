#pragma once
#include <allegro5/allegro5.h>
#include "types.h"
#include <stdbool.h>
/**
 * \brief Struktura drzwi
 */
typedef struct endDoor
{
	float x; /**< Pozycja na osi X. */
	float y; /**< Pozycja na osi X. */
}endDoor;

/**
 * \brief Wskaznik na drzwi.
 */
endDoor* endingDoor;
/**
 * \brief Wskaznik na grafike (sprite) drzwi.
 */
ALLEGRO_BITMAP* doorSprite;

void CreateDoor(float, float);
void DestroyDoor();
bool Door_hasPlayerIn(struct Character*);