#include "Blocks.h"
#include <stdio.h>
#include "types.h"
#include "endDoor.h"

/**
 * @brief Sprawdza, czy na danych koordynatach miesci sie blok.
 *
 * @param first Wskaznik na pierwszy element listy blokow.
 * @param X Wspolrzedna x.
 * @param Y Wspolrzedna y.
 * @param around Czy ma sprawdzac rowniez obok o 1 blok.
 * @param Player Wskaznik na gracza.
 * @return Czy blok istnieje.
 */
bool Block_Exists(struct dstr_block* first, int X, int Y, bool around, struct Character* Player)
{
    if (first != NULL) {
        struct dstr_block* block = first;
        int gridSize = (int)(128 * Player->Transform.scale.x);
        int mp = 2;
        if (!around) { mp = 1; }
        while (block != NULL)
        {
            if (block != NULL) {
                //printf("ADDRESS: %p\n", block);
                if (block->gridX < X + (gridSize / mp) && block->gridX > X - (gridSize / mp)) // here's the issue
                {
                    if (block->gridY < Y + (gridSize / mp) && block->gridY > Y - (gridSize / mp))
                    {
                        return true;
                    }
                    else if (block->gridY - gridSize == Y)
                    {
                        return around;
                    }
                    else if (block->gridY + gridSize == Y)
                    {
                        return around;
                    }
                }
                block = block->next;
                if (block == NULL)
                {
                    break;
                }
            }
            else {
                break;
            }
        }
    }
    return false;
}
/**
 * @brief Umieszcza nowy blok w liscie na danych koordynatach.
 *
 * @param first Podwojny wskaznik na pierwszy element listy blokow.
 * @param X Wspolrzedna x.
 * @param Y Wspolrzedna y.
 * @param staticBlock Wartosc logiczna - czy blok jest statyczny (niezniszczalny).
 * @return Wskaznik na nowy blok.
 */
struct dstr_block* Block_Insert(struct dstr_block** first, int X, int Y, bool staticBlock)
{
    struct dstr_block* nB = (struct dstr_block*)malloc(sizeof(struct dstr_block));
    if (nB != NULL)
    {
        nB->gridX = X;
        nB->gridY = Y;
        nB->next = NULL;
        nB->prev = NULL;
        nB->exists = true;
        nB->destroyable = !staticBlock;
        nB->hasDoor = false;
        if (first != NULL && *first != NULL)
        {
            struct dstr_block* end = *first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            end->next = nB;
            nB->prev = end;
        }
        else if (first != NULL)
        {
            *first = nB;
        }
    }
    //printf("address: %p\n", nB);

    return nB;
}
/**
 * @brief Umieszcza nowy blok w nowej liscie (tworzy go bez istniejacej listy) na danych koordynatach.
 *
 * @param X Wspolrzedna x.
 * @param Y Wspolrzedna y.
 * @param staticBlock Wartosc logiczna - czy blok jest statyczny (niezniszczalny).
 * @return Wskaznik na nowy blok.
 */
struct dstr_block* Block_createList(int X, int Y, bool staticBlock)
{
    struct dstr_block* nB = (struct dstr_block*)malloc(sizeof(struct dstr_block));
    if (nB != NULL)
    {
        nB->gridX = X;
        nB->gridY = Y;
        nB->next = NULL;
        nB->prev = NULL;
        nB->exists = true;
        nB->destroyable = !staticBlock;
        nB->hasDoor = false;
    }
    return nB;
}
/**
 * @brief Usuwa liste blokow.
 *
 * @param block Podwojny wskaznik na pierwszy element listy blokow.
 * @return Wartosc logiczna - czy usunieto.
 */
bool Block_RemoveList(struct dstr_block** block)
{
    while (*block != NULL)
    {
        struct dstr_block* next = (*block)->next;
        free(*block);
        *block = next;
    }
    return *block == NULL;
}
/**
 * @brief Szuka bloku na danych koordynatach.
 *
 * @param blocks Wskaznik na pierwszy element listy blokow.
 * @param X Wspolrzedna osi x.
 * @param Y Wspolrzedna osi y.
 * @param gridMethod Metoda szukania (czy tylko po konkretnych koordynatach siatki).
 * @param Player Wskaznik na gracza.
 * @return Blok, o ile znaleziono. (Inaczej NULL)
 */
