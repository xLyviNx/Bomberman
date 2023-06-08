#include "types.h"
#include "Bombs.h"
#include "Blocks.h"
#include "Explosions.h"
#include "soundstack.h"
#include "Enemies.h"
#include "saveSystem.h"
#include "endDoor.h"
#include "Blocks.h"
#include "main.h"

/**
 * @brief Domyslny czas do eksplozji.
 *
 * Stala preprocesora definiujaca domyslny czas do eksplozji w sekundach.
 */
#define exploTime 3.0;

 /**
  * @brief Tworzy liste bomb (oraz pierwsza bombe).
  *
  * @param pos Pozycja bomby.
  * @return Pierwszy element nowej listy bomb.
  */
struct BombList* Bomb_CreateList(struct Vector2 pos)
{
    struct BombList* nB = (struct BombList*)malloc(sizeof(struct BombList));
    if (nB != NULL)
    {
        nB->Position = pos;
        nB->prev = NULL;
        nB->exploded = false;
        nB->insidePlayer = true;
        nB->next = NULL;
        nB->timeLeft = exploTime;
    }
    return nB;
}
/**
 * @brief Usuwa liste bomb.
 *
 * @param firstBomb Podwojny wskaznik na liste bomb (pierwszy element).
 * @return Wartosc logiczna - czy usunieto liste.
 */
bool Bomb_RemoveList(struct BombList** firstBomb)
{
    while (*firstBomb != NULL)
    {
        struct BombList* next = (*firstBomb)->next;
        free(*firstBomb);
        *firstBomb = next;
    }
    return *firstBomb == NULL;
}
/**
 * @brief Dodaje bombe do listy bomb.
 *
 * @param first Wskaznik na pierwszy element listy bomb.
 * @param pos Pozycja nowej bomby
 * @return Wskaznik na utworzona bombe.
 */
struct BombList* Bomb_InsertInto(struct BombList* first, struct Vector2 pos)
{
    struct BombList* nB = NULL;
    if (first != NULL)
    {
        nB = (struct BombList*)malloc(sizeof(struct BombList));
        if (nB != NULL)
        {
            nB->Position = pos;
            nB->timeLeft = exploTime;
            nB->exploded = false;
            nB->insidePlayer = true;
            struct BombList* end = first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            //printf("Added at the end of %p\n", end);
            nB->prev = end;
            nB->next = NULL;
            end->next = nB;
        }
    }
    return nB;
}
/**
 * @brief Liczy istniejace bomby.
 *
 * @param first Pierwszy element listy bomb.
 * @return Liczba istniejacych bomb.
 */
unsigned int Bomb_count(struct BombList* first)
{
    unsigned int count = 0;
    //printf("first: %p\n", first);
    if (first != NULL)
    {
        count = 1;
        struct BombList* end = first;
        while (end->next != NULL)
        {
            //printf("bomb: %p\n", end);
            end = end->next;
            count++;
        }
    }
    return count;
}
/**
 * @brief Sprawdza czy na danych koordynatach istnieje jakas bomba.
 *
 * @param first Pierwszy element listy bomb.
 * @param pos Pozycja bomby.
 * @return Wartosc logiczna - czy bomba istnieje
 */
bool Bomb_ExistsAt(struct BombList* first, struct Vector2 pos)
{
    if (first != NULL)
    {
        struct BombList* seek = first;
        do
        {
            //printf("%lf vs %lf\n%lf vs %lf", seek->Position.x, pos.x, seek->Position.y, pos.y);
            if (seek->Position.x == pos.x && seek->Position.y == pos.y)
            {
                return true;
            }
            seek = seek->next;
        } while (seek != NULL ? seek->next != NULL : false);
    }
    return false;
}
/**
 * @brief Znajduje bombe na danych koordynatach
 *
 * @param first Pierwszy element listy bomb.
 * @param X Pozycja bomby (na osi X).
 * @param Y Pozycja bomby (na osi Y).
 * @param gridMethod Metoda znajdywania (czy tylko na siatce i idealnych koordynatach, czy w srodku bomby, nie tylko idealne koordynaty na siatce).
 * @param Player Wskaznik na gracza.
 * @return Znaleziona bomba lub NULL.
 */
