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
#include "types.h"
#include "Bombs.h"
#include "Blocks.h"
#include "Explosions.h"
#ifndef WIDTH
    #define WIDTH 832
    #define HEIGHT 832
#endif // !WIDTH

struct BombList* bombs = NULL;
const bool debug = true;
bool wasd[4] = { false, false, false, false };
double deltaTime = 0;
float cam_x_offset = 0;
float cam_y_offset = 0;
unsigned short GlobalAction = 0;

struct Character* Player = NULL;


struct Explosion* explosions = NULL;

void MovePlayer(struct Vector2 dir, struct dstr_block* blocks)
{
    if (Player != NULL)
    {
        struct Vector2 nDir;
        nDir.x = (dir.x * deltaTime) * 40;
        nDir.y = (dir.y * deltaTime) * 40;
        float npy = (Player->Transform.position.y + nDir.y);
        float npx = (Player->Transform.position.x + nDir.x);
        float halfplayer = ((Player->Transform.scale.x / 2.0) * 128);
        bool dstr = false;
        if (!is_on_block(blocks, npx, Player->Transform.position.y, dir.x, dir.y, &dstr, Player, bombs)) {
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
        }
        if (!is_on_block(blocks, Player->Transform.position.x, npy, 0, dir.y, &dstr, Player, bombs)) {
            if (npy > (Player->Transform.scale.y / 2.0) * 128 && npy < HEIGHT + 1 - ((128 * Player->Transform.scale.y)/2.0))
            {
                Player->Transform.position.y = npy;
            }
        }
        int cellsizex = (int)(128 * Player->Transform.scale.x);
        int cellsizey = (int)(128 * Player->Transform.scale.y);
        float x = (int)(cellsizex * ((int)(Player->Transform.position.x / cellsizex))+cellsizex/2);
        float y = (int)(cellsizex * ((int)(Player->Transform.position.y / cellsizey)) + cellsizey / 2);
        if (cam_x_offset > WIDTH)
        {
            cam_x_offset = WIDTH;
        }
        Player->Transform.gridPosition.x = x;
        Player->Transform.gridPosition.y = y;
        if (cam_x_offset < 0) cam_x_offset = 0;
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
            printf("Nie uda³o siê wczytaæ pliku gif bomby\n%d\n", al_get_errno());
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
                        plantBomb(Pause, Player, &bombs);
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
                    blocks = generate_blocks(level, Player);
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
                    Blocks_draw(blocks, dBlockSprite, sBlockSprite, Player, cam_x_offset, cam_y_offset);
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
                    loopBombs(&bombs, &blocks, explosionSound, &samples, deltaTime, Player, &cam_y_offset, &explosions);
                }
                renderBombs(BombAnim, AnimTime, cam_x_offset, cam_y_offset, Player, bombs);
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
