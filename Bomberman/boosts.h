#pragma once
#include <stdbool.h>
#include <allegro5/allegro5.h>
struct Boost
{
	float x;
	float y;
	unsigned short Type;
	struct Boost* next;
	struct Boost* prev;
};


void Boost_Add(struct Boost**, unsigned short, float, float);
bool Boost_Remove(struct Boost*, struct Boost**);
void Boost_Loop(struct Boost**, struct Character*, float, float, ALLEGRO_BITMAP*, ALLEGRO_BITMAP*, ALLEGRO_BITMAP*);
bool Boost_hasPlayerIn(struct Boost*, struct Character*);
void Boosts_Clear(struct Boost**);

