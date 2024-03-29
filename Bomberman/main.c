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
#include "main.h"
#include "Explosions.h"
#include <time.h>
#include "endDoor.h"
#include "boosts.h"
#ifndef WIDTH
    /**
    * @brief Szerokosc okna.
    */
    #define WIDTH 832
    /**
    * @brief Wysokosc okna.
    */
    #define HEIGHT 832
#endif // !WIDTH
/**
* @brief Wskaznik na liste bomb (jej pierwszy element) 
*/
struct BombList* bombs = NULL;
/**
* @brief Tablica wcisnietych przyciskow (do poruszania sie gracza)
*/
bool wasd[4] = { false, false, false, false };
/**
* @brief Czas miedzy klatkami.
*/
double deltaTime = 0;
/**
* @brief Offset kamery na osi X.
*/
float cam_x_offset = 0;
/**
* @brief Offset kamery na osi Y.
*/
float cam_y_offset = 0;
/**
 * @brief Zmienna globalna odpowiadajaca za "zdalne akcje" z kazdego poziomu kodu.
 */
unsigned short GlobalAction = 0;
/**
* @brief Wskaznik na gracza.
*/
struct Character* Player = NULL;

/**
 * @brief Finalne poruszanie sie gracza wraz z sprawdzaniem kolizji
 *
 * @param dir Kierunek poruszania sie gracza
 * @param blocks Wskaznik na liste blokow
 */
