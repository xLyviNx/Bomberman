#include "Enemies.h"
#include <stdlib.h>
#include "config.h"
bool Enemy_Add(Enemy** list, float initX, float initY, ALGIF_ANIMATION* Animation)
{
	Enemy* new = (Enemy*)malloc(sizeof(Enemy*));
	if (new) {
		new->next = NULL;
		new->prev = NULL;
		new->myAnim = Animation;
		new->x = initX;
		new->y = initY;
		if (*list)
		{
			Enemy* end = *list;
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
		return true;
	}
	return false;
}
bool Enemy_Remove(Enemy* element)
{
	if (element) {
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
bool Enemies_Clear(Enemy** list)
{
	if (list)
	{
		while (*list != NULL)
		{
			Enemy* next = (*list)->next;
			//free(*list);
			*list = next;
		}
		return *list == NULL;
	}
	return false;
}

void Enemies_Draw(Enemy* list, double animationtime, float x_of, float y_of)
{
	Enemy* curr = list;
	while (curr)
	{
		if (curr->x - x_of > -32 && curr->x - x_of < WIDTH + 32)
		{
			if (curr->myAnim)
			{
				al_draw_scaled_bitmap(algif_get_bitmap(curr->myAnim, animationtime), 0, 0, 128, 128, curr->x - 32 - x_of, curr->y - 32 - y_of, 64, 64, 0);
			}
		}
		curr = curr->next;
	}

}
void Enemies_Loop(Enemy* list, double animationtime, float xof, float yof)
{
	Enemy* curr = list;
	while (curr)
	{
		curr = curr->next;
	}
	Enemies_Draw(list, animationtime, xof, yof);
}
void Enemy_CollidesWithPlayer(Enemy* me, float X, float Y, float scale)
{
	int gridSize = (int)(128 * scale);

	if (me->x< X + gridSize && me->x > X - gridSize)
	{
		if (me->y < Y + gridSize && me->y > Y - gridSize)
		{
			return true;
		}
	}
	return false;
}