struct BombList* Bomb_Find(struct BombList* first, int X, int Y, bool gridMethod, struct Character* Player)
{
    struct BombList* found = NULL;
    if (first != NULL)
    {
        int i = 0;
        struct BombList* bomb = first;
        while (bomb != NULL)
        {
            //printf("\n%d: Bomb: %p, (%p - %p) exploded: %d\n", i, bomb, bomb->prev, bomb->next, bomb->exploded);
            i++;
            if (gridMethod) {
                if (bomb->Position.x == X && bomb->Position.y == Y)
                {
                    return bomb;
                }
            }
            else
            {
                int gridSize = (int)(128 * Player->Transform.scale.x);

                if (bomb->Position.x < X + gridSize && bomb->Position.x > X - gridSize)
                {
                    if (bomb->Position.y< Y + gridSize && bomb->Position.y> Y - gridSize)
                    {
                        return bomb;
                    }
                }
            }
            bomb = bomb->next;
        }
    }
    return found;
}
/**
 * @brief Sprawdza czy w srodku bomby jest gracz.
 *
 * @param bomb Wskaznik na bombe.
 * @param Player Wskaznik na gracza.
 * @return Czy gracz jest w srodku bomby.
 */
bool Bomb_checkPlayer(struct BombList* bomb, struct Character* Player)
{
    if (bomb != NULL)
    {

        int gridSize = (int)(128 * Player->Transform.scale.x);

        if (bomb->Position.x < Player->Transform.position.x + gridSize && bomb->Position.x > Player->Transform.position.x - gridSize)
        {
            if (bomb->Position.y< Player->Transform.position.y + gridSize && bomb->Position.y> Player->Transform.position.y - gridSize)
            {
                return true;
            }
        }
    }
    return false;
}
/**
 * @brief Usuwa bombe z listy.
 *
 * @param bomb Podwojny wskaznik na bombe.
 * @param bombs Podwojny wskaznik na liste bomb.
 * @param Player Wskaznik na gracza.
 * @return Wartosc logiczna - czy usunieto.
 */
bool Bomb_Remove(struct BombList** bomb, struct BombList** bombs, struct Character* Player)
{
    if (bombs != NULL && bomb != NULL && (*bomb) != NULL)
    {
        struct BombList* prev = (*bomb)->prev;
        struct BombList* next = (*bomb)->next;
        if (prev)
        {
            prev->next = next;
        }
        if (next)
        {
            next->prev = prev;
        }
        if (*bombs == *bomb)
        {
            if (next)
                *bombs = next;
            else
                *bombs = NULL;
        }
        free(*bomb);

        return true;
    }
    return false;
}
/**
 * @brief Podklada bombe.
 *
 * @param isPaused Czy gra jest w trakcie pauzy (blokuje wtedy podkladanie bomby).
 * @param Player Wskaznik na gracza.
 * @param bombs Podwojny wskaznik na liste bomb.
 * @param blocks Wskaznik na liste blokow.
 */
void plantBomb(bool isPaused, struct Character* Player, struct BombList** bombs, struct dstr_block* blocks)
{
    if (Player != NULL && !isPaused)
    {
        struct Vector2 pos;
        pos = Player->Transform.gridPosition;
        bool canplant = false;
        struct BombList* planted = NULL;
        if (Player->enabled)
        {
            unsigned int count = Bomb_count(*bombs);
            printf("Bombs BEFORE: %d\n", count);
            if (count > 0)
            {
                bool dst = false;
                if (count < Player->maxBombs && !is_on_block(blocks, Player->Transform.gridPosition.x, Player->Transform.gridPosition.y, &dst, Player, *bombs, true))
                {

                    canplant = true;
                }
            }
            else
            {
                planted = Bomb_CreateList(pos);
                *bombs = planted;
                printf("Bombs 0, creating a list. Address: %p\n   prev: %p\n   next: %p\n", *bombs, (*bombs)->prev, (*bombs)->next);
            }
        }
        if (canplant)
        {
            planted = Bomb_InsertInto(*bombs, pos);
        }

        if (planted == NULL)
        {
            printf("Bomb NOT planted.\n");
        }
        else {
            printf("Bomb planted.\n");
        }
    }
}
/**
 * @brief Renderowanie (wyswietlanie) istniejacych bomb.
 *
 * @param bombAnim Animacja bomby.
 * @param animtime Czas animacji bomby.
 * @param cam_x_offset Offset kamery w osi X.
 * @param cam_y_offset Offset kamery w osi Y.
 * @param Player Wskaznik na gracza.
 * @param bombs Wskaznik na liste bomb (pierwszy jej element).
 */
