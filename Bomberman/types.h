#pragma once
#include "algif5-master/src/algif.h"
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
bool is_on_block(struct dstr_block*, float, float, float, float, bool*, struct Character*, struct BombList*, bool);