void MovePlayer(struct Vector2 dir, struct dstr_block* blocks)
{
    if (Player != NULL)
    {
        //printf("Before Normalize: %lf, %lf\n", dir.x, dir.y);
        if (dir.x != 0 && dir.y != 0)
        {
            float length = sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= length;
            dir.y /= length;
            //dir.x *= 1.3;
            //dir.y *= 1.3;
            //printf("After Normalize: %lf, %lf\n", dir.x, dir.y);
        }
        struct Vector2 nDir;
        nDir.x = (dir.x * Player->Speed * deltaTime) * 40;
        nDir.y = (dir.y * Player->Speed * deltaTime) * 40;
        float npy = (Player->Transform.position.y + nDir.y);
        float npx = (Player->Transform.position.x + nDir.x);
        float halfplayer = ((Player->Transform.scale.x / 2.0) * 128);
        bool dstr = false;
        if (!is_on_block(blocks, npx, Player->Transform.position.y, &dstr, Player, bombs, false, 10)) {
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
                        cam_x_offset += nDir.x * 65 * deltaTime;
                    }
                }
                Player->Transform.position.x = npx;
            }
        }
        if (!is_on_block(blocks, Player->Transform.position.x, npy, &dstr, Player, bombs, false, 10)) {
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
/**
 * @brief Wyswietlanie osiagniecia
 *
 * @param timeleft Wskaznik na wartosc pozostalego czasu (float*)
 * @param id ID osiagniecia
 * @param font Jedna z czcionek
 * @param font2 Jedna z czcionek
 */
void Achievement(float* timeleft, int id, ALLEGRO_FONT* font, ALLEGRO_FONT* font2)
{
    *timeleft -= deltaTime;
    if (*timeleft > 0) {
        al_draw_filled_rectangle(WIDTH / 2 - 200, HEIGHT - 150, WIDTH / 2 + 200, HEIGHT - 80, al_map_rgba_f(0, 0, 0, 0.5));
        al_draw_textf(font, al_map_rgba_f(1, 1, 1, 1), WIDTH / 2, HEIGHT - 140, ALLEGRO_ALIGN_CENTER, "OSIAGNIECIE ODBLOKOWANE!");

        switch (id)
        {
        case 1:
        {
            al_draw_textf(font2, al_map_rgba_f(1, 1, 1, 1), WIDTH / 2, HEIGHT - 110, ALLEGRO_ALIGN_CENTER, "Wyeliminuj 2 przeciwnikow na raz.");
            break;
        }
        case 2:
        {
            al_draw_textf(font2, al_map_rgba_f(1, 1, 1, 1), WIDTH / 2, HEIGHT - 110, ALLEGRO_ALIGN_CENTER, "Znajdz ulepszenie.");
            break;
        }
        default:
        {break; }
        }
        
    }
    else
    {
        *timeleft = 0;
    }
}
/**
 * @brief Zarzadzanie poruszaniem sie gracza
 *
 * @param blocks Wskaznik na liste blokow
 */
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
    MovePlayer(dir, blocks);
}
/**
 * @brief Nawigacja po menu glownym (ekranie startowym)
 *
 * @param down Czy gracz zjezdza w dol czy do gory.
 * @param val Wskaznik na wartosc (ID) przycisku na ktory gracz sie przelacza (zmieniany poprzez wskaznik).
 * @param level Poziom (0 to menu glowne).
 * @param buttondown Czy klawisz byl wcisniety (a nie podniesiony).
 */
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
            else if (*val > 4)
            {
                *val = 4;
            }
        }
        else
        {
            *val = 0;
        }
    }
}
/**
 * @brief Obsluga przyciskow
 *
 * @param key Ktory klawisz wcisnieto.
 * @param down Czy klawisz byl wcisniety (a nie podniesiony).
 * @param currentLevel Wskaznik na numer aktualnego poziomu.
 * @param menuChoiceValue Wskaznik na wartosc (ID) aktualnie wybranego przycisku w menu.
 */
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
                    GlobalAction = 8;
                    break;
                }
                case 4:
                {
                    GlobalAction = 2;
                    break;
                }
                }
            }
            else if (*currentLevel<=5)
            {
                GlobalAction = 4;
            }
            else if (*currentLevel == 6)
            {
                GlobalAction = 1;
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
/**
 * @brief Wyswietlanie UI (Interfejsu Uzytkownika)
 *
 * @param uiFont Jedna z czcionek
 * @param LevelFont Jedna z czcionek
 * @param level Numer aktualnego poziomu.
 * @param enemies Wskaznik na liste przeciwnikow.
 * @param blocks Wskaznik na liste blokow.
 */
void drawUI(ALLEGRO_FONT** uiFont, ALLEGRO_FONT** LevelFont, int level, struct Enemy* enemies, struct dstr_block* blocks)
{
    al_draw_filled_rectangle(5, 5, WIDTH - 5, 55, al_map_rgba(0, 0, 0, 150));

    al_draw_textf(*uiFont, al_map_rgba(255, 255, 255, 80), 10, 12, ALLEGRO_ALIGN_LEFT, "PRZECIWNIKOW: %d", Enemies_Count(enemies));
    al_draw_textf(*uiFont, al_map_rgba(255, 255, 255, 80), 10, 35, ALLEGRO_ALIGN_LEFT, "BLOKOW: %d", Blocks_Count(blocks));
    al_draw_textf(*LevelFont, al_map_rgba(255, 255, 255, 80), WIDTH/2, 15, ALLEGRO_ALIGN_CENTER, "POZIOM: %d/5", level);
    al_draw_textf(*uiFont, al_map_rgba(255, 255, 255, 80), WIDTH - 15, 15, ALLEGRO_ALIGN_RIGHT, "BOMBY: %d/%d", Bomb_count(bombs), Player->maxBombs);
}
/**
 * @brief Funkcja glowna (startowa) - MAIN
 */
int main()
{
    const bool debug = false;
    bool zamknij = false;
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
        Player->maxBombs = 2;
        Player->bombRange = 1;
        Player->enabled = true;

        ALLEGRO_DISPLAY* display = NULL;
        ALLEGRO_EVENT_QUEUE* queue = NULL;

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
        ALLEGRO_SAMPLE* deathSound = al_load_sample("data/sounds/bomberdeath.ogg");
        if (!MenuMusic || !GameMusic || !deathSound || !explosionSound)
        {
            printf("SOUNDS LOADING FAILED! %p and %p\n", MenuMusic, GameMusic);
            return -11;
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
        al_register_event_source(queue, al_get_display_event_source(display));
        al_register_event_source(queue, al_get_keyboard_event_source());

        // Display a black screen
        al_clear_to_color(al_map_rgb(0, 30, 0));
        al_flip_display();

        // Start the timer
        ALLEGRO_FONT* debugFont = al_load_ttf_font("data/fonts/data-unifon.ttf", 12, 0);
        ALLEGRO_FONT* uiFont = al_load_ttf_font("data/fonts/PlatNomor-WyVnn.ttf", 18, 0);
        ALLEGRO_FONT* TitleFont = al_load_ttf_font("data/fonts/AldotheApache.ttf", 100, 0);
        ALLEGRO_FONT* levelfont = al_load_ttf_font("data/fonts/AldotheApache.ttf", 35, 0);
        ALLEGRO_FONT* ach1font = al_load_ttf_font("data/fonts/AldotheApache.ttf", 25, 0);
        ALLEGRO_FONT* MenuButton = al_load_ttf_font("data/fonts/Exo-Regular.ttf", 30, 0);
        if (debugFont == NULL || uiFont == NULL || !TitleFont || !ach1font)
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
        ALGIF_ANIMATION* Enemy3_animation = algif_load_animation("data/gifs/enemies/enemy3.gif");
        if (!Player->IdleAnim || !Player->RightWalkAnim || !Player->LeftWalkAnim || !Player->UpWalkAnim || !Player->DownWalkAnim)
        {
            printf("Nie udalo sie wczytac animacji gracza.\n");
            return -1;
        }
        if (!Enemy1_animation || !Enemy2_animation || !Enemy3_animation)
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
        ALLEGRO_BITMAP* grass = al_load_bitmap("data/grass.bmp");
        ALLEGRO_BITMAP* boost1 = al_load_bitmap("data/boost1.bmp");
        ALLEGRO_BITMAP* boost2 = al_load_bitmap("data/boost2.bmp");
        ALLEGRO_BITMAP* boost3 = al_load_bitmap("data/boost3.bmp");
        doorSprite = al_load_bitmap("data/door.bmp");
        if (!dBlockSprite || !sBlockSprite || !grass || !doorSprite || !boost1 || !boost2 || !boost3)
            return -15;

        double ctime = al_get_time();
        double oldTime = ctime;
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
        unsigned short MenuChoice = 0;
        if (!saveSystem_init()) return -10;
        int loadedLevel = saveSystem_LoadLevel();
        bool Pause = false;
        double AnimTime = al_get_time();
        ALLEGRO_SAMPLE_INSTANCE* DeathSoundInstance = al_create_sample_instance(deathSound);
        ALLEGRO_SAMPLE_INSTANCE* MMusicInstance = al_create_sample_instance(MenuMusic);
        //ALLEGRO_SAMPLE_INSTANCE* explosionsInstance = al_create_sample_instance(explosionSound);
        ALLEGRO_SAMPLE_INSTANCE* GMusicInstance = al_create_sample_instance(GameMusic);
        al_set_sample_instance_playmode(MMusicInstance, ALLEGRO_PLAYMODE_LOOP);
        al_attach_sample_instance_to_mixer(MMusicInstance, al_get_default_mixer());
        al_set_sample_instance_playmode(GMusicInstance, ALLEGRO_PLAYMODE_LOOP);
        al_attach_sample_instance_to_mixer(GMusicInstance, al_get_default_mixer());
        al_attach_sample_instance_to_mixer(DeathSoundInstance, al_get_default_mixer());
        struct Explosion* explosions = NULL;
        struct Enemy* EnemyList = NULL;
        struct SampleStackElement* samples = NULL;
        struct Boost* Boosts = NULL;
        //  int Lifes = 3;
        bool diedsoundplayed = false;
        bool gonext = false;
        int currAch = 0;
        float AchTime = 0;
        bool hasAch1 = false;
        bool hasAch2 = false;
        while (!zamknij)
        {
            if (GlobalAction == 2)
            {
                zamknij = true;
                return;
            }
            al_wait_for_vsync();
            ALLEGRO_EVENT event;
            al_wait_for_event_timed(queue, &event, 0.00);
            ctime = al_get_time();
            deltaTime = ctime - oldTime;
            syncDeltaTime += deltaTime;
            oldTime = ctime;
            frames++;
            
            switch (event.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
            {
                GlobalAction = 2;
                continue;
            }
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
            if (level != 0 && level<=5) {
                if (!al_get_sample_instance_playing(GMusicInstance) && Player->enabled)
                {
                    al_play_sample_instance(GMusicInstance);
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
                        if (Pause || !Player->enabled || gonext)
                        {
                            Bomb_RemoveList(&bombs);
                            Block_RemoveList(&blocks);
                            Explosion_RemoveList(&explosions);
                            al_stop_sample_instance(GMusicInstance);
                            Enemies_Clear(&EnemyList);
                            DestroyDoor();
                            cam_x_offset = 0;
                            cam_y_offset = 0;
                            diedsoundplayed = false;
                            gonext = false;
                            Pause = false;
                            level = 0;
                            loadedLevel = 0;
                            Boosts_Clear(&Boosts);

                            GlobalAction = 0;
                            continue;

                        }
                        GlobalAction = 0;
                        break;
                    }
                    case 5:
                    {
                        plantBomb(Pause, Player, &bombs, blocks);
                        GlobalAction = 0;
                        break;
                    }
                    case 6:
                    {
                        currAch = 1;
                        AchTime = 3;
                        GlobalAction = 0;
                        break;
                    }
                    case 7:
                    {
                        currAch = 2;
                        AchTime = 3;
                        GlobalAction = 0;
                        break;
                    }
                    default:
                    {
                        GlobalAction = 0;
                        break;
                    }
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
                al_draw_bitmap(grass, 0 - cam_x_offset, 0, 0);
                bool check = false;
                if (!Pause)
                {
                    loopBlocks(&blocks, EnemyList);
                }
                if (endingDoor)
                {
                    al_draw_scaled_bitmap(doorSprite, 0, 0, 128, 128, endingDoor->x - 32 - cam_x_offset, endingDoor->y - cam_y_offset - 32, 64, 64, 0);
                    if (Door_hasPlayerIn(Player))
                    {
                        if (level && level < 5) {
                            int nextlevel = (level + 1);
                            char lstr[32] = "";
                            _itoa_s(nextlevel, lstr, LINE_LENGTH, 10);
                            saveSystem_printAtLine(1, lstr);
                            _itoa_s(Player->maxBombs, lstr, LINE_LENGTH, 10);
                            saveSystem_printAtLine(3, lstr);
                            _itoa_s(Player->bombRange, lstr, LINE_LENGTH, 10);
                            saveSystem_printAtLine(4, lstr);
                            snprintf(lstr, LINE_LENGTH*sizeof(char), "%f", Player->Speed);
                            //printf("Saving speed: %s", lstr);
                            saveSystem_printAtLine(5, lstr);

                            gonext = true;
                            GlobalAction = 4;
                            continue;
                        }
                    }
                }
                Boost_Loop(&Boosts, Player, cam_x_offset, cam_y_offset, boost1, boost2, boost3);
                Enemies_Loop(EnemyList, AnimTime, cam_x_offset, cam_y_offset, Player, deltaTime, blocks, bombs, Pause);

                if (!check) {
                    Blocks_draw(blocks, dBlockSprite, sBlockSprite, Player, cam_x_offset, cam_y_offset);
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
                        al_draw_scaled_rotated_bitmap(sprite, 64, 64, Player->Transform.position.x - cam_x_offset, Player->Transform.position.y - cam_y_offset, Player->Transform.scale.x, Player->Transform.scale.y, 0, 0);
                        al_draw_rounded_rectangle((Player->Transform.gridPosition.x - (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y - cam_y_offset - (128 * Player->Transform.scale.y) / 2), (Player->Transform.gridPosition.x + (128 * Player->Transform.scale.x) / 2) - cam_x_offset, (Player->Transform.gridPosition.y - cam_y_offset + (128 * Player->Transform.scale.y) / 2), 20, 20, al_map_rgba(150, 20, 20, 3), 2);
                    }
                }
                check = false;
                if (!Pause)
                {
                    loopExplosions(&explosions, &check, deltaTime);
                    if (!check)
                    {
                        renderExplosions(explosions, cam_x_offset, cam_y_offset);
                    }
                    loopBombs(&bombs, &blocks, explosionSound, &samples, deltaTime, Player, &cam_y_offset, &explosions, &EnemyList, &Boosts);
                }
                renderBombs(BombAnim, AnimTime, cam_x_offset, cam_y_offset, Player, bombs);

                drawUI(&uiFont, &levelfont, level, EnemyList, blocks);
                if (AchTime > 0 && currAch)
                {
                    Achievement(&AchTime, currAch, ach1font, uiFont);
                }
                if (debug)
                {
                    if (displayFpsDelay <= 0)
                    {
                        displayFps = frames / syncDeltaTime;
                        displayFpsDelay = 0.2;
                        syncDeltaTime = 0;
                        frames = 0;
                    }
                    else
                    {
                        displayFpsDelay -= deltaTime;
                    }
                    al_draw_textf(debugFont, al_map_rgba(0, 255, 0, 150), 10, HEIGHT - 20, ALLEGRO_ALIGN_LEFT, "fps: %.2lf", displayFps);
                    al_draw_textf(debugFont, al_map_rgba(100, 255, 0, 150), 10, HEIGHT - 40, ALLEGRO_ALIGN_LEFT, "/\\t: %lf", deltaTime);
                    al_draw_textf(debugFont, al_map_rgba(100, 255, 0, 150), 10, HEIGHT - 60, ALLEGRO_ALIGN_LEFT, "X: %lf, %Y: %lf", Player->Transform.position.x, Player->Transform.position.y);
                }
                if (!Player->enabled)
                {
                    if (!diedsoundplayed)
                    {
                        diedsoundplayed = true;
                        al_play_sample_instance(DeathSoundInstance);
                        al_stop_sample_instance(GMusicInstance);
                    }
                    al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(0, 0, 0, 180));
                    al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH / 2, HEIGHT / 2 - 150, ALLEGRO_ALIGN_CENTER, "GAME OVER");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT / 2 - 50, ALLEGRO_ALIGN_CENTER, "Kliknij ENTER aby wyjsc do menu...");
                }
                if (Pause)
                {
                    al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(0,0,0,180));
                    al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH / 2, HEIGHT/2 - 150, ALLEGRO_ALIGN_CENTER, "PAUZA");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT/2 - 50, ALLEGRO_ALIGN_CENTER, "Kliknij ESC aby wrocic do gry.");
                    al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 120), WIDTH / 2, HEIGHT/2 - 25, ALLEGRO_ALIGN_CENTER, "Kliknij ENTER aby wyjsc do menu.");
                }
            }
            else if (level==0)
            {

                if (!al_get_sample_instance_playing(MMusicInstance))
                {
                    al_play_sample_instance(MMusicInstance);
                }
                if (MenuChoice == 0)
                    MenuChoice = 1;
                al_clear_to_color(al_map_rgb(10, 30, 20));
                al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, 30, ALLEGRO_ALIGN_CENTER, "BOOM-ba Man");
                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, HEIGHT-20, ALLEGRO_ALIGN_CENTER, "Sygut Grzegorz, Strzepek Piotr, Szylinski Krzysztof, Synowiec Adrian");
                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, HEIGHT-40, ALLEGRO_ALIGN_CENTER, "PP2 Projekt (1ID14B SEM2 2023)");
                ALLEGRO_COLOR butColor = al_map_rgb(80, 80, 80);
                ALLEGRO_COLOR butColor2 = butColor;
                ALLEGRO_COLOR butColor3 = butColor;
                ALLEGRO_COLOR butColor4 = butColor;
                if (MenuChoice==1)
                    butColor = al_map_rgb(255, 255, 255);
                else if (MenuChoice==2)
                    butColor2 = al_map_rgb(255, 255, 255);
                else if (MenuChoice == 3)
                    butColor3 = al_map_rgb(255, 255, 255);
                else if (MenuChoice == 4)
                    butColor4 = al_map_rgb(255, 255, 255);
                al_draw_textf(MenuButton, butColor, WIDTH / 2, 300, ALLEGRO_ALIGN_CENTER, "Nowa Gra");
                al_draw_textf(MenuButton, butColor2, WIDTH / 2, 350, ALLEGRO_ALIGN_CENTER, "Kontynuuj (Level %d)",loadedLevel);
                al_draw_textf(MenuButton, butColor3, WIDTH / 2, 400, ALLEGRO_ALIGN_CENTER, "Statystyki");
                al_draw_textf(MenuButton, butColor4, WIDTH / 2, 450, ALLEGRO_ALIGN_CENTER, "Wyjdz");
                if (loadedLevel == 0)
                {
                    loadedLevel = saveSystem_LoadLevel();
                    Player->bombRange = saveSystem_LoadRange();
                    Player->Speed = saveSystem_LoadSpeed();
                    Player->maxBombs = saveSystem_LoadBombs();
                }
                switch (GlobalAction)
                {
                    case 1:
                    {
                        loadedLevel = saveSystem_LoadLevel();

                        Player->Transform.position.x = WIDTH / 2;
                        Player->Transform.position.y = HEIGHT / 2;
                        al_stop_sample_instance(MMusicInstance);
                        Player->bombRange = saveSystem_LoadRange();
                        Player->Speed = saveSystem_LoadSpeed();
                        Player->maxBombs = saveSystem_LoadBombs();
                        level = loadedLevel;
                        if (!blocks)
                        {
                            blocks = generate_blocks(level, Player);
                        }
                        switch (level)
                        {
                            case 1:
                            {
                                float X, Y;
                                for (int i = 0; i <3; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 70, Enemy1_animation,false);
                                }
                                break;
                            }
                            case 2:
                            {
                                float X, Y;

                                for (int i = 0; i < 3; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level,bombs);
                                    Enemy_Add(&EnemyList, X, Y, 120, Enemy2_animation,false);

                                }
                                for (int i = 0; i < 2; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level,bombs);
                                    Enemy_Add(&EnemyList, X, Y, 70, Enemy1_animation,false);
                                }
                                break;
                            }
                            case 3:
                            {
                                float X, Y;

                                for (int i = 0; i < 2; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 120, Enemy2_animation,false);

                                }
                                for (int i = 0; i < 4; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 70, Enemy1_animation,false);
                                }
                                break;
                            }
                            case 4:
                            {
                                float X, Y;

                                for (int i = 0; i < 2; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 120, Enemy2_animation,false);

                                }
                                for (int i = 0; i < 6; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 70, Enemy1_animation,false);
                                }
                                for (int i = 0; i < 2; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 150, Enemy3_animation, true);
                                }
                                break;
                            }
                            case 5:
                            {
                                float X, Y;

                                for (int i = 0; i < 3; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 120, Enemy2_animation, false);

                                }
                                for (int i = 0; i < 5; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 70, Enemy1_animation, false);
                                }
                                for (int i = 0; i < 5; i++) {
                                    Enemy_RandomPosition(EnemyList, &X, &Y, blocks, Player, level, bombs);
                                    Enemy_Add(&EnemyList, X, Y, 150, Enemy3_animation, true);
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                        //Enemy_Add(&EnemyList, WIDTH/2+256, HEIGHT/2, 0.1, Enemy2_animation);
                        Player->enabled = true;
                        GlobalAction = 0;
                        break;
                    }
                    case 8:
                    { 
                        level = 6;
                        hasAch1 = hasAchievement(1);
                        hasAch2 = hasAchievement(2);
                        Player->Speed = saveSystem_LoadSpeed();
                        Player->maxBombs = saveSystem_LoadBombs();
                        Player->bombRange = saveSystem_LoadRange();
                        MenuChoice = 0;
                        continue;
                    }
                    default:
                        { break; }
                }


            }
            else if (level == 6)
            {
                al_clear_to_color(al_map_rgb(10, 10, 50));
                al_draw_textf(TitleFont, al_map_rgba(255, 255, 255, 255), WIDTH / 2, 30, ALLEGRO_ALIGN_CENTER, "Statystyki");
                al_draw_textf(MenuButton, al_map_rgba(255, 255, 255, 255), 30, 180, ALLEGRO_ALIGN_LEFT, "Eliminacja 2 przeciwnikow na raz: %s", hasAch1? "Tak" : "Nie");
                al_draw_textf(MenuButton, al_map_rgba(255, 255, 255, 255), 30, 220, ALLEGRO_ALIGN_LEFT, "Znaleziono ulepszenie: %s", hasAch2? "Tak" : "Nie");
                al_draw_textf(MenuButton, al_map_rgba(255, 255, 255, 255), 30, 300, ALLEGRO_ALIGN_LEFT, "Predkosc Gracza: %lf", Player->Speed);
                al_draw_textf(MenuButton, al_map_rgba(255, 255, 255, 255), 30, 330, ALLEGRO_ALIGN_LEFT, "Maksymalnie Bomb: %d", Player->maxBombs);
                al_draw_textf(MenuButton, al_map_rgba(255, 255, 255, 255), 30, 360, ALLEGRO_ALIGN_LEFT, "Zasieg bomb: %d", Player->bombRange);

                al_draw_textf(uiFont, al_map_rgba(255, 255, 255, 255), WIDTH/2, HEIGHT-30, ALLEGRO_ALIGN_CENTER, "Wcisnij ENTER by wyjsc do menu");
                if (GlobalAction == 1)
                {
                    level = 0;
                    GlobalAction = 0;
                }
            }
            SampleStack_Loop(&samples, deltaTime);
            al_flip_display();

        }
        Boosts_Clear(&Boosts);
        DestroyDoor();
        algif_destroy_animation(Player->IdleAnim);
        algif_destroy_animation(Player->DownWalkAnim);
        algif_destroy_animation(Player->LeftWalkAnim);
        algif_destroy_animation(Player->RightWalkAnim);
        algif_destroy_animation(Player->UpWalkAnim);
        Bomb_RemoveList(&bombs);
        Block_RemoveList(&blocks);
        Explosion_RemoveList(&explosions);
        Enemies_Clear(&EnemyList);
        free(Player);
        al_destroy_font(debugFont);
        al_destroy_font(uiFont);
        al_destroy_font(TitleFont);
        al_destroy_font(levelfont);
        al_destroy_font(ach1font);
        al_destroy_font(MenuButton);
        al_uninstall_audio();
        al_destroy_sample(MenuMusic);
        al_destroy_sample(GameMusic);
        saveSystem_close();
        al_uninstall_keyboard();
        al_destroy_display(display);
        al_shutdown_primitives_addon();
        algif_destroy_animation(Enemy1_animation);
        algif_destroy_animation(Enemy2_animation);
        algif_destroy_animation(Enemy3_animation);
    }
    return 0;
}
