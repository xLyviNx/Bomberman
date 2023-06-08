#include "Enemies.h"
#include "Blocks.h"
#include "Bombs.h"
#include <stdlib.h>
#include "types.h"
#include <time.h>
#ifndef WIDTH
	#define WIDTH 832
	#define HEIGHT 832
#endif // !WIDTH
/**
 * @brief Dodaje przeciwnika (tworzac przy tym ewentualnie nowa liste)
 *
 * @param list Podwojny wskaznik na liste przeciwnikow.
 * @param initX Pozycja startowa przeciwnika na osi X.
 * @param initY Pozycja startowa przeciwnika na osi Y.
 * @param speed Predkosc przeciwnika.
 * @param Animation Animacja przeciwnika.
 * @param throughWalls Czy przeciwnik bedzie przechodzic przez sciany.
 * @return Czy przeciwnik zostal dodany
 */
bool Enemy_Add(struct Enemy** list, float initX, float initY, float speed, ALGIF_ANIMATION * Animation, bool throughWalls)
{
	struct Enemy* new = (struct Enemy*)malloc(sizeof(struct Enemy));
	if (new) {
		new->next = NULL;
		new->prev = NULL;
		new->myAnim = Animation;
		new->speed = speed;
		new->x = initX;
		new->y = initY;
		new->throughWalls = throughWalls;
		srand(time(NULL) + Enemies_Count(*list) * 10);
		new->movedir = (rand() % 4) + 1;
		new->randomTurn = (rand() % 15) + 1;
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
/**
 * @brief Usuwa przeciwnika
 *
 * @param element Wskaznik na przeciwnika.
 * @param first Podwojny wskaznik na liste przeciwnikow.
 * @return Czy przeciwnik zostal usuniety
 */
bool Enemy_Remove(struct Enemy* element, struct Enemy** first)
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
/**
 * @brief Czysci i usuwa liste przeciwnikow
 *
 * @param list Podwojny wskaznik na liste przeciwnikow.
 */
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

/**
 * @brief Renderuje (wyswietla) istniejacych przeciwnikow
 *
 * @param list Wskaznik na liste przeciwnikow.
 * @param animationtime Czas animacji.
 * @param x_of Offset kamery na osi X.
 * @param y_of Offset kamery na osi Y.
 */
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
			else
			{
				//printf("1: info: %lf, %lf\n", curr->x, curr->y);
			}
		}
		else
		{
			//printf("2: info: %lf, %lf\n", curr->x, curr->y);
		}
		curr = curr->next;
	}

}
/**
 * @brief Porusza przeciwnikiem.
 *
 * @param list Wskaznik na liste przeciwnikow.
 * @param me Wskaznik na przeciwnika.
 * @param dT deltaTime (czas od ostatniej wyswietlonej klatki).
 * @param blocks Wskaznik na liste blokow.
 * @param bombs Wskaznik na liste bomb.
 * @param Player Wskaznik na gracza.
 */
void Enemy_Move(struct Enemy* list, struct Enemy* me, float dT, struct dstr_block* blocks, struct BombList* bombs, struct Character* Player)
{
	if (me && list)
	{
		struct Vector2 nDir;
		struct Vector2 dir;
		dir.x = 0;
		dir.y = 0;
		switch (me->movedir)
		{
		case 1:
		{
			dir.x = 1;
			break;
		}
		case 2:
		{
			dir.x = -1;
			break;
		}
		case 3:
		{
			dir.y = 1;
			break;
		}
		case 4:
		{
			dir.y = -1;
			break;
		}
		default:
			break;
		}
		nDir.x = (dir.x * dT) * me->speed;
		nDir.y = (dir.y * dT) * me->speed;
		float npx = (me->x + nDir.y);
		float npy = (me->y + nDir.x);
		bool dstr = false;
		bool onBlock = is_on_block(blocks, npx, npy, &dstr, Player, bombs, true);
		if (!onBlock) { dstr = false; }
		bool moved = false;
		if (!onBlock || (onBlock && dstr && me->throughWalls)) {
			if (me->throughWalls)
			{
				//printf("Block: %d, dstr?")
			}
			if (npx >= (Player->Transform.scale.y / 2.0) * 128 && npx <= ((WIDTH * 2) - (Player->Transform.scale.y / 2.0) * 128) + 1)
			{
				if (npy > (Player->Transform.scale.y / 2.0) * 128 && npy < HEIGHT + 1 - ((128 * Player->Transform.scale.y) / 2.0))
				{

					struct Enemy* at = Enemy_FindAt(list, Player, npx, npy, me, 1);
					if (!at) {
						if (npx < 0) npx = 0;
						me->x = npx;
						me->y = npy;
						moved = true;
					}
				}
			}
		}
		me->randomTurn -= dT;
		if (!moved || me->randomTurn<=0.0)
		{
			int oldDir = me->movedir;
			srand(time(NULL) + Enemies_Count(list) * 10);
			do {
				me->movedir = (rand() % 4) + 1;
			} while (me->movedir == oldDir);
			me->randomTurn = (rand() % 15) + 1;

		}
	}
}
/**
 * @brief Petla przeciwnikow (funkcja dzialajaca na kazdym przeciwniku co klatke)
 *
 * @param list Wskaznik na liste przeciwnikow.
 * @param animationtime Czas animacji.
 * @param xof Offset kamery na osi X.
 * @param yof Offset kamery na osi Y.
 * @param Player Wskaznik na gracza.
 * @param dT deltaTime (czas od ostatniej wyswietlonej klatki).
 * @param blocks Wskaznik na liste blokow.
 * @param bombs Wskaznik na liste bomb.
 * @param Paused Czy pauza jest aktywna.
 */
