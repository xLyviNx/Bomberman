#pragma once
#include <stdbool.h>
#include <allegro5/allegro_audio.h>
#include "Enemies.h"
#ifndef WIDTH
    #define WIDTH 832
    #define HEIGHT 832
#endif // !WIDTH
struct dstr_block
{
    int gridX;
    int gridY;
    bool exists;
    bool destroyable;
    struct dstr_block* next;
    struct dstr_block* prev;
    bool hasDoor;
};

bool Block_Exists(struct dstr_block*, int, int, bool, struct Character*);
struct dstr_block* Block_Insert(struct dstr_block**, int, int, bool);
struct dstr_block* Block_createList(int, int, bool);
bool Block_RemoveList(struct dstr_block**);
struct dstr_block* Block_Find(struct dstr_block*, int, int, bool, struct Character*);
bool Block_Remove(struct dstr_block*, struct dstr_block**, struct Enemy*);
unsigned int Blocks_Count(struct dstr_block*);
void Blocks_draw(struct dstr_block*, ALLEGRO_BITMAP*, ALLEGRO_BITMAP*, struct Character*, float, float);
void loopBlocks(struct dstr_block**, struct Enemy*);
void Block_random(struct dstr_block*, int, int*, int*, int, struct Character*);
struct dstr_block* generate_blocks(int, struct Character*);
void GenerateStaticBlocks(struct dstr_block**, struct Character*);
