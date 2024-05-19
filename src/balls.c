#include "raylib.h"
#include "raymath.h"


#ifndef PLATFORM_WEB

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#else

#define FLT_MAX 3.402823466e+38F
#define RAND_MAX 2147483647 
int rand(void);
double fmod(double x, double y);
double pow(double x, double y);

#endif // PLATFORM_WEB

// Gruvbox Palette 
// ------------------------------------------------------------
#define GRUVBOX_RED     CLITERAL(Color){0xFB, 0x49, 0x34, 0xFF}
#define GRUVBOX_GREEN   CLITERAL(Color){0xB8, 0xbb, 0x26, 0xFF}
#define GRUVBOX_YELLOW  CLITERAL(Color){0xFA, 0xBD, 0x2F, 0xFF}
#define GRUVBOX_BLUE    CLITERAL(Color){0x83, 0xA5, 0x98, 0xFF}
#define GRUVBOX_PURPLE  CLITERAL(Color){0xD3, 0x86, 0x9B, 0xFF}
#define GRUVBOX_AQUA    CLITERAL(Color){0x8E, 0xC0, 0x7C, 0xFF}
#define GRUVBOX_ORANGE  CLITERAL(Color){0xFE, 0x80, 0x19, 0xFF}
// -------------------------------------------------------------

#define BACKGROUND_COLOR CLITERAL(Color){0x18, 0x18, 0x18, 0xFF}

#define PARTICLES 25

#define WIDTH 1500
#define HEIGHT 800

static int width = WIDTH;
static int height = HEIGHT;

// Settings
// --------------------------------------

// --------------------------------------

static Color colors[] = {GRUVBOX_RED, GRUVBOX_YELLOW, GRUVBOX_BLUE, GRUVBOX_PURPLE, GRUVBOX_AQUA, GRUVBOX_ORANGE, GRUVBOX_GREEN};
//static Color colors[] = {RED, YELLOW, GREEN, PURPLE, BROWN, PINK, ORANGE, GOLD, BLUE, VIOLET};
static int colors_count = sizeof(colors)/sizeof(*colors);


typedef struct {
    Vector2 pos;
    Vector2 velocity;
    float max_lifetime;
    float lifetime;
    float radius;
    Color color;
} Particle;

static Particle particles[PARTICLES];


typedef enum {
    TYPE_RED = 0,
    TYPE_YELLOW,
    TYPE_GREEN,
    TYPE_BLUE,
    TYPE_BROWN,
    TYPE_PURPLE,
    TYPE_GOLD,
    TYPE_NONE,
} BallType;

typedef struct {
    BallType type;
    float timer;
} Ball;

#define BALLS 6
#define BOTTLES (TYPE_NONE + 2)

#define BALL_PAD 5
#define BALL_TOP_PAD 20
#define BALL_RADIUS 20
#define BOTTLE_PAD 45
#define BOTTLE_WIDTH (BALL_RADIUS*2 + BALL_PAD*2)
#define BOTTLE_HEIGHT (BALL_RADIUS*BALLS*2 + BALL_TOP_PAD)
#define FIELD_WIDTH (BOTTLE_WIDTH*BOTTLES + BOTTLE_PAD*BOTTLES)
#define TEXT_PAD 50

typedef struct {
    Ball balls[BALLS];
} Bottle;

static Bottle bottles[BOTTLES] = {0}; 
static int prev_selected_bottle = -1;
static int selected_bottle = -1;


typedef enum {
    GAME_PLAY = 0,
    GAME_WIN,
    GAME_BALL_MOVE,
    GAME_BALL_UP,
} GameState;

static float timer = 0.0f;
static GameState state = GAME_PLAY;


typedef struct {
    int from_bottle;
    int from_index;
    int to_bottle;
    int to_index;
} Move;

static Move prev_move = {0};
static Move current_move = {0};


