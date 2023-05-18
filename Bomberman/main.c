#include "algif5-master/src/algif.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/events.h>
#include <allegro5/keycodes.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <math.h>
#include <allegro5/allegro_acodec.h>
#include "soundstack.h"
#include "saveSystem.h"
#include "Enemies.h"

#ifndef WIDTH
    #define WIDTH 832
    #define HEIGHT 832
#endif // !WIDTH


#define exploTime 3.0;
const bool debug = true;
bool wasd[4] = { false, false, false, false };
double deltaTime = 0;
float cam_x_offset = 0;
float cam_y_offset = 0;
unsigned short GlobalAction = 0;
struct Vector2
{
    double x;
    double y;
};
struct Transform
{
    struct Vector2 position;
    struct Vector2 gridPosition;
    struct Vector2 scale;
};
struct Character
{
    struct Transform Transform;
    float Speed;
    ALGIF_ANIMATION* IdleAnim;
    ALGIF_ANIMATION* RightWalkAnim;
    ALGIF_ANIMATION* LeftWalkAnim;
    ALGIF_ANIMATION* UpWalkAnim;
    ALGIF_ANIMATION* DownWalkAnim;
    bool remoteBombs;
    unsigned short int bombRange;
    unsigned short int displayBombs;
    unsigned short int maxBombs;
    bool enabled;
    unsigned short walking;
};

struct Character* Player = NULL;
struct BombList
{
    float timeLeft;
    struct Vector2 Position;
    bool isRemote;
    bool exploded;
    bool insidePlayer;
    struct BombList* next;
    struct BombList* prev;
};
struct BombList* bombs = NULL;
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
struct Explosion
{
    float timeLeft;
    int gridX;
    int gridY;
    int i;
    struct Explosion* next;
    struct Explosion* prev;
};
struct Explosion* explosions = NULL;