void Enemies_Loop(struct Enemy* list, double animationtime, float xof, float yof, struct Character* Player, float dT, struct dstr_block* blocks, struct BombList* bombs, bool Paused)
{
	struct Enemy* curr = list;
	while (curr)
	{
		if (Enemy_CollidesWithPlayer(curr, Player, 0.5))
		{
			Player->enabled = false;
		}
		if (!Paused && Player->enabled) {
			Enemy_Move(list, curr, dT, blocks, bombs, Player);
		}
		curr = curr->next;
	}
	Enemies_Draw(list, animationtime, xof, yof);
}
/**
 * @brief Sprawdza czy przeciwnik koliduje z graczem.
 *
 * @param me Wskaznik na przeciwnika.
 * @param player Wskaznik na gracza.
 * @param hitboxScale Dostosowanie wielkosci wykrywania kolizji.
 * @return Wartosc logiczna - Czy przeciwnik koliduje z graczem.
 */
bool Enemy_CollidesWithPlayer(struct Enemy* me, struct Character* player, float hitboxScale)
{
	return CollideWithPlayer(me->x, me->y, player, 0.5);
}
/**
 * @brief Sprawdza czy cos koliduje z graczem.
 *
 * @param X Wspolrzedna na osi X.
 * @param Y Wspolrzedna na osi Y.
 * @param player Wskaznik na gracza.
 * @param hitboxScale Dostosowanie wielkosci wykrywania kolizji.
 * @return Wartosc logiczna - czy cos koliduje z graczem.
 */
bool CollideWithPlayer(float X, float Y, struct Character* player, float hitboxScale)
{
	int gridSize = (int)(128 * player->Transform.scale.x);
	gridSize = (gridSize * hitboxScale);
	if (X< player->Transform.position.x + gridSize && X > player->Transform.position.x - gridSize)
	{
		if (Y < player->Transform.position.y + gridSize && Y > player->Transform.position.y - gridSize)
		{
			return true;
		}
	}
	return false;
}
/**
 * @brief Sprawdza czy na danych koordynatach jest jakis przeciwnik.
 *
 * @param first Wskaznik na liste przeciwnikow.
 * @param player Wskaznik na gracza.
 * @param X Wspolrzedna na osi X.
 * @param Y Wspolrzedna na osi Y.
 * @param skipThis Wskaznik na przeciwnika ktorego ma nie brac pod uwage przy sprawdzaniu.
 * @param hitboxSize Dostosowanie wielkosci wykrywania.
 * @return Znaleziony przeciwnik (lub NULL).
 */
struct Enemy* Enemy_FindAt(struct Enemy* first, struct Character* player, float X, float Y, struct Enemy* skipThis, float hitboxSize)
{
	int gridSize = (int)(128 * player->Transform.scale.x);
	//gridSize = (gridSize * hitboxScale);
	struct Enemy* curr = first;
	gridSize *= hitboxSize;
	while (curr)
	{
		if (curr->x< X + gridSize && curr->x > X - gridSize)
		{
			if (curr->y < Y + gridSize && curr->y > Y - gridSize)
			{
				if (skipThis != curr) {
					return curr;
				}
			}
		}
		curr = curr->next;
	}

	return NULL;
}
/**
 * @brief Sprawdza ile przeciwnikow istnieje.
 *
 * @param list Wskaznik na liste przeciwnikow.
 * @return Ilosc przeciwnikow.
 */
int Enemies_Count(struct Enemy* list)
{
	struct Enemy* curr = list;
	int count = 0;
	while (curr)
	{
		count++;
		curr = curr->next;
	}
	return count;
}
/**
 * @brief Zwraca losowe koordynaty dla przeciwnika w zaleznosci od poziomu
 *
 * @param list Wskaznik na liste przeciwnikow.
 * @param X Wskaznik na wspolrzedna osi X (zwracanie poprzez wskaznik).
 * @param Y Wskaznik na wspolrzedna osi Y (zwracanie poprzez wskaznik).
 * @param blocks Wskaznik na liste blokow.
 * @param Player Wskaznik na gracza.
 * @param level Poziom.
 * @param bombs Wskaznik na liste bomb.
 */
void Enemy_RandomPosition(struct Enemy* list, float* X, float* Y, struct dstr_block* blocks, struct Character* Player, int level, struct BombList* bombs)
{
	int i = Enemies_Count(list);
	unsigned int seed = (unsigned int)(level * 10000 + i * 100 + level * i);
	srand(seed);
	int gridSize = (int)(128 * Player->Transform.scale.x);
	int rX = 0;
	int rY = 0;
	bool dstr = false;
	do
	{
		rX = (rand() % ((WIDTH*2) / gridSize)) - 1;
		rY = (rand() % (HEIGHT / gridSize)) -1;
		if (rX < 1) { rX = 1; }
		if (rY < 1) { rY = 1; }
		*X = (rX * gridSize) - (gridSize / 2);
		*Y = (rY * gridSize) - (gridSize / 2);

	} while (is_on_block(blocks, *X, *Y, &dstr, Player, bombs, true) || CollideWithPlayer(*X, *Y, Player, 2) || Enemy_FindAt(list, Player, *X, *Y, NULL, 1));
	printf("Spawning at: %lf, %lf\n", *X, *Y);
}