void set_current_move(int from_bottle, int from_index, int to_bottle, int to_index)
{
    current_move.from_bottle = from_bottle;
    current_move.from_index  = from_index;
    current_move.to_bottle   = to_bottle;
    current_move.to_index    = to_index;
}


BallType get_ball_type(bool reset)
{
    static int types[TYPE_NONE] = {0};
    static bool types_inited = false;
    if (reset == true) types_inited = false;
    if (!types_inited) {
        for (int i = 0; i < TYPE_NONE; ++i) types[i] = BALLS;
        if (reset == true) return TYPE_NONE;
        types_inited = true;
    }

    BallType type; 
    do {
      type = rand() % TYPE_NONE; 
    } while (types[type] == 0);

    types[type] -= 1;

    return type; 
}


void init_bottles(void)
{
    for (int i = 0; i < BALLS; ++i) {
        bottles[0].balls[i].type = TYPE_NONE;
        bottles[1].balls[i].type = TYPE_NONE;
    }
    
    get_ball_type(true);
    for (int i = 2; i < BOTTLES; ++i) {
        for (int j = 0; j < BALLS; ++j) {
            bottles[i].balls[j].type = get_ball_type(false);
        }
    }
}
















void init_mouse_particles(void)
{
    for (int i = 0; i < PARTICLES; ++i) {
        particles[i].lifetime = 0.0f;
    }
}


int get_free_particle_index(void)
{
    for (int i = 0; i < PARTICLES; ++i) {
        if (particles[i].lifetime <= 0.0f) return i;
    }
   return -1; 
}


void rand_mouse_particle(Vector2 pos)
{

    int index = get_free_particle_index();
    if (index < 0) return;
    Particle *particle = &particles[index];
    particle->pos = pos;
    particle->radius = 3 + rand() % 5;
    particle->lifetime = (float)rand() / (float)RAND_MAX;
    particle->max_lifetime = particle->lifetime; 
    if (true) {
        Vector2 mouse_delta = Vector2Normalize(GetMouseDelta());
        Vector2 rand_vec = {.x = 10 + rand() % 100, .y = 10 + rand() % 100}; 
        particle->velocity = Vector2Multiply(Vector2Negate(mouse_delta), rand_vec);
    } else {
        particle->velocity.x = -100 + rand() % 200;
        particle->velocity.y = -100 + rand() % 200;
    }
    particle->color = colors[rand() % colors_count];
}


void update_particle_pos(Particle particles[], int particles_size, int index, float dt)
{
    if (index < 0 || index >= particles_size) return;
    Particle *particle = &particles[index];

    float x = particle->pos.x + particle->velocity.x*dt;
    if (x - particle->radius < 0 || x + particle->radius > width) {
        particle->velocity.x *= -1;
    } else {
        particle->pos.x = x;
    }

    float y = particle->pos.y + particle->velocity.y*dt;
    if (y - particle->radius < 0 || y + particle->radius > height) {
        particle->velocity.y *= -1;
    } else {
        particle->pos.y = y;
    }
}


void draw_particles(Particle particles[], int particles_count, float dt)
{
    for (int i = 0; i < particles_count; ++i) {
        Particle *particle = &particles[i];
        if (particle->lifetime <= 0) continue;
        float value = particle->lifetime / particle->max_lifetime;
        DrawCircleV(particle->pos, particle->radius, ColorAlpha(particle->color, value));
        update_particle_pos(particles, particles_count, i, dt);
        particle->lifetime -= dt;
    }
}


void draw_switch(const char *text, int x, int y, bool *value) {
    Vector2 switch_pos = {.x = x, .y = y};
    int outer_radius = 10;
    int inner_radius = outer_radius-5;
    int text_size = 20;
    int text_x = switch_pos.x + outer_radius + 10;
    int text_y = switch_pos.y - text_size/2;
    
    DrawCircleV(switch_pos, outer_radius, BLACK);
    DrawText(text, text_x, text_y, text_size, RED);

    if (*value) {
        DrawCircleV(switch_pos, inner_radius, GREEN);
    } else {
        DrawCircleV(switch_pos, inner_radius, RED);
    }
    
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointCircle(mouse, switch_pos, outer_radius) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *value = !(*value);
    }
}

