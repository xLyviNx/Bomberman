#include "boosts.h"
#include "types.h"
#include <allegro5/allegro_primitives.h>
void Boost_Add(struct Boost** list, unsigned short typ, float x, float y)
{
	struct Boost* new = (struct Boost*)malloc(sizeof(struct Boost));
	if (new) {
		new->x = x;
		new->y = y;
		new->Type = typ;
		new->next = NULL;
		new->prev = NULL;
		if (*list)
		{
			struct Boost* end = *list;
			while (end->next)
			{
				end = end->next;
			}
			end->next = new;
			new->prev = end;
		}
		else
		{
			*list = new;
		}
	}
}
bool Boost_Remove(struct Boost* element, struct Boost** first)
{
	if (element) {
		if (*first == element)
		{
			if (element->next)
				*first = element->next;
			else
				*first = NULL;
		}
		if (element->prev)
		{
			element->prev->next = element->next;
		}
		if (element->next)
		{
			element->next->prev = element->prev;
		}
		free(element);
		return true;
	}
	return false;
}
void Boost_Loop(struct Boost** list, struct Character* Player, float cxo, float cyo, ALLEGRO_BITMAP* bmp1, ALLEGRO_BITMAP* bmp2, ALLEGRO_BITMAP* bmp3)
{
	struct Boost* curr = *list;
	while (curr)
	{
		ALLEGRO_BITMAP* bmp = NULL;
		if (curr->Type == 1) { bmp = bmp1; }
		else if (curr->Type == 2) { bmp = bmp2; }
		else if (curr->Type == 3) { bmp = bmp3; }
		else { continue; }
		//al_draw_rectangle(curr->x - 32 - cxo, curr->y - 32 - cyo, curr->x + 32 - cxo, curr->y + 32 - cyo, al_map_rgb(255, 255, 0), 10);
		al_draw_scaled_bitmap(bmp, 0, 0, 128, 128, curr->x - 32 - cxo, curr->y - 32 - cyo, 128 * (Player->Transform.scale.x), 128 * (Player->Transform.scale.y), 0);
		if (Boost_hasPlayerIn(curr, Player))
		{
			switch (curr->Type)
			{
				case 1:
				{
					Player->bombRange++;
					break;
				}
				case 2:
				{
					Player->Speed++;
					break;
				}
				case 3:
				{
					Player->maxBombs++;
					break;
				}
				default:
				{
					break;
				}
			}
			Boost_Remove(curr, list);
			return;
		}
		curr = curr->next;
	}
}

bool Boost_hasPlayerIn(struct Boost* me, struct Character* Player)
{
	if (me)
	{
		int gridSize = (int)(128 * Player->Transform.scale.x);
		if (me->x< Player->Transform.position.x + gridSize && me->x > Player->Transform.position.x - gridSize)
		{
			if (me->y < Player->Transform.position.y + gridSize && me->y > Player->Transform.position.y - gridSize)
			{
				return true;
			}
		}
	}
	return false;
}
void Boosts_Clear(struct Boost** list)
{
	if (list)
	{
		while (*list != NULL)
		{
			struct Boost* next = (*list)->next;
			free(*list);
			*list = next;
		}
	}
}