struct Explosion* Explosion_Insert(struct Explosion** first, int X, int Y, int i)
{
    struct Explosion* nE = (struct Explosion*)malloc(sizeof(struct Explosion));
    if (nE != NULL)
    {
        nE->gridX = X;
        nE->gridY = Y;
        nE->next = NULL;
        nE->prev = NULL;
        nE->timeLeft = 0.2;
        nE->i = i;
        if ((*first) != NULL)
        {
            struct Explosion* end = *first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            end->next = nE;
            nE->prev = end;
        }
        else
        {
            (*first) = nE;
        }
        struct Explosion* t = (*first);
        int i = 0;
        while (t != NULL)
        {
            //printf("- %d: prev: %p,  next: %p\n", i, t->prev, t->next);
            t = t->next;
            i++;
        }

    }
    return nE;
}
bool Explosion_Remove(struct Explosion** exp)
{
    if (debug)
    {
        //printf("Deleting EXPLOSIOON: %p\n", (*exp));
    }
    if (exp != NULL && (*exp) != NULL)
    {
        struct Explosion* prev = (*exp)->prev;
        struct Explosion* next = (*exp)->next;
        if (next != NULL) {
            //printf("next: %d\n", next->gridX);
        }
        if (prev != NULL) {
            //printf("prev: %d\n", prev->gridX);
        }
        free((*exp));
        if (prev != NULL)
        {
            prev->next = next;
            if (next != NULL) {
                next->prev = prev;
            }
            (*exp) = prev;
        }
        else if (next != NULL)
        {
            next->prev = NULL;
            //printf("Swapping with %p\n", next);
            (*exp) = next;
        }
        else {
            (*exp) = NULL;
        }
        //printf("Now: %p\n", (*exp));
        return true;
    }
    return false;
}
bool Explosion_RemoveList(struct Explosion** explosions)
{
    while (*explosions != NULL)
    {
        struct Explosion* next = (*explosions)->next;
        free(*explosions);
        *explosions = next;
    }
    return *explosions == NULL;
}
struct dstr_block
{
    int gridX;
    int gridY;
    bool exists;
    bool destroyable;
    struct dstr_block* next;
    struct dstr_block* prev;
};
bool Block_Exists(struct dstr_block* first, int X, int Y, bool around)
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
    }
    return nB;
}
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
struct dstr_block* Block_Find(struct dstr_block* blocks, int X, int Y, bool gridMethod)
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
            else
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
bool Block_Remove(struct dstr_block** first, struct dstr_block** block)
{
    if (debug)
    {
        //printf("Deleting BLOCK: %p (FIRST IS: %p)\n", (*block), (*first));
    }
    if (block != NULL && (*block))
    {
        struct dstr_block* prev = (*block)->prev;
        struct dstr_block* next = (*block)->next;
        if (debug)
        {
            //printf("XY: %d, %d, PREV: %p, NEXT: %p\n", (*block)->gridX, (*block)->gridY, (*block)->prev, (*block)->next);
        }
        if ((*block) == (*first))
        {
            //printf("Is first.\n");
            next->prev = NULL;
            (*first) = next;
            //printf("Now Address: %p\n", (*first));
            free((*block));
            (*block) = NULL;
        }
        else if (prev != NULL)
        {
            prev->next = next;
            if (next != NULL) {
                next->prev = prev;
            }
            free((*block));
            (*block) = NULL;
        }
        return true;
    }
    return false;
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
            end = end->next;
            count++;
        }
    }
    return count;
}
unsigned int Blocks_Count(struct dstr_block* first)
{
    unsigned int count = 0;
    //printf("first: %p\n", first);
    if (first != NULL)
    {
        count = 1;
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
        } while (seek != NULL? seek->next != NULL : false);
    }
    return false;
}
struct BombList* Bomb_Find(struct BombList* first, int X, int Y, bool gridMethod)
{
    struct BombList* found = NULL;
    if (first != NULL)
    {
        struct BombList* bomb = bombs;
        while (bomb != NULL)
        {
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
bool Bomb_checkPlayer(struct BombList* bomb)
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
bool Bomb_Remove(struct BombList** bomb)
{
    if (debug)
    {
        printf("Deleting Bomb: %p\n", bomb);
    }
    if (bomb != NULL && (*bomb) != NULL)
    {
        struct BombList* prev = (*bomb)->prev;
        struct BombList* next = (*bomb)->next;
        //printf("PREV ADDRESS: %p\n", prev);
        bool any = false;
        if (prev != NULL)
        {
            prev->next = (*bomb)->next;
            any = true;
        }
        else
        {
            bombs = (*bomb)->next;
        }
        if (next != NULL)
        {
            next->prev = (*bomb)->prev;
            any = true;
        }
        if (!any)
        {
            bombs = NULL;
        }
        if (Player != NULL)
        {
            Player->displayBombs--;
        }
        return true;
    }
    return false;
}
bool is_on_block(struct dstr_block* blocks, float x, float y, float dX, float dY, bool* destroyable) {
  
    struct dstr_block* blck = Block_Find(blocks, x, y, false);
    struct BombList* atBomb = Bomb_Find(bombs, x, y, false);
    bool bombCont = false;
    if (atBomb)
    {
        bombCont = !atBomb->insidePlayer;
    }
    (*destroyable) = blck != NULL && blck->destroyable;
    return blck != NULL || bombCont;
}
void MovePlayer(struct Vector2 dir, struct dstr_block* blocks)
{
    if (Player != NULL)
    {
        struct Vector2 nDir;
        nDir.x = (dir.x * deltaTime) * 40;
        nDir.y = (dir.y * deltaTime) * 40;
        //if (dir.x != 0 && dir.y != 0) { nDir.y *= 0.7; nDir.x *= 0.7; }
        float npy = (Player->Transform.position.y + nDir.y);
        float npx = (Player->Transform.position.x + nDir.x);
        float halfplayer = ((Player->Transform.scale.x / 2.0) * 128);
        bool dstr = false;
        if (!is_on_block(blocks, npx, Player->Transform.position.y, dir.x, dir.y, &dstr)) {
            if (npx >= halfplayer && npx <= ((WIDTH*2)-halfplayer)+1)
            {
                if (npx < 0) npx = 0;
                if (cam_x_offset >= 0)
                {
                    if ((npx - cam_x_offset - 50 < (Player->Transform.scale.x / 2.0) * 128
                        && nDir.x < 0)
                        ||
                        (npx - cam_x_offset + 50 > WIDTH - ((Player->Transform.scale.x / 2.0) * 128)
                            && nDir.x > 0))
                    {
                        cam_x_offset += nDir.x * 5;
                    }
                }
                Player->Transform.position.x = npx;
            }
            //printf("offset: %lf!\n", cam_x_offset);
        }
        if (!is_on_block(blocks, Player->Transform.position.x, npy, 0, dir.y, &dstr)) {
            if (npy > (Player->Transform.scale.y / 2.0) * 128 && npy < HEIGHT + 1 - ((128 * Player->Transform.scale.y)/2.0))
            {
                Player->Transform.position.y = npy;
            }
        }
        //}
        int cellsizex = (int)(128 * Player->Transform.scale.x);
        int cellsizey = (int)(128 * Player->Transform.scale.y);
        float x = (int)(cellsizex * ((int)(Player->Transform.position.x / cellsizex))+cellsizex/2);
        float y = (int)(cellsizex * ((int)(Player->Transform.position.y / cellsizey)) + cellsizey / 2);
        /*float xdf = Player->Transform.position.x / cellsizex - (int)(floor(Player->Transform.position.x / cellsizex));
        float ydf = Player->Transform.position.y/cellsizey - (int)(floor(Player->Transform.position.y / cellsizey));
        if (xdf > 0.6 && dir.x > 0)
        {
            x += cellsizex;
        }
        else if (xdf < 0.4 && dir.x < 0)
        {
            x -= cellsizey;
        }
        if (ydf > 0.6 && dir.y > 0)
        {
            y += cellsizey;
        }
        else if (xdf < 0.4 && dir.y < 0)
        {
            y -= cellsizey;
        }*/
        //printf("ydf: %lf, %lf\n", ydf, dir.y);
        if (cam_x_offset > WIDTH)
        {
            cam_x_offset = WIDTH;
        }
        Player->Transform.gridPosition.x = x;
        Player->Transform.gridPosition.y = y;
        if (cam_x_offset < 0) cam_x_offset = 0;
        //printf("x: %lf\n", Player->Transform.position.x);
        //printf("%lf, %lf\n", Player->Transform.position.x, Player->Transform.position.y);
    }
}
void drawAllFilledRectInView() {
    int gridSize = (int)(128 * Player->Transform.scale.x); // rozmiar kratki
    float rectX, rectY;
    int i, j;
    for (i = gridSize; i < cam_x_offset + WIDTH + gridSize; i += gridSize * 2) {
        for (j = 0; j < HEIGHT; j += gridSize * 2) {
            if ((i / gridSize + j / gridSize) % 2 == 1) {
                rectX = i - cam_x_offset;
                rectY = j - gridSize;
                al_draw_filled_rectangle(rectX, rectY - cam_y_offset, rectX + gridSize, rectY - cam_y_offset + gridSize, al_map_rgb(150, 150, 150));
            }
        }
    }
}
void GenerateStaticBlocks(struct dstr_block** first)
{
    int gridSize = (int)(128 * Player->Transform.scale.x); // rozmiar kratki
    for (int i = 2; i < (WIDTH*2 / gridSize)+1; i += 2)
    {
        for (int j = 2; j < (HEIGHT / gridSize); j += 2)
        {
            Block_Insert(first, (i * gridSize) - gridSize / 2, (j * gridSize) - gridSize / 2, true);
        }
    }
}
void Blocks_draw(struct dstr_block* first, ALLEGRO_BITMAP* dBS, ALLEGRO_BITMAP* sBS)
{
    struct dstr_block* temp = first;
    int gridSize = (int)(128 * Player->Transform.scale.x); // rozmiar kratki
    while (temp != NULL)
    {
        if (temp != NULL) {
            //printf("NOT NULL %p\n", temp);
            if (temp == NULL)
            {
                break;
            }
            //printf("aaaa: %p\n", temp);
            if (temp->exists) {
                if (temp->gridX - cam_x_offset - gridSize / 2 < WIDTH && temp->gridX - cam_x_offset + gridSize / 2 > 0)
                {
                    if (temp->gridY < HEIGHT && temp->gridY > 0)
                    {
                        if (temp->destroyable) {
                            //al_draw_filled_rectangle(temp->gridX - cam_x_offset - gridSize / 2, temp->gridY - cam_y_offset - gridSize / 2, temp->gridX - cam_x_offset + gridSize / 2, temp->gridY - cam_y_offset + gridSize / 2, al_map_rgb(100, 100, 100));
                            al_draw_scaled_bitmap(dBS, 0, 0, 128, 128, temp->gridX - gridSize /2 - cam_x_offset, temp->gridY - cam_y_offset - gridSize / 2, gridSize, gridSize, 0);
                        }
                        else
                        {
                            //al_draw_filled_rectangle(temp->gridX - cam_x_offset - gridSize / 2, temp->gridY - cam_y_offset - gridSize / 2, temp->gridX - cam_x_offset + gridSize / 2, temp->gridY - cam_y_offset + gridSize / 2, al_map_rgb(160, 160, 160));
                            al_draw_scaled_bitmap(sBS, 0, 0, 128, 128, temp->gridX - cam_x_offset - gridSize / 2, temp->gridY - cam_y_offset - gridSize / 2, gridSize, gridSize, 0);

                        }
                    }
                }
            }
            temp = temp->next;
        }
        else
        {
            printf("TEMP = NULL\n");
        }
    }
}
void playerMovement(struct dstr_block* blocks)
{
    struct Vector2 dir;
    dir.x = 0;
    dir.y = 0;
    if (wasd[0] && !wasd[2])
    {
        dir.y = -1;
        Player->walking = 2;
    }
    else if (!wasd[0] && wasd[2])
    {
        dir.y = 1;
        Player->walking = 1;
    }
    if (wasd[1] && !wasd[3])
    {
        dir.x = -1;
        Player->walking = 4;

    }
    else if (!wasd[1] && wasd[3])
    {
        dir.x = 1;
        Player->walking = 3;

    }
    if (!dir.x && !dir.y)
        Player->walking=0;
    dir.x *= Player->Speed;
    dir.y *= Player->Speed;
    MovePlayer(dir, blocks);
}
void plantBomb(bool isPaused)
{
    if (Player != NULL && !isPaused)
    {
        struct Vector2 pos;
        pos = Player->Transform.gridPosition;
        bool canplant = false;
        struct BombList* planted = NULL;
        if (Player->enabled)
        {
            unsigned int count = Bomb_count(bombs);
            printf("Bombs BEFORE: %d\n", count);
            if (count > 0)
            {
                if (count < Player->maxBombs && !Bomb_ExistsAt(bombs, pos))
                {

                    canplant = true;
                }
            }
            else
            {
                planted = Bomb_CreateList(pos, Player->remoteBombs);
                bombs = planted;
                printf("Bombs 0, creating a list. Address: %p\n   prev: %p\n   next: %p\n", bombs, bombs->prev, bombs->next);
            }
        }
        if (canplant)
        {
            planted = Bomb_InsertInto(bombs, pos, Player->remoteBombs);
        }

        if (planted == NULL)
        {
            printf("Bomb NOT planted.\n");
        }
        else {
            printf("Bomb planted.\n");
            Player->displayBombs++;
        }
    }
}
void renderBombs(ALGIF_ANIMATION* bombAnim, double animtime)
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

void renderExplosions(struct Explosion* expl)
{
    if (expl != NULL)
    {
        struct Explosion* exp = expl;
        int i = 0;
        while (expl != NULL && exp != NULL)
        {
            if (exp == NULL) {
                break;
            }
            
            //printf("Address Now: %p\n", exp);
            //float fac = (1 - (exp->i / Player->bombRange));
            //printf("FAC: %lf\n", fac);
            float size = 128 * exp->timeLeft;
            // printf("size: %lf\n", size);
            al_draw_filled_circle(exp->gridX-cam_x_offset, exp->gridY-cam_y_offset, size, al_map_rgb(180, 140, 0));
            if (exp != NULL) {
                exp = exp->next;
                
            }
            else break;
        }
    }
}
void explodeBomb(struct BombList** bomb, struct dstr_block** blocks, ALLEGRO_SAMPLE* exploSound, struct SampleStackElement** samples)
{
    if (bomb != NULL && (*bomb) != NULL)
    {
        (*bomb)->exploded = true;
        (*bomb)->timeLeft = 0.05;
        int gridSize = (int)(128 * Player->Transform.scale.x);
        int xdir = 0;
        int ydir = 0;
        cam_y_offset += 30;
        ALLEGRO_SAMPLE_INSTANCE* newExploSound = al_create_sample_instance(exploSound);
        al_attach_sample_instance_to_mixer(newExploSound, al_get_default_mixer());
        al_play_sample_instance(newExploSound);
        *samples = SampleStack_Push(*samples, newExploSound);
        if ((*bomb)->Position.x == Player->Transform.gridPosition.x && (*bomb)->Position.y == Player->Transform.gridPosition.y)
        {
            Player->enabled = false;
        }
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
                struct dstr_block* mblock = (Block_Find((*blocks), X, Y, true));

                bool destroyable = mblock != NULL && mblock->destroyable;
                bool test = false;

                if (is_on_block((*blocks), X, Y, 0, 0, &test) || mblock != NULL)
                {
                    if (mblock && destroyable) {
                        mblock->exists = false;
                    }
                    break;
                }
                Explosion_Insert(&explosions, X, Y, i);
                if (Player->enabled && X == Player->Transform.gridPosition.x && Y == Player->Transform.gridPosition.y)
                {
                    Player->enabled = false;
                }

            }
        }
    }
}
void loopExplosions(struct Explosion** expl, bool* removed)
{
    if (expl != NULL)
    {
        struct Explosion* exp = (*expl);
        while (exp != NULL)
        {
            exp->timeLeft -= deltaTime;
            if (exp->timeLeft <= 0)
            {
                (*removed) = Explosion_Remove(&exp);
                //printf("Removed %p ??? - %d\n", exp, *removed);
                (*expl) = exp; // update the value of exp

                return;
            }
            if (exp->next != NULL) {
                exp = exp->next;
            }
            else break;
        }
    }
}
void loopBombs(struct dstr_block** blocks, ALLEGRO_SAMPLE* exploSound, struct SampleStackElement** samples)
{
    if (bombs != NULL)
    {
        struct BombList* bomb = bombs;
        int i = 0;
        while (bombs != NULL && bomb != NULL)
        {
            if (bomb->insidePlayer)
            {
                bomb->insidePlayer = Bomb_checkPlayer(bomb);
            }
            bomb->timeLeft -= deltaTime;
            if (bomb->timeLeft <= 0 && !bomb->exploded)
            {
                printf("BOMB (ID: %d) SHOULD NOW EXPLODE!\n", i);
                explodeBomb(&bomb, blocks, exploSound, samples);
                //Bomb_Remove(&bomb);
                return;
            }
            else if (bomb->timeLeft <= 0 && bomb->exploded)
            {
                Bomb_Remove(&bomb);
            }
            if (bomb->next != NULL) {
                bomb = bomb->next;
            }
            else break;
            i++;
        }
    }
}
void loopBlocks(struct dstr_block** blocks)
{
    if (blocks != NULL)
    {
        struct dstr_block* bl = (*blocks);
        int i = 0;
        while (bl != NULL)
        {
            i++;
            //printf("JA PIERDOLE! %p, %p, %p\n", bl, bl->prev, bl->prev);
            //printf("BL - %p\n", bl);
            if (!bl->exists)
            {
                if (Block_Remove(blocks, &bl))
                {
                    (*blocks) = (*blocks);
                }
                return;
            }
            if (bl ->next != NULL) {
                bl = bl->next;
            }
            else break;
        }
        //printf("BLOCKS: %d\n", i);
    }
}
void menuMove(bool down, unsigned short* val, int level, bool buttondown)
{
    if (buttondown)
    {
        if (level == 0)
        {
            *val += down ? 1 : -1;
            if (*val < 1)
            {
                *val = 1;
            }
            else if (*val > 3)
            {
                *val = 3;
            }
        }
        else
        {
            *val = 0;
        }
    }
}
void button(int key, bool down, int *currentLevel, unsigned short* menuChoiceValue)
{
    switch (key)
    {
    case ALLEGRO_KEY_W:
    {
        wasd[0] = down;
        menuMove(false, menuChoiceValue, *currentLevel, down);
        break;
    }
    case ALLEGRO_KEY_A:
    {
        wasd[1] = down;
        break;
    }
    case ALLEGRO_KEY_S:
    {
        wasd[2] = down;
        menuMove(true, menuChoiceValue, *currentLevel, down);
        break;
    }
    case ALLEGRO_KEY_D:
    {
        wasd[3] = down;
        break;
    }
    case ALLEGRO_KEY_SPACE:
    {
        if (down)
        {
            if (*currentLevel)
            {
                GlobalAction = 5;
            }
        }
        break;
    }
    case ALLEGRO_KEY_UP:
    {
        menuMove(false, menuChoiceValue, *currentLevel, down);
        break;
    }
    case ALLEGRO_KEY_DOWN:
    {
        menuMove(true, menuChoiceValue, *currentLevel, down);
        break;
    }
    case ALLEGRO_KEY_ENTER:
    {
        if (down)
        {
            if (!*currentLevel)
            {
                switch (*menuChoiceValue)
                {
                case 1:
                {
                    saveSystem_resetData();
                    GlobalAction = 1;
                    break;
                }
                case 2:
                {
                    GlobalAction = 1;
                    break;
                }
                case 3:
                {
                    GlobalAction = 2;
                    break;
                }
                }
            }
            else
            {
                GlobalAction = 4;
            }
        }
        break;
    }
    case ALLEGRO_KEY_ESCAPE:
    {
        if (down) {
            GlobalAction = 3;
        }
        break;
    }
    default:
    {
        break;
    }
    }
}
void drawUI(ALLEGRO_FONT** uiFont)
{
    al_draw_filled_rectangle(5, 5, WIDTH - 5, 55, al_map_rgba(0, 0, 0, 150));

    al_draw_textf(*uiFont, al_map_rgba(255, 255, 255, 80), WIDTH - 15, 15, ALLEGRO_ALIGN_RIGHT, "BOMBS: %d/%d", Player->displayBombs, Player->maxBombs);
}
void drawGrid(int maxx, int maxy, int xsize, int ysize)
{
    if (Player != NULL) {
        int i, j;
        float x_offset = fmod(cam_x_offset, WIDTH);
        int window_ext = (int)(cam_x_offset / WIDTH);

        for (i = -window_ext; i < maxx + 1; i++)
        {
            float x1 = (i * xsize) - xsize - x_offset;
            float x2 = (i * xsize) + xsize - x_offset;

            if (x1 > WIDTH) {
                x1 -= WIDTH;
                x2 -= WIDTH;
            }
            else if (x2 < 0) {
                x1 += WIDTH;
                x2 += WIDTH;
            }

            for (j = 0; j < maxy; j++)
            {
                al_draw_rectangle(x1+xsize, (j * ysize) - ysize, x2 + xsize, (j * ysize) + ysize, al_map_rgba(255, 180, 30, 1), 1.5);
            }
        }
    }
}
bool zamknij = false;
const int FPS = 30;

void Block_random(struct dstr_block* first, int level, int* X, int* Y, int i)
{
    unsigned int seed = (unsigned int)(level * 10000 + i * 100 + level * i);
    srand(seed);
    int gridSize = (int)(128 * Player->Transform.scale.x);
    int rX = 0;
    int rY = 0;
    do
    {
        rX = (((rand() % (WIDTH / gridSize))* 2)+1);
        rY = (((rand() % (HEIGHT / gridSize)) +1));
        *X = (rX * gridSize) - gridSize / 2;
        *Y = (rY * gridSize) - gridSize / 2;
        //*Y += 1;
    } while (Block_Exists(first, *X, *Y, true) || (*X==(int)Player->Transform.position.x && *Y == (int)Player->Transform.position.y));
    
    //printf("%d - %d, %d - %d\n", rX, *X, rY, *Y);
}
struct dstr_block* generate_blocks(int level)
{
    struct dstr_block* Blocks = NULL;
    int limit = 20 + (level*11);
    if (limit > 75)
    {
        limit = 75;
    }
    for (int i = 0; i < limit; i++)
    {
        
        int X = 0;
        int Y = 0;
        Block_random(Blocks, level, &X, &Y, i);
        struct dstr_block* bl = NULL;
        if (Blocks == NULL)
        {
            bl = Block_createList(X, Y, false);
        }
        else
        {
            bl = Block_Insert(&Blocks, X, Y, false);
        }
        //printf("count: %d\n", Blocks_Count(Blocks));
        if (bl != NULL)
        {
            bl->next = NULL;
            if (Blocks == NULL) {
                Blocks = bl;
            }
        }
        //printf("BL3: %d\n", i);

    }
    GenerateStaticBlocks(&Blocks);
    return Blocks;
}
int main()
{
    Player = (struct Character*)malloc(sizeof(struct Character));
    if (Player != NULL)
    {
        if (!al_init()) {
            fprintf(stderr, "Failed to initialize allegro.\n");
            return -2;
        }
        if (!al_init_font_addon())
        {
            return -1;
        }
        Player->Transform.scale.x = 0.5;
        Player->Transform.scale.y = 0.5;
        Player->Transform.position.x = WIDTH / 2;
        Player->Transform.position.y = HEIGHT / 2;
        Player->Speed = 3.0;
        Player->displayBombs = 0;
        Player->maxBombs = 2;
        Player->remoteBombs = false;
        Player->bombRange = 1;
        Player->enabled = true;

        ALLEGRO_DISPLAY* display = NULL;
        ALLEGRO_EVENT_QUEUE* queue = NULL;
        ALLEGRO_TIMER* timer = NULL;
        // Initialize allegro

        if (!al_init_primitives_addon()) { return -1; }
        if (!al_install_keyboard()) { return -1; }
        if (!al_init_ttf_addon()) { return -1; }
        if (!al_init_image_addon()) { return -1; }
        if (!al_install_audio()) { return -12; }
        al_init_acodec_addon();
        if (!al_reserve_samples(3)) { return -13; }
        ALLEGRO_SAMPLE* explosionSound = al_load_sample("data/sounds/explosion.ogg");
        ALLEGRO_SAMPLE* MenuMusic = al_load_sample("data/sounds/bombermenu.ogg");
        ALLEGRO_SAMPLE* GameMusic = al_load_sample("data/sounds/bombergameplay.ogg");
        if (!MenuMusic || !GameMusic)
        {
            printf("MUSIC LOADING FAILED! %p and %p\n", MenuMusic, GameMusic);
            return -11;
        }
        // Initialize the timer
        timer = al_create_timer(1.0 / FPS);
        if (!timer) {
            fprintf(stderr, "Failed to create timer.\n");
            return 1;
        }

        // Create the display
        display = al_create_display(WIDTH, HEIGHT);
        if (!display) {
            fprintf(stderr, "Failed to create display.\n");
            return 1;
        }
        //al_set_display_option(display, ALLEGRO_VSYNC, 1);
        // Create the event queue
        queue = al_create_event_queue();
        if (!queue) {
            fprintf(stderr, "Failed to create event queue.");
            return 1;
        }
        // Register event sources
        al_register_event_source(queue, al_get_keyboard_event_source());
        al_register_event_source(queue, al_get_timer_event_source(timer));

        // Display a black screen
        al_clear_to_color(al_map_rgb(0, 30, 0));
        al_flip_display();

        // Start the timer
        al_start_timer(timer);
        ALLEGRO_FONT* debugFont = al_load_ttf_font("data/fonts/data-unifon.ttf", 12, 0);
        ALLEGRO_FONT* uiFont = al_load_ttf_font("data/fonts/PlatNomor-WyVnn.ttf", 18, 0);
        ALLEGRO_FONT* TitleFont = al_load_ttf_font("data/fonts/AldotheApache.ttf", 100, 0);
        ALLEGRO_FONT* MenuButton = al_load_ttf_font("data/fonts/Exo-Regular.ttf", 30, 0);
        if (debugFont == NULL || uiFont == NULL || !TitleFont)
        {
            printf("Blad odczytu czcionki. \n");
            return -3;
        }
        //Player->sprite = al_load_bitmap("data/Player.bmp");
        Player->IdleAnim = algif_load_animation("data/gifs/player/idle.gif");
        Player->RightWalkAnim = algif_load_animation("data/gifs/player/right.gif");
        Player->LeftWalkAnim = algif_load_animation("data/gifs/player/left.gif");
        Player->UpWalkAnim = algif_load_animation("data/gifs/player/up.gif");
        Player->DownWalkAnim = algif_load_animation("data/gifs/player/down.gif");
        ALGIF_ANIMATION* Enemy1_animation = algif_load_animation("data/gifs/enemies/enemy1.gif");
        ALGIF_ANIMATION* Enemy2_animation = algif_load_animation("data/gifs/enemies/enemy2.gif");
        if (!Player->IdleAnim || !Player->RightWalkAnim || !Player->LeftWalkAnim || !Player->UpWalkAnim || !Player->DownWalkAnim)
        {
            printf("Nie udalo sie wczytac animacji gracza.\n");
            return -1;
        }
        if (!Enemy1_animation || !Enemy2_animation)
        {
            printf("Nie udalo sie wczytac animacji przeciwnikow.\n");
            return -1;
        }
        ALGIF_ANIMATION* BombAnim = algif_load_animation("data/gifs/bomb.gif");
        if (BombAnim == NULL)
        {
            printf("Nie uda�o si� wczyta� pliku gif bomby\n%d\n", al_get_errno());
            return -1;
        }

        ALLEGRO_BITMAP* dBlockSprite = al_load_bitmap("data/dBlock.bmp");
        ALLEGRO_BITMAP* sBlockSprite = al_load_bitmap("data/sBlock.bmp");
        if (!dBlockSprite || !sBlockSprite)
            return -15;

        double time = al_get_time();
        double oldTime = time;
        float displayFps = 0;
        float displayFpsDelay = 0;
        int frames = 0;
        double syncDeltaTime = 0;


        int xsize = (int)(128 * Player->Transform.scale.x);
        int ysize = (int)(128 * Player->Transform.scale.y);
        int maxx = (int)(WIDTH / (xsize ));
        int maxy = (int)(WIDTH / (ysize ));
        int level = 0;
        cam_y_offset = 0;
        struct dstr_block* blocks = NULL;
        float dTest = 0;
        bool gridEnabled = false;
        unsigned short MenuChoice = 0;
        if (!saveSystem_init()) return -10;
        int loadedLevel = saveSystem_LoadLevel();
        bool Pause = false;
        double AnimTime = al_get_time();
        ALLEGRO_SAMPLE_INSTANCE* MMusicInstance = al_create_sample_instance(MenuMusic);
        //ALLEGRO_SAMPLE_INSTANCE* explosionsInstance = al_create_sample_instance(explosionSound);
        ALLEGRO_SAMPLE_INSTANCE* GMusicInstance = al_create_sample_instance(GameMusic);
        al_set_sample_instance_playmode(MMusicInstance, ALLEGRO_PLAYMODE_LOOP);
        al_attach_sample_instance_to_mixer(MMusicInstance, al_get_default_mixer());
        al_set_sample_instance_playmode(GMusicInstance, ALLEGRO_PLAYMODE_LOOP);
        al_attach_sample_instance_to_mixer(GMusicInstance, al_get_default_mixer());
        struct Enemy* EnemyList = NULL;
        struct SampleStackElement* samples = NULL;
        //  int Lifes = 3;
        while (!zamknij)
        {
            if (GlobalAction == 2)
            {
                zamknij = true;
                return;
            }
            //printf("LEVEL: %d\n", level);
            //printf("======== New Frame ========\n");
            //ALLEGRO_TIMEOUT timeout;
            //al_init_timeout(&timeout, 0.06);
            //al_wait_for_vsync();
            ALLEGRO_EVENT event;
            al_wait_for_event_timed(queue, &event, 0.00);
            time = al_get_time();
            deltaTime = time - oldTime;
            syncDeltaTime += deltaTime;
            oldTime = time;
            frames++;
            
            switch (event.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                zamknij = true;
                return 0;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                button(event.keyboard.keycode, true, &level, &MenuChoice);
                break;
            }
            case ALLEGRO_EVENT_KEY_UP:
            {
                button(event.keyboard.keycode, false, &level, &MenuChoice);
                break;
            }
            }
            if (level != 0) {

                if (!al_get_sample_instance_playing(GMusicInstance))
                {
                    al_play_sample_instance(GMusicInstance);
                    //printf("Play GAME MUSIC?\n");
                }
                switch (GlobalAction)
                {
                    case 3:
                    {
                        Pause = !Pause;
                        GlobalAction = 0;
                        break;
                    }
                    case 4:
                    {
                        if (Pause || !Player->enabled)
                        {
                            Bomb_RemoveList(&bombs);
                            Block_RemoveList(&blocks);
                            Explosion_RemoveList(&explosions);
                            al_stop_sample_instance(GMusicInstance);
                            Enemies_Clear(&EnemyList);
                            //EnemyList = NULL;
                            Pause = false;
                            level = 0;
                            GlobalAction = 0;
                            continue;

                        }
                        GlobalAction = 0;
                        break;
                    }
                    case 5:
                    {
                        plantBomb(Pause);
                        GlobalAction = 0;
                        break;
                    }
                    default:
                    {
                        GlobalAction = 0;
                        break;
                    }
                }

                if (!blocks)
                {
                    blocks = generate_blocks(level);
                }
                if (!Pause) 
                {
                    AnimTime = al_get_time();
                    if (cam_y_offset > 0)
                    {
                        cam_y_offset -= deltaTime * 100;
                    }
                }
                if (cam_y_offset < 0) { cam_y_offset = 0; }
                al_clear_to_color(al_map_rgb(10, 100, 10));
                //drawAllFilledRectInView();
                bool check = false;
                //dTest += deltaTime;
                //if (dTest > 1) {
                if (!Pause)
                {
                    loopBlocks(&blocks);
                }
                //dTest = 0;
            //}
                Enemies_Loop(EnemyList, AnimTime, cam_x_offset, cam_y_offset);
                if (!check) {
                    Blocks_draw(blocks, dBlockSprite, sBlockSprite);
                    //printf("BLOCKS ADDRESS: %p\n", blocks);
                }
                if (Player->enabled)
                {
                    if (!Pause)
                    {
                        if (!check) {
                            playerMovement(blocks);
                        }
                    }
                    ALGIF_ANIMATION* anim = NULL;
                    switch (Player->walking)
                    {
                    case 1:
                    {
                        anim = Player->DownWalkAnim;
                        break;
                    }
                    case 2:
                    {
                        anim = Player->UpWalkAnim;
                        break;
                    }
                    case 3:
                    {
                        anim = Player->RightWalkAnim;
                        break;
                    }
                    case 4:
                    {
                        anim = Player->LeftWalkAnim;
                        break;
                    }
                    default:
                    {
                        anim = Player->IdleAnim;
                        break;
                    }
                    }
                    if (anim) {
                        ALLEGRO_BITMAP* sprite = algif_get_bitmap(anim, AnimTime);
                        //al_draw_filled_rectangle(Player->Transform.position.x - Player->Transform.scale.x / 2.0, Player->Transform.position.y - Player->Transform.scale.y / 2.0, Player->Transform.position.x + Player->Transform.scale.x, Player->Transform.position.y + Player->Transform.scale.y, color_blue);
                        al_draw_scaled_rotated_bitmap(sprite, 64, 64, Player->Transform.position.x - cam_x_offset, Player->Transform.position.y - cam_y_offset, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                        //al_draw_scaled_rotated_bitmap(Player->sprite, 64, 64, Player->Transform.gridPosition.x, Player->Transform.gridPosition.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                        al_draw_rounded_rectangle((Player->Transform.gridPosition.x - (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y - cam_y_offset - (128 * Player->Transform.scale.y) / 2), (Player->Transform.gridPosition.x + (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y - cam_y_offset + (128 * Player->Transform.scale.y) / 2), 20, 20, al_map_rgba(150, 20, 20, 3), 2);
                    }
                }
                if (gridEnabled)
                {
                    drawGrid(maxx, maxy, xsize, ysize);
                }

                drawUI(&uiFont);
                if (debug)
                {
                    if (displayFpsDelay <= 0)
                    {
                        //printf("Synced Delta Time: %lf. FRAMES: %d\n", syncDeltaTime, frames);
                        displayFps = frames / syncDeltaTime;
                        displayFpsDelay = 0.2;
                        syncDeltaTime = 0;
                        frames = 0;
                    }
                    else
                    {
                        displayFpsDelay -= deltaTime;
                    }
                    al_draw_textf(debugFont, al_map_rgba(0, 255, 0, 150), 10, 10, ALLEGRO_ALIGN_LEFT, "fps: %.2lf", displayFps);
                    al_draw_textf(debugFont, al_map_rgba(100, 255, 0, 150), 10, 30, ALLEGRO_ALIGN_LEFT, "/\\t: %lf", deltaTime);
                }
                check = false;
                if (!Pause)
                {
                    loopExplosions(&explosions, &check);

                    if (!check)
                    {
                        renderExplosions(explosions);
                    }
                    loopBombs(&blocks, explosionSound, &samples);
                }
                renderBombs(BombAnim, AnimTime);
                if (!Player->enabled)
                {
                    al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(0, 0, 0, 180));
                    al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH / 2, HEIGHT / 2 - 150, ALLEGRO_ALIGN_CENTER, "GAME OVER");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT / 2 - 50, ALLEGRO_ALIGN_CENTER, "Kliknij aby wyjsc do menu...");
                }
                if (Pause)
                {
                    al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(0,0,0,180));
                    al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH / 2, HEIGHT/2 - 150, ALLEGRO_ALIGN_CENTER, "PAUZA");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT/2 - 50, ALLEGRO_ALIGN_CENTER, "Kliknij ESC aby wrocic do gry.");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT/2 - 25, ALLEGRO_ALIGN_CENTER, "Kliknij ENTER aby wyjsc do menu.");
                }
            }
            else
            {

                if (!al_get_sample_instance_playing(MMusicInstance))
                {
                    al_play_sample_instance(MMusicInstance);
                    //printf("Play Menu?\n");
                }
                if (MenuChoice == 0)
                    MenuChoice = 1;
                al_clear_to_color(al_map_rgb(10, 30, 20));
                al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, 30, ALLEGRO_ALIGN_CENTER, "Bomberman");
                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, HEIGHT-20, ALLEGRO_ALIGN_CENTER, "Sygut Grzegorz, Strzepek Piotr, Szylinski Krzysztof, Synowiec Adrian");
                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, HEIGHT-40, ALLEGRO_ALIGN_CENTER, "PP2 Projekt (1ID14B SEM2 2023)");
                ALLEGRO_COLOR butColor = al_map_rgb(80, 80, 80);
                ALLEGRO_COLOR butColor2 = butColor;
                ALLEGRO_COLOR butColor3 = butColor;
                if (MenuChoice==1)
                    butColor = al_map_rgb(255, 255, 255);
                else if (MenuChoice==2)
                    butColor2 = al_map_rgb(255, 255, 255);
                else if (MenuChoice == 3)
                    butColor3 = al_map_rgb(255, 255, 255);
                al_draw_textf(MenuButton, butColor, WIDTH / 2, 300, ALLEGRO_ALIGN_CENTER, "Nowa Gra");
                al_draw_textf(MenuButton, butColor2, WIDTH / 2, 350, ALLEGRO_ALIGN_CENTER, "Kontynuuj (Level %d)",loadedLevel);
                al_draw_textf(MenuButton, butColor3, WIDTH / 2, 400, ALLEGRO_ALIGN_CENTER, "Wyjdz");

                switch (GlobalAction)
                {
                    case 1:
                    {
                        loadedLevel = saveSystem_LoadLevel();

                        Player->Transform.position.x = WIDTH / 2;
                        Player->Transform.position.y = HEIGHT / 2;
                        al_stop_sample_instance(MMusicInstance);
                        level = loadedLevel;
                        Enemy_Add(&EnemyList, WIDTH/2, HEIGHT/2, Enemy1_animation);
                        Enemy_Add(&EnemyList, WIDTH/2+128, HEIGHT/2, Enemy2_animation);
                        Player->enabled = true;
                        GlobalAction = 0;
                        break;
                    }
                    default:
                        { break; }
                }

            }
            SampleStack_Loop(&samples, deltaTime);
            al_flip_display();

        }
        free(Player);
        al_uninstall_audio();
        al_destroy_sample(MenuMusic);
        al_destroy_sample(GameMusic);
        saveSystem_close();
        al_uninstall_keyboard();
        al_destroy_display(display);
        al_shutdown_primitives_addon();
    }
    return 0;
}