void draw_menu(void) 
{
    //int x = 20, y = 20;
/*    
    // Draw radius slidebar controls   
    Rectangle max_radius_slider = {x, y, 200, 10};
    int MAX_RADIUS_SLIDEBAR_RADIUS = 6;
    int cx = x + Lerp(0.0f, 100.0f, (float)circle_radius_max/100);
    int cy = y + 10/2;
    DrawRectangleRec(max_radius_slider, RED);
    DrawCircle(cx, cy, MAX_RADIUS_SLIDEBAR_RADIUS, BLACK); 
    procees_slider(cx, cy);
*/
    //draw_switch("Pop on collision", x, y, &pop_on_collision);
    //draw_switch("Based mouse particle velocity on mouse delta", x, y + 30, &based_mouse_particle_velocity_on_mouse_delta);
    
}


bool move_ball(int from, int to)
{
    int from_ball_index = -1;
    BallType from_ball_type = TYPE_NONE;
    for (int i = 0; i < BALLS; ++i) {
        if (bottles[from].balls[i].type == TYPE_NONE) continue; 
        from_ball_type = bottles[from].balls[i].type; 
        from_ball_index = i;
        break;
    }
    if (from_ball_type == TYPE_NONE) return false;

    int to_ball_index = -1;
    for (int i = 0; i < BALLS; ++i) {
        if (bottles[to].balls[i].type == TYPE_NONE) {
            if (bottles[to].balls[i+1].type == from_ball_type || i == BALLS - 1) {
                to_ball_index = i;
                break;
            }
        }
    }
    if (to_ball_index == -1) return false;
    
    set_current_move(from, from_ball_index, to, to_ball_index); 
    state = GAME_BALL_MOVE;
    timer = 0.5f;
    bottles[to].balls[to_ball_index].type = from_ball_type;
    bottles[from].balls[from_ball_index].type = TYPE_NONE;
    return true;
}


void check_win(void)
{
    for (int i = 0; i < BOTTLES; ++i) {
        BallType type = bottles[i].balls[0].type;
        for (int j = 1; j < BALLS; ++j) {
            if (bottles[i].balls[j].type != type) return;
        }
    }
    state = GAME_WIN; 
}


void draw_balls_move(int bottle, int x, int y) 
{
    for (int j = 0; j < BALLS; ++j) { 
        if (bottle == current_move.from_bottle && j == current_move.from_index) continue;
        if (bottle == current_move.to_bottle && j == current_move.to_index) continue;
        if (bottles[bottle].balls[j].type == TYPE_NONE) continue;
        Color color = colors[bottles[bottle].balls[j].type];
        int bx = x + BALL_RADIUS + BALL_PAD;
        int by = y + BALL_RADIUS + j*BALL_RADIUS*2 + BALL_TOP_PAD - 3;
        DrawCircle(bx, by, BALL_RADIUS, color);
    }
    
    if (bottle != current_move.from_bottle) return; 

    if (timer > 0.25) {
        int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
        int from_x = x + BALL_RADIUS + BALL_PAD;
        int to_x = sx + current_move.to_bottle*BOTTLE_WIDTH + current_move.to_bottle*BOTTLE_PAD + BALL_RADIUS + BALL_PAD;
        int by = y - BALL_RADIUS - BALL_PAD; 
        int bx = 0;
        if (to_x > from_x) {
            bx = Lerp(from_x, to_x, (0.5 - timer)/0.25);
        } else {
            bx = Lerp(to_x, from_x, (0.25 - (0.5 - timer))/0.25);
        } 
        Color color = colors[bottles[current_move.to_bottle].balls[current_move.to_index].type];
        DrawCircle(bx, by, BALL_RADIUS, color);
    } else {
        int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
        int from_y = y - BALL_RADIUS - BALL_PAD; 
        int to_y = y + BALL_RADIUS + current_move.to_index*BALL_RADIUS*2 + BALL_TOP_PAD - 3;
        int by = Lerp(to_y, from_y, timer/0.25);
        int bx = sx + current_move.to_bottle*BOTTLE_WIDTH + current_move.to_bottle*BOTTLE_PAD + BALL_RADIUS + BALL_PAD;
        Color color = colors[bottles[current_move.to_bottle].balls[current_move.to_index].type];
        DrawCircle(bx, by, BALL_RADIUS, color);
    }
    timer -= GetFrameTime();
    if (timer <= 0.0f) {
        state = GAME_PLAY;
        prev_move = current_move;
    }
}