struct dstr_block* Block_Find(struct dstr_block* blocks, int X, int Y, bool gridMethod, struct Character* Player)
{
    struct dstr_block* found = NULL;
    if (blocks != NULL)
    {
        struct dstr_block* block = blocks;
        while (block != NULL)
        {
            if (gridMethod) {
                if (block->gridX == X && block->gridY == Y && block->exists)
                {
                    //printf("Returning.\n");
                    return block;
                }
            }
            else if (block != NULL)
            {
                int gridSize = (int)(128 * Player->Transform.scale.x);

                if (block->gridX < X + gridSize && block->gridX > X - gridSize) // here's the issue
                {
                    if (block->gridY < Y + gridSize && block->gridY > Y - gridSize)
                    {
                        return block;
                    }
                }
            }
            block = block->next;
        }
    }
    return found;
}
/**
 * @brief Usuwa konkretny blok z listy.
 *
 * @param element Wskaznik na element listy blokow do usuniecia.
 * @param first Podwojny wskaznik na pierwszy element listy blokow.
 * @param enemies Wskaznik na liste przeciwnikow.
 * @return Czy usunieto blok.
 */
bool Block_Remove(struct dstr_block* element, struct dstr_block** first, struct Enemy* enemies)
{
    if (element) {
        srand(time(NULL));
        int rng = rand() % 4;
        if (!enemies && (!rng || element->hasDoor))
        {
            CreateDoor(element->gridX, element->gridY);
        }
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
 * @brief Zliczanie blokow.
 *
 * @param first Wskaznik na pierwszy element listy blokow.
 * @return Ilosc blokow.
 */
unsigned int Blocks_Count(struct dstr_block* first)
{
    unsigned int count = 0;
    //printf("first: %p\n", first);
    if (first != NULL)
    {
        if (first->destroyable) {
            count = 1;
        }
        struct dstr_block* end = first;
        while (end->next != NULL)
        {
            end = end->next;
            if (end->destroyable) {
                count++;
            }
        }
    }
    return count;
}
/**
 * @brief Renderowanie (wyswietlanie) istniejacych blokow.
 *
 * @param first Wskaznik na pierwszy element listy blokow.
 * @param dBS Sprite zniszczalnego bloku (wskaznik).
 * @param sBS Sprite niezniszczalnego bloku (wskaznik).
 * @param Player Wskaznik na gracza.
 * @param cam_x_offset Offset kamery na osi X.
 * @param cam_y_offset Offset kamery na osi Y.
 */
void Blocks_draw(struct dstr_block* first, ALLEGRO_BITMAP* dBS, ALLEGRO_BITMAP* sBS, struct Character* Player, float cam_x_offset, float cam_y_offset)
{
    struct dstr_block* temp = first;
    int gridSize = (int)(128 * Player->Transform.scale.x); // rozmiar kratki
    while (temp != NULL)
    {
        if (temp != NULL) {
            if (temp == NULL)
            {
                break;
            }
            if (temp->exists) {
                if (temp->gridX - cam_x_offset - gridSize / 2 < WIDTH && temp->gridX - cam_x_offset + gridSize / 2 > 0)
                {
                    if (temp->gridY < HEIGHT && temp->gridY > 0)
                    {
                        if (temp->destroyable) {
                            al_draw_scaled_bitmap(dBS, 0, 0, 128, 128, temp->gridX - gridSize / 2 - cam_x_offset, temp->gridY - cam_y_offset - gridSize / 2, gridSize, gridSize, 0);
                        }
                        else
                        {
                            al_draw_scaled_bitmap(sBS, 0, 0, 128, 128, temp->gridX - cam_x_offset - gridSize / 2, temp->gridY - cam_y_offset - gridSize / 2, gridSize, gridSize, 0);

                        }
                    }
                }
            }
            temp = temp->next;
        }
    }
}
/**
 * @brief Petla blokow (wykonywana co klatke).
 *
 * @param blocks Podwojny wskaznik na pierwszy element listy blokow.
 * @param enemies Wskaznik na pierwszy element listy przeciwnikow.
 */
void loopBlocks(struct dstr_block** blocks, struct Enemy* enemies)
{
    if (blocks != NULL)
    {
        struct dstr_block* bl = (*blocks);
        int i = 0;
        while (bl != NULL)
        {
            i++;
            if (!bl->exists)
            {
                Block_Remove(bl, blocks, enemies);
                return;
            }
            if (bl->next != NULL) {
                bl = bl->next;
            }
            else break;
        }
    }
}
/**
 * @brief Generuje (pseudo) losowe koordynaty dla bloku w zaleznosci od aktualnego poziomu.
 *
 * @param first Wskaznik na pierwszy element listy blokow.
 * @param level Poziom dla ktorego generuje koordynaty.
 * @param X Wskaznik na koordynaty na osi X (zwracanie poprzez wskaznik).
 * @param Y Wskaznik na koordynaty na osi Y (zwracanie poprzez wskaznik).
 * @param i Ktory blok (z petli for)
 * @param Player Wskaznik na gracza.
 */
void Block_random(struct dstr_block* first, int level, int* X, int* Y, int i, struct Character* Player)
{
    unsigned int seed = (unsigned int)(level * 10000 + i * 100 + level * i);
    srand(seed);
    int gridSize = (int)(128 * Player->Transform.scale.x);
    int rX = 0;
    int rY = 0;
    do
    {
        rX = (((rand() % (WIDTH / gridSize)) * 2) + 1);
        rY = (((rand() % (HEIGHT / gridSize)) + 1));
        *X = (rX * gridSize) - gridSize / 2;
        *Y = (rY * gridSize) - gridSize / 2;
        //*Y += 1;
    } while (Block_Exists(first, *X, *Y, true, Player) || (*X == (int)Player->Transform.position.x && *Y == (int)Player->Transform.position.y) || (*X == (int)Player->Transform.position.x && *Y == (int)Player->Transform.position.y + 64));

    //printf("%d - %d, %d - %d\n", rX, *X, rY, *Y);
}
/**
 * @brief Generuje niezniszczalne bloki.
 *
 * @param first Podwojny wskaznik na pierwszy element listy blokow.
 * @param Player Wskaznik na gracza.
 */
void GenerateStaticBlocks(struct dstr_block** first, struct Character* Player)
{
    int gridSize = (int)(128 * Player->Transform.scale.x);
    for (int i = 2; i < (WIDTH * 2 / gridSize) + 1; i += 2)
    {
        for (int j = 2; j < (HEIGHT / gridSize); j += 2)
        {
            Block_Insert(first, (i * gridSize) - gridSize / 2, (j * gridSize) - gridSize / 2, true);
        }
    }
}
/**
 * @brief Generuje zniszczalne bloki.
 *
 * @param level Ktory poziom.
 * @param Player Wskaznik na graczca.
 * @return Lista blokow.
 */
struct dstr_block* generate_blocks(int level, struct Character* Player)
{
    struct dstr_block* Blocks = NULL;
    int limit = 20 + (level * 11);
    if (limit > 75)
    {
        limit = 75;
    }
    for (int i = 0; i < limit; i++)
    {

        int X = 0;
        int Y = 0;
        Block_random(Blocks, level, &X, &Y, i, Player);
        struct dstr_block* bl = NULL;
        if (Blocks == NULL)
        {
            bl = Block_createList(X, Y, false);
        }
        else
        {
            bl = Block_Insert(&Blocks, X, Y, false);
        }
        if (bl != NULL)
        {
            bl->next = NULL;
            if (Blocks == NULL) {
                Blocks = bl;
            }
        }
    }
    GenerateStaticBlocks(&Blocks, Player);
    return Blocks;
}