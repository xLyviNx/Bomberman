#include "types.h"
#include "Bombs.h"
#include "Blocks.h"
bool is_on_block(struct dstr_block* blocks, float x, float y, float dX, float dY, bool* destroyable, struct Character* Player, struct BombList* bombs, bool bombanyway) {

    struct dstr_block* blck = Block_Find(blocks, x, y, false, Player);
    struct BombList* atBomb = Bomb_Find(bombs, x, y, false, Player);
    bool bombCont = false;
    if (atBomb)
    {
        bombCont = !atBomb->insidePlayer;
    }
    if (bombanyway && atBomb)
        bombCont = true;
    (*destroyable) = blck != NULL && blck->destroyable;
    return blck != NULL || bombCont;
}