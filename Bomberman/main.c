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
const int width = 800;
const int height = 600;
const bool debug = true;
bool wasd[4] = { false, false, false, false };
double deltaTime = 0;

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
struct Transform
{
    struct Vector2 position;
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
void MovePlayer(struct Vector2 dir)
{
    if (Player != NULL)
    {
        dir.x = (dir.x * deltaTime) * 40;
        dir.y = (dir.y * deltaTime) * 40;
        float npy = (Player->Transform.position.y + dir.y);
        float npx = (Player->Transform.position.x + dir.x);
        if (npx > (Player->Transform.scale.x / 2.0) * 128 && npx < width - (Player->Transform.scale.x / 2.0) * 128)
        {
            Player->Transform.position.x = npx;
        }
        if (npy > (Player->Transform.scale.y / 2.0) * 128 && npy < height - (Player->Transform.scale.y / 2.0) * 128)
        {
            Player->Transform.position.y = npy;
        }
        //printf("%lf, %lf\n", Player->Transform.position.x, Player->Transform.position.y);
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
    if (dir.x != 0 && dir.y != 0) { dir.x = dir.x *0.7; dir.y = dir.y * 0.7; }
    dir.x *= Player->Speed;
    dir.y *= Player->Speed;
    MovePlayer(dir);
}
void plantBomb()
{
    if (Player != NULL)
    {
        bool canplant = false;
        struct BombList* planted = NULL;
        if (Player->enabled)
        {
            unsigned int count = Bomb_count(bombs);
            printf("Bombs BEFORE: %d\n", count);
            if (count > 0)
            {
                if (count < Player->maxBombs) {
                    canplant = true;
                }
            }
            else
            {
                planted = Bomb_CreateList(Player->Transform.position, Player->remoteBombs);
                bombs = planted;
                printf("Bombs 0, creating a list. Address: %p\n   prev: %p\n   next: %p\n", bombs, bombs->prev, bombs->next);
            }
        }
        if (canplant)
        {
            planted = Bomb_InsertInto(bombs, Player->Transform.position, Player->remoteBombs);
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
            al_draw_scaled_rotated_bitmap(bombSprite, 64, 64, bomb->Position.x, bomb->Position.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
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

bool zamknij = false;
const int FPS = 30;
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
        Player->Speed = 2.0;
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
            if (Player->enabled)
            {
                //al_draw_filled_rectangle(Player->Transform.position.x - Player->Transform.scale.x / 2.0, Player->Transform.position.y - Player->Transform.scale.y / 2.0, Player->Transform.position.x + Player->Transform.scale.x, Player->Transform.position.y + Player->Transform.scale.y, color_blue);
                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 80), width-10, 10, ALLEGRO_ALIGN_RIGHT, "BOMBS: %d/%d", Player->displayBombs, Player->maxBombs);
                al_draw_scaled_rotated_bitmap(Player->sprite, 64, 64, Player->Transform.position.x, Player->Transform.position.y, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                playerMovement();
            }
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
                al_draw_textf(debugFont, al_map_rgba(0, 255, 0, 150), 5, 5, ALLEGRO_ALIGN_LEFT, "fps: %.2lf", displayFps);
                al_draw_textf(debugFont, al_map_rgba(100, 255, 0, 150), 5, 17, ALLEGRO_ALIGN_LEFT, "/\\t: %lf", deltaTime);
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