void draw_balls(int bottle, int x, int y) 
{
    bool first = true;
    for (int j = 0; j < BALLS; ++j) {
        if (bottles[bottle].balls[j].type == TYPE_NONE) continue;
        Color color = colors[bottles[bottle].balls[j].type];
        int bx = x + BALL_RADIUS + BALL_PAD;
        int by = y + BALL_RADIUS + j*BALL_RADIUS*2 + BALL_TOP_PAD - 3;
        if (first && selected_bottle == bottle) {
            int max_by = y - BALL_RADIUS - BALL_PAD;
            if (bottles[bottle].balls[j].timer < 0.15f)
            {
                float time = bottles[bottle].balls[j].timer / 0.15f;
                by = Lerp(by, max_by, time);
                bottles[bottle].balls[j].timer += GetFrameTime();
            } else {
                by = max_by;
            }
        } else if (first && prev_selected_bottle == bottle) {
            int max_by = y - BALL_RADIUS - BALL_PAD;
            if (bottles[bottle].balls[j].timer < 0.15f)
            {
                float time = bottles[bottle].balls[j].timer;
                by = Lerp(by, max_by, (0.15f - time)/0.15f);
                bottles[bottle].balls[j].timer += GetFrameTime();
            }
        }
        DrawCircle(bx, by, BALL_RADIUS, color);
        first = false;
    }
}


void set_first_not_none_ball_timer(int bottle)
{
      for (int i = 0; i < BALLS; i++) {
          if (bottles[bottle].balls[i].type == TYPE_NONE) continue;
          bottles[bottle].balls[i].timer = 0.0f;
          break;
      }
}

void process_mouse_click(Rectangle bottle_rec, int bottle)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), bottle_rec)) {
        if (selected_bottle == bottle) { 
            prev_selected_bottle = bottle;
            set_first_not_none_ball_timer(bottle);
            selected_bottle = -1; 
        } else if (selected_bottle == -1) {
            prev_selected_bottle = -1;
            selected_bottle = bottle;            
            set_first_not_none_ball_timer(bottle);
        } else {
            if (move_ball(selected_bottle, bottle)) {
                prev_selected_bottle = -1; 
                check_win();
            } else {
                prev_selected_bottle = selected_bottle;
                set_first_not_none_ball_timer(selected_bottle);
            }
            selected_bottle = -1; 
        }
    }
}


void draw_cancel_move_button(void)
{
    Rectangle button_rec = {50, 50, 200, 50};
    DrawRectangleRec(button_rec, WHITE);
    DrawText("Cancel move", 60, 60, 30, BLACK);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_rec)) {
        if (prev_move.to_bottle == -1) return;
        BallType ball = bottles[prev_move.to_bottle].balls[prev_move.to_index].type; 
        bottles[prev_move.to_bottle].balls[prev_move.to_index].type = TYPE_NONE;
        bottles[prev_move.from_bottle].balls[prev_move.from_index].type = ball;
        prev_move = (Move){-1, -1, -1, -1};
    }
}


