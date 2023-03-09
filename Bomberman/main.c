#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/events.h>
#include <allegro5/keycodes.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
static const int width = 800;
static const int height = 600;

enum CharacterType
{
    PlayerType,
    AI
};
struct Vector2
{
    double x;
    double y;
};
struct Transform
{
    struct Vector2 position;
    struct Vector2 rotation;
    struct Vector2 scale;
};
struct Character
{   
    struct Transform Transform;
    bool enabled;

};

void buttonDown(int key, ALLEGRO_DISPLAY* display)
{
    switch (key)
    {
    case ALLEGRO_KEY_W:
    {

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
struct Character Player;
int main()
{
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize allegro.\n");
        return -2;
    }
    if (!al_init_font_addon())
    {
        return -1;
    }
    Player.Transform.scale.x = 0.05*width;
    Player.Transform.scale.y = 0.05*height;
    Player.enabled = true;
    ALLEGRO_DISPLAY* display = NULL;
    ALLEGRO_EVENT_QUEUE* queue = NULL;
    ALLEGRO_TIMER* timer = NULL;
    bool redraw = true;

    // Initialize allegro

    if (!al_init_primitives_addon()) { return -1; }
    if (!al_install_keyboard()) { return -1; }
    if (!al_init_ttf_addon()) { return -1; }
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
    ALLEGRO_FONT* font = al_load_ttf_font("data/data-unifon.ttf", 24, 0);
    if (font == NULL)
    {
        printf("Blad odczytu czcionki. \n");
        return -3;
    }
    /*ALLEGRO_BITMAP* playerBMP = al_load_bitmap("player.bmp");
    if (playerBMP == NULL)
    {
        ALLEGRO_PATH* resource_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
        printf("Nie uda³o siê wczytaæ pliku: %s\n", al_path_cstr(resource_path, '/'));
        return -1;
    }*/
    while (!zamknij) {
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.06);
        ALLEGRO_EVENT event;
        al_draw_text(font, al_map_rgb(255, 255, 255), 150, 150, ALLEGRO_ALIGN_CENTER, "Hello, World!");
        al_wait_for_event_timed(queue, &event, 0.1);
        //al_wait_for_vsync();

        switch (event.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                zamknij = true;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                buttonDown(event.keyboard.keycode, display);
                break;
            }
        }
        al_clear_to_color(al_map_rgb(50, 255, 0));
        ALLEGRO_COLOR color_blue = al_map_rgb(0, 0, 255);
        if (Player.enabled)
        {
            al_draw_filled_rectangle(Player.Transform.position.x - Player.Transform.scale.x / 2.0, Player.Transform.position.y - Player.Transform.scale.y / 2.0, Player.Transform.position.x + Player.Transform.scale.x, Player.Transform.position.y + Player.Transform.scale.y, color_blue);
        }
        al_flip_display();
        puts("printed frame");
    }

    al_uninstall_keyboard();
    al_destroy_display(display);
    al_shutdown_primitives_addon();
    return 0;
}