void renderBombs(ALGIF_ANIMATION* bombAnim, double animtime, float cam_x_offset, float cam_y_offset, struct Character* Player, struct BombList* bombs)
{
    if (bombs != NULL)
    {
        struct BombList* bomb = bombs;
        while (bombs != NULL && bomb != NULL)
        {
            if (bomb->insidePlayer)
            {
                //al_draw_filled_rectangle(bomb->Position.x - 32-cam_x_offset, bomb->Position.y - 32 - cam_y_offset, bomb->Position.x + 32 - cam_x_offset, bomb->Position.y + 32 - cam_y_offset, al_map_rgba(255, 0, 0, 50));
            }
            al_draw_scaled_rotated_bitmap(algif_get_bitmap(bombAnim, animtime), 64, 64, bomb->Position.x - cam_x_offset, bomb->Position.y - cam_y_offset, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
            if (bomb->next != NULL) {
                bomb = bomb->next;
            }
            else break;
        }
    }
}
/**
 * @brief Wysadza jedna z bomb.
 *
 * @param bomb Podwojny wskaznik na bombe.
 * @param bombs Podwojny wskaznik na liste bomb.
 * @param blocks Podwojny wskaznik na liste blokow
 * @param exploSound Dzwiek eksplozji (wskaznik na sample).
 * @param samples Podwojny wskaznik na stos dzwiekow eksplozji.
 * @param Player Wskaznik na gracza.
 * @param cam_y_offset Offset kamery w osi Y.
 * @param explosions Podwojny wskaznik na liste eksplozji.
 * @param Enemies Podwojny wskaznik na liste przeciwnikow.
 * @param Boosts Podwojny wskaznik na liste ulepszen.
 */
void explodeBomb(struct BombList** bomb, struct BombList** bombs, struct dstr_block** blocks, ALLEGRO_SAMPLE* exploSound, struct SampleStackElement** samples, struct Character* Player, float* cam_y_offset, struct Explosion** explosions, struct Enemy** Enemies, struct Boost** Boosts)
{
    if (bomb != NULL && (*bomb) != NULL)
    {

        (*bomb)->exploded = true;
        (*bomb)->timeLeft = 0.05;
        int gridSize = (int)(128 * Player->Transform.scale.x);
        int xdir = 0;
        int ydir = 0;
        *cam_y_offset += 30;
        ALLEGRO_SAMPLE_INSTANCE* newExploSound = al_create_sample_instance(exploSound);
        al_attach_sample_instance_to_mixer(newExploSound, al_get_default_mixer());
        al_play_sample_instance(newExploSound);
        *samples = SampleStack_Push(*samples, newExploSound);
        if ((*bomb)->Position.x == Player->Transform.gridPosition.x && (*bomb)->Position.y == Player->Transform.gridPosition.y)
        {
            Player->enabled = false;
        }
        int EliminatedEnemies = 0;
        int DestroyedBlocks = 0;
        float lastx, lasty;
        float blocklastx, blocklasty;
        for (int j = 0; j < 4; j++) {
            switch (j)
            {
            case 0:
            {
                xdir = 0;
                ydir = 1;
                break;
            }
            case 1:
            {
                xdir = 0;
                ydir = -1;
                break;
            }
            case 2:
            {
                xdir = -1;
                ydir = 0;
                break;
            }
            case 3:
            {
                xdir = 1;
                ydir = 0;
                break;
            }
            default:
            {
                break;
            }
            }
            for (int i = 1; i <= Player->bombRange; i++)
            {
                int X = (*bomb)->Position.x + (xdir * i * gridSize);
                int Y = (*bomb)->Position.y + (ydir * i * gridSize);
                struct dstr_block* mblock = (Block_Find((*blocks), X, Y, true, Player));
                bool destroyable = mblock != NULL && mblock->destroyable;
                bool test = false;

                if (is_on_block((*blocks), X, Y, &test, Player, *bombs, true) || mblock != NULL)
                {

                    if (mblock && destroyable) {
                        mblock->exists = false;
                        DestroyedBlocks++;
                        blocklastx = X;
                        blocklasty = Y;
                    }
                    break;
                }
                Explosion_Insert(explosions, X, Y, i);
                if (Player->enabled && X == Player->Transform.gridPosition.x && Y == Player->Transform.gridPosition.y)
                {
                    Player->enabled = false;
                }
                struct Enemy* elim = NULL;
                do {
                    elim = Enemy_FindAt(*Enemies, Player, X, Y, NULL, 1);
                    if (elim) {
                        EliminatedEnemies++;
                        Enemy_Remove(elim, Enemies);
                        lastx = X;
                        lasty = Y;
                    }
                } while (elim);
               
            }
            if (EliminatedEnemies > 1)
            {
                if (!hasAchievement(1))
                {
                    saveSystem_printAtLine(2, "1");
                    GlobalAction = 6;
                }
            }
        }
        if (EliminatedEnemies > 0)
        {
            if (Enemies_Count(*Enemies) <= 0)
            {
                int hmb = Blocks_Count(*blocks);
                if (hmb <= 0)
                {
                    CreateDoor(lastx, lasty);
                }
                else
                {
                    srand(time(NULL));
                    int random = rand() % (hmb);
                    struct dstr_block* cb = *blocks;
                    int i = 0;
                    while (cb)
                    {

                        if (cb->destroyable)
                        {
                            if (i == random)
                            {
                                cb->hasDoor = true;
                                break;
                            }
                            i++;
                        }
                        cb = cb->next;
                    }
                }
            }
        }
        if (DestroyedBlocks >= 2)
        {
            srand(time(NULL));
            int chance = rand() % 3;
            if (chance)
            {
                if (!hasAchievement(2))
                {
                    saveSystem_printAtLine(6, "1");
                    GlobalAction = 7;
                }
                unsigned short type = 1+ (rand() % 3);
                printf("Spawning Boost Type: %d\n", type);
                Boost_Add(Boosts, type, blocklastx, blocklasty);
            }
        }
    }
}
/**
 * @brief Funkcja wywolywana co klatke (petla) wykonujaca operacje dla wszystkich bomb.
 *
 * @param bombs Podwojny wskaznik na liste bomb.
 * @param blocks Podwojny wskaznik na liste blokow.
 * @param exploSound Dzwiek eksplozji (Wskaznik na sample)
 * @param samples Podwojny wskaznik na stos dzwiekow eksplozji.
 * @param dT deltaTime (czas miedzy klatkami).
 * @param Player Wskaznik na gracza.
 * @param cam_y_offset Wskaznik (float*) na offset kamery w osi Y.
 * @param explosions Podwojny wskaznik na liste eksplozji.
 * @param Enemies Podwojny wskaznik na liste przeciwnikow.
 * @param Boosts Podwojny wskaznik na liste ulepszen.
 */
void loopBombs(struct BombList** bombs, struct dstr_block** blocks, ALLEGRO_SAMPLE* exploSound, struct SampleStackElement** samples, float dT, struct Character* Player, float* cam_y_offset, struct Explosion** explosions, struct Enemy** Enemies, struct Boost** Boosts)
{
    if (bombs != NULL && *bombs)
    {
        struct BombList* bomb = *bombs;
        int i = 0;
        while (*bombs != NULL && bomb != NULL)
        {
            //printf("LOOP: %p, %lf\n", bomb, bomb->timeLeft);
            if (bomb->insidePlayer)
            {
                bomb->insidePlayer = Bomb_checkPlayer(bomb, Player);
            }
            bomb->timeLeft -= dT;
            if (bomb->timeLeft <= 0 && !bomb->exploded)
            {
                printf("BOMB (ID: %d) SHOULD NOW EXPLODE!\n", i);
                explodeBomb(&bomb, bombs, blocks, exploSound, samples, Player, cam_y_offset, explosions, Enemies, Boosts);
                //Bomb_Remove(&bomb);
                return;
            }
            else if (bomb->timeLeft <= 0 && bomb->exploded)
            {
                Bomb_Remove(&bomb,bombs, Player);
                return;
            }
            if (bomb->next != NULL) {
                bomb = bomb->next;
            }
            else break;
            i++;
        }
    }
}