void draw_bottles(void) 
{
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - BOTTLE_HEIGHT/2;
    for (int i = 0; i < BOTTLES; ++i) {
        int x = sx + i*BOTTLE_WIDTH + i*BOTTLE_PAD;
        Rectangle bottle_rec = {x, sy, BOTTLE_WIDTH, BOTTLE_HEIGHT};
        DrawRectangleLinesEx(bottle_rec, 3, GRUVBOX_RED);
        switch (state) {
            case GAME_PLAY: {
                draw_balls(i, x, sy); 
                process_mouse_click(bottle_rec, i);      
                break;
            } 
            case GAME_BALL_MOVE: {
                draw_balls_move(i, x, sy); 
                break;
            }
            // TODO: check spelling
            default: break;// assert("Unreacheble");
        }
    }
    draw_cancel_move_button();
}

float in_out_cubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

void draw_win_screen(void)
{
    static float color_time = 0.0f;
    float color_time_max    = 5.0f;

    static float position_time     = 0.25f;
    float position_time_max        = 0.5f; 
    static bool position_time_back = true;

    const char *text_win     = "You win!";
    const char *text_restart = "Press [R] for restart!";
    int text_win_size = 100;
    int text_restart_size = 50;
    int text_win_x     = GetScreenWidth()/2 - MeasureText(text_win,     text_win_size    )/2;
    int text_restart_x = GetScreenWidth()/2 - MeasureText(text_restart, text_restart_size)/2;
  
    float dt = GetFrameTime();
    if (!position_time_back) {
        position_time += dt;
        if (position_time >= position_time_max) position_time_back = true; 
    } else {
        position_time -= dt;
        if (position_time <= 0) position_time_back = false; 
    }

    color_time = fmod(color_time + dt, color_time_max);
    float hue = Lerp(0, 360, color_time/color_time_max);
    Color color = ColorFromHSV(hue, 1.0f, 1.0f); 

    int min_text_y = GetScreenHeight()/2 - (text_win_size + text_restart_size - TEXT_PAD)/2 - 25;
    int max_text_y = GetScreenHeight()/2 - (text_win_size + text_restart_size - TEXT_PAD)/2 + 25;
    int text_y = Lerp(min_text_y, max_text_y, in_out_cubic(position_time/position_time_max));

    int text_win_y     = text_y - text_win_size     + TEXT_PAD/2;
    int text_restart_y = text_y + text_restart_size - TEXT_PAD/2;

    DrawText(text_win, text_win_x, text_win_y, text_win_size, color);
    DrawText(text_restart, text_restart_x, text_restart_y, text_restart_size, color);
}


void game_frame(void)
{
    BeginDrawing();
        height = GetScreenHeight();
        width = GetScreenWidth();
        ClearBackground(BACKGROUND_COLOR);
        if (IsKeyPressed(KEY_R) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && state == GAME_WIN)) {
            state = GAME_PLAY;
            prev_move = (Move){-1, -1, -1, -1};
            current_move = (Move){-1, -1, -1, -1};
            init_bottles();
        }
        switch (state) {
            case GAME_WIN:       draw_win_screen(); break;
            case GAME_PLAY:      draw_bottles();    break;
            case GAME_BALL_MOVE: draw_bottles();    break;
            default:                               break;
        }
    EndDrawing();
}


void raylib_js_set_entry(void (*entry)(void));


int main(void)
{
    #ifndef PLATFORM_WEB
        srand(time(NULL));
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    #endif

    //InitWindow(WIDTH, HEIGHT, "Balls");
    InitWindow(0, 0, "Balls");
    SetTargetFPS(60);
    width = GetScreenWidth();
    height = GetScreenHeight();
    //init_mouse_particles();
    init_bottles();
    prev_move = (Move){-1, -1, -1, -1};
    current_move = (Move){-1, -1, -1, -1};

#ifdef PLATFORM_WEB
    raylib_js_set_entry(game_frame);
#else 
    while (!WindowShouldClose()) {
            game_frame();
    }
    CloseWindow();
#endif

    return 0;
}
