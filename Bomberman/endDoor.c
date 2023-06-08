#include "endDoor.h"


endDoor* endingDoor = NULL;

ALLEGRO_BITMAP* doorSprite = NULL;

/**
 * @brief Tworzy drzwi do nastepnego poziomu lub zmienia ich koordynaty
 *
 * @param x Wspolrzedna X
 * @param y Wspolrzedna Y.
 */
void CreateDoor(float x, float y)
{
	if (!endingDoor) {
		endingDoor = (endDoor*)malloc(sizeof(endDoor));
	}
	if (endingDoor)
	{
		endingDoor->x = x;
		endingDoor->y = y;
	}
}
/**
 * @brief Usuwa drzwi i zwalnia pamiec.
 */
void DestroyDoor()
{
	if (endingDoor)
	{
		free(endingDoor);
		endingDoor = NULL;
	}
}
/**
 * @brief Sprawdza czy gracz wszedl w drzwi.
 *
 * @param Player Wskaznik na gracza.
 * @return Wartosc logiczna - czy gracz koliduje z drzwiami.
 */
bool Door_hasPlayerIn(struct Character* Player)
{
	if (endingDoor)
	{
		int gridSize = (int)(128 * Player->Transform.scale.x);
		if (endingDoor->x< Player->Transform.position.x + gridSize && endingDoor->x > Player->Transform.position.x - gridSize)
		{
			if (endingDoor->y < Player->Transform.position.y + gridSize && endingDoor->y > Player->Transform.position.y - gridSize)
			{
				return true;
			}
		}
	}
	return false;
}