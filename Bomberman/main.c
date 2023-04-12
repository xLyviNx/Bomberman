#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/events.h>
#include <allegro5/keycodes.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>

#define exploTime 3.0;
const int width = 832;
const int height = 832;
const bool debug = true;
bool wasd[4] = { false, false, false, false };
double deltaTime = 0;
float cam_x_offset = 0;
struct Vector2
{
    double x;
    double y;
};

struct BombList
{
    float timeLeft;
    struct Vector2 Position;
    bool isRemote;
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
        nB->next = NULL;
        nB->timeLeft = exploTime;
    }
    return nB;
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
            struct BombList* end = first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            printf("Added at the end of %p\n", end);
            nB->prev = end;
            nB->next = NULL;
            end->next = nB;
        }
    }
    return nB;
}
struct dstr_block
{
    int gridX;
    int gridY;
    struct dstr_block* next;
};
struct dstr_block* Block_Insert(struct BombList* first, int X, int Y)
{
    struct dstr_block* nB = NULL;
    if (first != NULL)
    {
        nB = (struct dstr_block*)malloc(sizeof(struct dstr_block));
        if (nB != NULL)
        {
            nB->gridX = X;
            nB->gridY = Y;
            struct dstr_block* end = first;
            while (end->next != NULL)
            {
                end = end->next;
            }
            printf("Added BLOCK at the end of %p\n", end);
            nB->next = NULL;
            end->next = nB;
        }
    }
    return nB;
}
bool Block_Exists(struct dstr_block* first, int X, int Y)
{
    struct dstr_block* block = first;
    while (block != NULL)
    {
        if (block->gridX == X && block->gridY == Y)
        {
            return true;
        }
        block = block->next;
    }
    return false;
}
unsigned int Bomb_count(struct BombList* first)
{
    unsigned int count = 0;
    printf("first: %p\n", first);
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
bool Bomb_ExistsAt(struct BombList* first, struct Vector2 pos)
{
    if (first != NULL)
    {
        struct BombList* seek = first;
        do
        {
            printf("%lf vs %lf\n%lf vs %lf", seek->Position.x, pos.x, seek->Position.y, pos.y);
            if (seek->Position.x == pos.x && seek->Position.y == pos.y)
            {
                return true;
            }
            seek = seek->next;
        } while (seek != NULL? seek->next != NULL : false);
    }
    return false;
}
struct Transform
{
    struct Vector2 position;
    struct Vector2 gridPosition;
    struct Vector2 rotation;
    struct Vector2 scale;
};
struct Character
{   
    struct Transform Transform;
    float Speed;
    ALLEGRO_BITMAP* sprite;
    bool remoteBombs;
    unsigned short int displayBombs;
    unsigned short int maxBombs;
    bool enabled;
};
struct Character* Player = NULL;
bool Bomb_Remove(struct BombList* bomb)
{
    if (debug)
    {
        printf("Deleting Bomb: %p\n", bomb);
    }
    if (bomb != NULL)
    {
        struct BombList* prev = bomb->prev;
        struct BombList* next = bomb->next;
        printf("PREV ADDRESS: %p\n", prev);
        bool any = false;
        if (prev != NULL)
        {
            prev->next = bomb->next;
            any = true;
        }
        else
        {
            bombs = bomb->next;
        }
        if (next != NULL)
        {
            next->prev = bomb->prev;
            any = true;
        }
        if (!any)
        {
            bombs = NULL;
        }
        free(bomb);
        bomb = NULL;
        if (Player != NULL)
        {
            Player->displayBombs--;
        }
        return true;
    }
    return false;
}
bool is_on_block(float x, float y, float dX, float dY) {
    int gridSize = (int)(128 * Player->Transform.scale.x);
    //float tarx = Player->Transform.position.x + (dX * ((float)gridSize/4));
    //float tary = Player->Transform.position.y + (dY * ((float)gridSize/4));
    float xdiv = x / gridSize;
    float ydiv = y / gridSize;

    if (debug) {
        //printf("div: %lf, %lf\n", xdiv, ydiv);
        //al_draw_rectangle((xdiv * gridSize) - gridSize / 2, (ydiv * gridSize) - gridSize / 2, (xdiv * gridSize) + gridSize / 2, (ydiv * gridSize) + gridSize / 2, al_map_rgba(255, 0, 0, 50), 3);
        //al_draw_rectangle(tarx - gridSize / 2, tary - gridSize / 2, tarx + gridSize / 2, tary + gridSize / 2, al_map_rgba(0, 0, 255, 50), 3);
    }
    bool con1 = ((int)xdiv & 1) && ((int)ydiv & 1);
    //bool con2 = ((int)xdiv & 1) && (int)tary / gridSize & 1;
    //bool con2 = false;
    //bool con3 = false;
    //bool con3 = ((int)tarx / gridSize & 1 && (int)ydiv & 1);
    //int cx = (int)tarx / gridSize;
    //int cy = (int)tary / gridSize;
    //float xdiff = x/ gridSize - (floor(x / gridSize));
    //float diff = y/ gridSize - (floor(y / gridSize));

    //bool con4 = cx & 1 && cy & 1;
    //bool con4 = false;
    //printf(":%lf, %lf, %d, %d, %d\n", xdiff, diff, con4, cx, cy);
    return con1;
}
void MovePlayer(struct Vector2 dir)
{
    if (Player != NULL)
    {
        struct Vector2 nDir;
        if (dir.x != 0 && dir.y != 0) { nDir.x = dir.x * 0.7; nDir.y = dir.y * 0.7; }
        nDir.x = (dir.x * deltaTime) * 40;
        nDir.y = (dir.y * deltaTime) * 40;
        float npy = (Player->Transform.position.y + nDir.y);
        float npx = (Player->Transform.position.x + nDir.x);
        float halfplayer = ((Player->Transform.scale.x / 2.0) * 128);
        if (!is_on_block(npx, Player->Transform.position.y, dir.x, dir.y)) {
            if (npx >= halfplayer && npx <= ((width*2)-halfplayer))
            {
                if (npx < 0) npx = 0;
                if (cam_x_offset >= 0)
                {
                    if ((npx - cam_x_offset - 50 < (Player->Transform.scale.x / 2.0) * 128
                        && nDir.x < 0)
                        ||
                        (npx - cam_x_offset + 50 > width - ((Player->Transform.scale.x / 2.0) * 128)
                            && nDir.x > 0))
                    {
                        cam_x_offset += nDir.x * 5;
                    }
                }
                Player->Transform.position.x = npx;
            }
            //printf("offset: %lf!\n", cam_x_offset);
        }
        if (!is_on_block(Player->Transform.position.x, npy, 0, dir.y)) {
            if (npy > (Player->Transform.scale.y / 2.0) * 128 && npy < height - (Player->Transform.scale.y / 2.0) * 128)
            {
                Player->Transform.position.y = npy;
            }
        }
        //}
        int cellsizex = (int)(128 * Player->Transform.scale.x);
        int cellsizey = (int)(128 * Player->Transform.scale.y);
        float x = (int)(cellsizex * ((int)(Player->Transform.position.x / cellsizex))+cellsizex/2);
        float y = (int)(cellsizex * ((int)(Player->Transform.position.y / cellsizey)) + cellsizey / 2);
        float xdf = Player->Transform.position.x/cellsizex - (int)(floor(Player->Transform.position.x / cellsizex));
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
            y -= cellsizey;
        }
        else if (xdf < 0.4 && dir.y < 0)
        {
            y += cellsizey;
        }
        //printf("ydf: %lf, %lf\n", ydf, dir.y);
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
    for (i = cam_x_offset - (int)fmod(cam_x_offset + gridSize, gridSize * 2) - gridSize + gridSize; i < cam_x_offset + width + gridSize; i += gridSize * 2) {
        for (j = (int)fmod(gridSize + 1, 2) - gridSize + gridSize; j < height; j += gridSize * 2) {
            if ((i / gridSize + j / gridSize) % 2 == 1) {
                rectX = i - cam_x_offset;
                rectY = j - gridSize;
                al_draw_filled_rectangle(rectX, rectY, rectX + gridSize, rectY + gridSize, al_map_rgb(150, 150, 150));
            }
        }
    }
}
void Blocks_draw(struct dstr_block* first)
{
    struct dstr_block* temp = first;
    int gridSize = (int)(128 * Player->Transform.scale.x); // rozmiar kratki
    while (temp != NULL)
    {
        if (temp != NULL) {
            printf("NOT NULL %p\n", temp);
            if (temp->gridX - cam_x_offset < width && temp->gridX - cam_x_offset > 0)
            {
                if (temp->gridY < height && temp->gridY > 0)
                {
                    al_draw_filled_rectangle(temp->gridX - cam_x_offset - gridSize / 2, temp->gridY - gridSize / 2, temp->gridX - cam_x_offset + gridSize / 2, temp->gridY + gridSize / 2, al_map_rgb(20, 20, 20));
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
void playerMovement()
{
    struct Vector2 dir;
    dir.x = 0;
    dir.y = 0;
    if (wasd[0] && !wasd[2])
    {
        dir.y = -1;
    }
    else if (!wasd[0] && wasd[2])
    {
        dir.y = 1;
    }
    if (wasd[1] && !wasd[3])
    {
        dir.x = -1;
    }
    else if (!wasd[1] && wasd[3])
    {
        dir.x = 1;
    }
    dir.x *= Player->Speed;
    dir.y *= Player->Speed;
    MovePlayer(dir);
}
void plantBomb()
{
    if (Player != NULL)
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
void renderBombs(ALLEGRO_BITMAP* bombSprite)
{
    if (bombs != NULL)
    {
        struct BombList* bomb = bombs;
        while (bombs != NULL && bomb != NULL)
        {
            al_draw_scaled_rotated_bitmap(bombSprite, 64, 64, bomb->Position.x-cam_x_offset, bomb->Position.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
            if (bomb->next != NULL) {
                bomb = bomb->next;
            }
            else break;
        }
    }
}
void loopBombs()
{
    if (bombs != NULL)
    {
        struct BombList* bomb = bombs;
        int i = 0;
        while (bombs != NULL && bomb != NULL)
        {
            bomb->timeLeft -= deltaTime;
            if (bomb->timeLeft <= 0)
            {
                printf("BOMB (ID: %d) SHOULD NOW EXPLODE!\n", i);
                //explodeBomb(bomb);
                Bomb_Remove(bomb);
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
void button(int key, bool down)
{
    switch (key)
    {
    case ALLEGRO_KEY_W:
    {
        wasd[0] = down;
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
            plantBomb();
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
    al_draw_filled_rectangle(5, 5, width - 5, 55, al_map_rgba(0, 0, 0, 150));

    al_draw_textf(*uiFont, al_map_rgba(255, 255, 255, 80), width - 15, 15, ALLEGRO_ALIGN_RIGHT, "BOMBS: %d/%d", Player->displayBombs, Player->maxBombs);
}
void drawGrid(int maxx, int maxy, int xsize, int ysize)
{
    if (Player != NULL) {
        int i, j;
        float x_offset = fmod(cam_x_offset, width);
        int window_ext = (int)(cam_x_offset / width);

        for (i = -window_ext; i < maxx + 1; i++)
        {
            float x1 = (i * xsize) - xsize - x_offset;
            float x2 = (i * xsize) + xsize - x_offset;

            if (x1 > width) {
                x1 -= width;
                x2 -= width;
            }
            else if (x2 < 0) {
                x1 += width;
                x2 += width;
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
void Block_random(struct dstr_block* first, int level, int* X, int* Y)
{
    srand(level);
    int gridSize = (int)(128 * Player->Transform.scale.x);
    int rX = rand() % (int)(width / Player->Transform.scale.x);
    int rY = rand() % (int)(height / Player->Transform.scale.y);
    if (Block_Exists(first, rX * gridSize, rY * gridSize))
    {
        Block_random(first, level, X, Y);
    }
    *X = rX * gridSize;
    *Y = rY * gridSize;
}
struct dstr_block* generate_blocks(int level)
{
    srand(level);
    struct dstr_block* Blocks = NULL;
    int limit = level * 10;
    for (int i = 0; i < limit; i++)
    {
        struct dstr_block* bl = Block_Insert
        if (bl != NULL)
        {
            int X = 0;
            int Y = 0;
            Block_random(Blocks, level, &X, &Y);
            bl->gridX = X;
            bl->gridX = Y;
            bl->next = NULL;
        }
    }
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
        Player->Transform.position.x = width / 2;
        Player->Transform.position.y = height / 2;
        Player->Speed = 3.0;
        Player->displayBombs = 0;
        Player->maxBombs = 8;
        Player->remoteBombs = false;
        Player->enabled = true;
        ALLEGRO_DISPLAY* display = NULL;
        ALLEGRO_EVENT_QUEUE* queue = NULL;
        ALLEGRO_TIMER* timer = NULL;

        // Initialize allegro

        if (!al_init_primitives_addon()) { return -1; }
        if (!al_install_keyboard()) { return -1; }
        if (!al_init_ttf_addon()) { return -1; }
        if (!al_init_image_addon()) { return -1; }
        // Initialize the timer
        timer = al_create_timer(1.0 / FPS);
        if (!timer) {
            fprintf(stderr, "Failed to create timer.\n");
            return 1;
        }

        // Create the display
        display = al_create_display(width, height);
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
        ALLEGRO_FONT* debugFont = al_load_ttf_font("data/data-unifon.ttf", 12, 0);
        ALLEGRO_FONT* uiFont = al_load_ttf_font("data/PlatNomor-WyVnn.ttf", 18, 0);
        if (debugFont == NULL || uiFont == NULL)
        {
            printf("Blad odczytu czcionki. \n");
            return -3;
        }
        Player->sprite = al_load_bitmap("data/Player.bmp");
        if (Player->sprite == NULL)
        {
            printf("Nie udalo sie wczytac pliku bmp gracza.\nAdres gracza: %p\nAdres sprite'a: %p\nKod bledu: %d\n", Player, Player != NULL? Player->sprite : Player, al_get_errno());
            return -1;
        }
        ALLEGRO_BITMAP* BombSprite = al_load_bitmap("data/bomb.bmp");
        if (BombSprite == NULL)
        {
            printf("Nie uda³o siê wczytaæ pliku bmp bomby\n%d\n", al_get_errno());
            return -1;
        }
        double time = al_get_time();
        double oldTime = time;
        float displayFps = 0;
        float displayFpsDelay = 0;
        int frames = 0;
        double syncDeltaTime = 0;


        int xsize = (int)(128 * Player->Transform.scale.x);
        int ysize = (int)(128 * Player->Transform.scale.y);
        int maxx = (int)(width / (xsize ));
        int maxy = (int)(width / (ysize ));
        int level = 1;

        struct dstr_block* blocks = generate_blocks(level);

        bool gridEnabled = false;
        while (!zamknij) {
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
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                button(event.keyboard.keycode, true);
                break;
            }
            case ALLEGRO_EVENT_KEY_UP:
            {
                button(event.keyboard.keycode, false);
                break;
            }
            }
            al_clear_to_color(al_map_rgb(10, 100, 10));
            ALLEGRO_COLOR color_blue = al_map_rgb(0, 0, 255);
            drawAllFilledRectInView();
            Blocks_draw(&blocks);
            if (Player->enabled)
            {
                //al_draw_filled_rectangle(Player->Transform.position.x - Player->Transform.scale.x / 2.0, Player->Transform.position.y - Player->Transform.scale.y / 2.0, Player->Transform.position.x + Player->Transform.scale.x, Player->Transform.position.y + Player->Transform.scale.y, color_blue);
                al_draw_scaled_rotated_bitmap(Player->sprite, 64, 64, Player->Transform.position.x - cam_x_offset, Player->Transform.position.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                //al_draw_scaled_rotated_bitmap(Player->sprite, 64, 64, Player->Transform.gridPosition.x, Player->Transform.gridPosition.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                al_draw_filled_rounded_rectangle((Player->Transform.gridPosition.x - (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y - (128 * Player->Transform.scale.y) / 2), (Player->Transform.gridPosition.x + (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y + (128 * Player->Transform.scale.y) / 2), 20, 20, al_map_rgba(0, 100, 100, 10));
                playerMovement();
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
            renderBombs(BombSprite);
            loopBombs();
            al_flip_display();
        }
        free(Player);
        al_uninstall_keyboard();
        al_destroy_display(display);
        al_shutdown_primitives_addon();
    }
    return 0;
}
