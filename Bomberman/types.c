#include "types.h"
#include "Bombs.h"
#include "Blocks.h"

/**
 * @brief Sprawdza, czy na danych koordynatach miesci sie blok lub bomba.
 *
 * @param blocks Wskaznik na pierwszy element listy blokow.
 * @param x Wspolrzedna x.
 * @param y Wspolrzedna y.
 * @param destroyable Wskaznik na bool'a mowiacego czy blok da sie zniszczyc.
 * @param Player Wskaznik na gracza.
 * @param bombs Wskaznik na liste bomb.
 * @param bombanyway Bezwzgledne sprawdzenie czy na koordynatach miesci sie bomba (uzywane do pozwolenia lub nie graczowi poruszac sie w srodku bomby).
 * @param subPixels Zmiana wielkosci wykrywania kolizji (w pikselach).
 */
bool is_on_block(struct dstr_block* blocks, float x, float y, bool* destroyable, struct Character* Player, struct BombList* bombs, bool bombanyway, int subPixels) {

    struct dstr_block* blck = Block_Find(blocks, x, y, false, Player, subPixels);
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