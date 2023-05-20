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
#define exploTime 3.0;


struct BombList* Bomb_CreateList(struct Vector2 pos, bool remote)
{
    struct BombList* nB = (struct BombList*)malloc(sizeof(struct BombList));
    if (nB != NULL)
    {
        nB->Position = pos;
        nB->isRemote = remote;
        nB->prev = NULL;
        nB->exploded = false;
        nB->insidePlayer = true;
        nB->next = NULL;
        nB->timeLeft = exploTime;
    }
    return nB;
}
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
struct BombList* Bomb_InsertInto(struct BombList* first, struct Vector2 pos, bool remote)
{
    struct BombList* nB = NULL;
    if (first != NULL)
    {
        nB = (struct BombList*)malloc(sizeof(struct BombList));
        if (nB != NULL)
        {
            nB->Position = pos;
            nB->isRemote = remote;
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
                if (count < Player->maxBombs && !is_on_block(blocks, Player->Transform.gridPosition.x, Player->Transform.gridPosition.y, Player->Transform.gridPosition.x, Player->Transform.gridPosition.y, &dst, Player, *bombs, true))
                {

                    canplant = true;
                }
            }
            else
            {
                planted = Bomb_CreateList(pos, Player->remoteBombs);
                *bombs = planted;
                printf("Bombs 0, creating a list. Address: %p\n   prev: %p\n   next: %p\n", *bombs, (*bombs)->prev, (*bombs)->next);
            }
        }
        if (canplant)
        {
            planted = Bomb_InsertInto(*bombs, pos, Player->remoteBombs);
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
        //al_destroy_sample_instance(newExploSound);
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

                if (is_on_block((*blocks), X, Y, 0, 0, &test, Player, *bombs, true) || mblock != NULL)
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
            printf("CHANCE: %d\n", chance);
            if (chance)
            {
                if (!hasAchievement(2))
                {
                    saveSystem_printAtLine(6, "1");
                    GlobalAction = 7;
                }
                unsigned short type = 1+ (rand() % 3);
                printf("Type: %d\n", type);
                Boost_Add(Boosts, type, blocklastx, blocklasty);
            }
        }
    }
}
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
