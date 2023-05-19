#pragma once
#include <allegro5/allegro5.h>
#include "types.h"
#include <stdbool.h>
typedef struct endDoor
{
	float x;
	float y;
}endDoor;
endDoor* endingDoor;
ALLEGRO_BITMAP* doorSprite;

void CreateDoor(float, float);
void DestroyDoor();
bool Door_hasPlayerIn(struct Character*);