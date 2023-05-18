#include "Enemies.h"
#include <stdlib.h>
#ifndef WIDTH
	#define WIDTH 832
	#define HEIGHT 832
#endif // !WIDTH
bool Enemy_Add(struct Enemy** list, float initX, float initY, ALGIF_ANIMATION * Animation)
{
	struct Enemy* new = (struct Enemy*)malloc(sizeof(struct Enemy));
	if (new) {
		new->next = NULL;
		new->prev = NULL;
		new->myAnim = Animation;
		new->x = initX;
		new->y = initY;
		if (*list)
		{
			struct Enemy* end = *list;
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
bool Enemy_Remove(struct Enemy* element)
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
void Enemies_Clear(struct Enemy** list)
{
	if (list)
	{
		while (*list != NULL)
		{
			struct Enemy* next = (*list)->next;
			free(*list);
			*list = next;
		}
	}
}


void Enemies_Draw(struct Enemy* list, double animationtime, float x_of, float y_of)
{
	struct Enemy* curr = list;
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
void Enemies_Loop(struct Enemy* list, double animationtime, float xof, float yof)
{
	struct Enemy* curr = list;
	while (curr)
	{
		curr = curr->next;
	}
	Enemies_Draw(list, animationtime, xof, yof);
}
void Enemy_CollidesWithPlayer(struct Enemy* me, float X, float Y, float scale)
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