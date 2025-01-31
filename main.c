#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
void sleep(int milliseconds) { Sleep(milliseconds); }
#else
#include <unistd.h>
void sleep(int milliseconds) { usleep(milliseconds); }
#endif

#include <raylib.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600

#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 50
#define PLAYER_SPEED 5

#define BALL_RADIUS 5
#define INITIAL_BALL_SPEED 2

#define INIT_GAME_REFERENCES(game)                                             \
    struct Player *p1 = &game->p1;                                             \
    struct Player *p2 = &game->p2;                                             \
    struct Ball *ball = &game->ball;

struct Player {
    int speed;
    int pos_x;
    int pos_y;
    int points;
};

struct Ball {
    int pos_x;
    int pos_y;
    int speed_x;
    int speed_y;
};

enum State {
    Init,
    Pause,
    Run,
    Score,
    Reset,
};

struct SoundLibrary {
    Sound paddle_1;
    Sound paddle_2;
    Sound start_score;
};

struct Game {
    struct Player p1;
    struct Player p2;
    struct Ball ball;
    enum State state;
    struct SoundLibrary sounds;
    bool debug;
};

// Adds or subtracts from absolute value. (-5 + 3) = -8
int add_abs(int target, int add) {
    return target < 0 ? target - add : target + add;
}

void intToStr(int num, char *buffer, size_t size) {
    snprintf(buffer, size, "%d", num);
}

bool is_colliding(struct Player *player, struct Ball *ball) {
    return ball->pos_x >= player->pos_x &&
           ball->pos_x <= player->pos_x + PLAYER_WIDTH &&
           ball->pos_y >= player->pos_y &&
           ball->pos_y <= player->pos_y + PLAYER_HEIGHT;
}

void debug_lines(struct Game *game) {
    INIT_GAME_REFERENCES(game);

    // Draw static reference lines
    DrawLine(0, HEIGHT / 2, WIDTH, HEIGHT / 2, RED);
    DrawLine(0, HEIGHT / 8, WIDTH, HEIGHT / 8, RED);
    DrawLine(WIDTH / 4, 0, WIDTH / 4, HEIGHT, RED);
    DrawLine(WIDTH / 4 + WIDTH / 2, 0, WIDTH / 4 + WIDTH / 2, HEIGHT, RED);

    // Predict ball trajectory
    int temp_x = ball->pos_x;
    int temp_y = ball->pos_y;
    int temp_speed_x = ball->speed_x;
    int temp_speed_y = ball->speed_y;

    while (temp_x > 0 && temp_x < WIDTH) {
        int next_x = temp_x + temp_speed_x;
        int next_y = temp_y + temp_speed_y;

        // Check for ceiling & floor collisions
        if (next_y > HEIGHT - BALL_RADIUS || next_y < BALL_RADIUS) {
            temp_speed_y *= -1; // Reverse Y direction
        }

        // Check for paddle collisions
        if (is_colliding(p1, &(struct Ball){next_x, next_y, temp_speed_x,
                                            temp_speed_y}) ||
            is_colliding(p2, &(struct Ball){next_x, next_y, temp_speed_x,
                                            temp_speed_y})) {
            temp_speed_x *= -1; // Reverse X direction
        }

        // Draw the trajectory segment
        DrawLine(temp_x, temp_y, next_x, next_y, YELLOW);

        // Move the simulated ball
        temp_x = next_x;
        temp_y = next_y;
    }
}

void reset(struct Game *game) {
    struct Player p1 = {0, PLAYER_WIDTH, HEIGHT / 2 - PLAYER_HEIGHT / 2, 0};
    struct Player p2 = {0, WIDTH - 2 * PLAYER_WIDTH,
                        HEIGHT / 2 - PLAYER_HEIGHT / 2, 0};
    struct Ball ball = {WIDTH / 2, HEIGHT / 2, -INITIAL_BALL_SPEED, 2};
    enum State state = Init;

    game->state = state;
    game->ball = ball;
    game->p1 = p1;
    game->p2 = p2;
    PlaySound(game->sounds.start_score);
}

void draw(struct Game *game) {
    INIT_GAME_REFERENCES(game);

    // Middle line
    DrawLine(WIDTH / 2, HEIGHT, WIDTH / 2, 0, GRAY);

    // Draw Players
    DrawRectangle(p1->pos_x, p1->pos_y, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);
    DrawRectangle(p2->pos_x, p2->pos_y, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);

    // Draw Ball
    DrawCircle(ball->pos_x, ball->pos_y, BALL_RADIUS, WHITE);

    char p1_points[10], p2_points[10];
    intToStr(p1->points, p1_points, sizeof(p1_points));
    intToStr(p2->points, p2_points, sizeof(p2_points));

    // Draw Points
    DrawText(p1_points, WIDTH / 4 - 13, HEIGHT / 8 - 23, 50, WHITE);
    DrawText(p2_points, WIDTH / 4 + WIDTH / 2 - 13, HEIGHT / 8 - 23, 50, WHITE);

    DrawText("P: Pause\nD: Debug lines", 5, HEIGHT - 47, 20, GRAY);
}

void tick(struct Game *game) {
    INIT_GAME_REFERENCES(game);

    // Check for player collisions and increase the absolute value of the ball's
    // speed by 1
    int random = rand() % 8;
    int add = random == 1 ? 1 : 0;

    if (is_colliding(p1, ball)) {
        ball->speed_x = abs(ball->speed_x) + add;
        ball->speed_y = add_abs(ball->speed_y, add);
        PlaySound(game->sounds.paddle_1);
    }
    if (is_colliding(p2, ball)) {
        ball->speed_x = -abs(ball->speed_x + add);
        ball->speed_y = add_abs(ball->speed_y, add);
        PlaySound(game->sounds.paddle_2);
    }

    // Speed loop
    ball->pos_x += ball->speed_x;
    ball->pos_y += ball->speed_y;

    if ((p1->pos_y < HEIGHT - PLAYER_HEIGHT + 3 ||
         p1->speed == -PLAYER_SPEED) &&
        (p1->pos_y > 3 || p1->speed == PLAYER_SPEED))
        p1->pos_y += p1->speed;
    if ((p2->pos_y < HEIGHT - PLAYER_HEIGHT + 3 ||
         p2->speed == -PLAYER_SPEED) &&
        (p2->pos_y > 3 || p2->speed == PLAYER_SPEED))
        p2->pos_y += p2->speed;

    // Ball ceiling & floor collisions (Wall collision checks are in the points
    // function)
    if (ball->pos_y > HEIGHT - BALL_RADIUS || ball->pos_y < BALL_RADIUS)
        ball->speed_y *= -1;

    // Keybind collector
    // W⬆ S⬇ + UP & Down arrow keys for player 2
    // R resets (but is declared in main function)
    p1->speed = 0;
    p2->speed = 0;

    if (IsKeyDown(KEY_UP))
        p2->speed = -PLAYER_SPEED;
    if (IsKeyDown(KEY_DOWN))
        p2->speed = PLAYER_SPEED;

    if (IsKeyDown(KEY_W))
        p1->speed = -PLAYER_SPEED;
    if (IsKeyDown(KEY_S))
        p1->speed = PLAYER_SPEED;

    if (IsKeyPressed(KEY_D)) {
        game->debug = !game->debug;
    }
}

void points_system(struct Game *game) {
    INIT_GAME_REFERENCES(game);

    bool score_1 = ball->pos_x > WIDTH;
    bool score_2 = ball->pos_x < 2;

    if (score_1)
        p1->points += 1;
    if (score_2)
        p2->points += 1;

    if (score_1 || score_2) {
        game->state = Score;
        p1->pos_y = HEIGHT / 2 - PLAYER_HEIGHT / 2;
        p2->pos_y = HEIGHT / 2 - PLAYER_HEIGHT / 2;

        struct Ball new_ball = {WIDTH / 2, HEIGHT / 2,
                                -((ball->speed_x > 0) ? 2 : -2),
                                -((ball->speed_y > 0) ? 2 : -2)};
        game->ball = new_ball;
    }
}

void draw_paused() {
    ClearBackground(BLACK);
    DrawText("PAUSE", WIDTH / 4 + 30, HEIGHT / 2 - 50, 100, RAYWHITE);
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "PONG! this time, I made it in C!");
    SetTargetFPS(120);

    //            ⬇ This suppresses a warning
    srand(
        (unsigned int)time(NULL)); // Predictable? Yes, but it's a pong game :)

    struct Player p1 = {0, PLAYER_WIDTH, HEIGHT / 2 - PLAYER_HEIGHT / 2, 0};
    struct Player p2 = {0, WIDTH - 2 * PLAYER_WIDTH,
                        HEIGHT / 2 - PLAYER_HEIGHT / 2, 0};
    struct Ball ball = {WIDTH / 2, HEIGHT / 2, -INITIAL_BALL_SPEED, 2};
    enum State state = Init;

    InitAudioDevice();

    Sound paddle_1 = LoadSound("paddle_1.mp3");
    Sound paddle_2 = LoadSound("paddle_2.mp3");
    Sound start_score = LoadSound("pong_start-score.mp3");

    struct SoundLibrary sounds = {paddle_1, paddle_2, start_score};

    struct Game game = {p1, p2, ball, state, sounds, false};

    bool key_p_pressed = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R))
            reset(&game);

        if (IsKeyDown(KEY_P)) {
            if (!key_p_pressed) {
                key_p_pressed = true;
                if (game.state == Pause) {
                    game.state = Run;
                } else if (game.state == Run) {
                    game.state = Pause;
                }
            }
        } else {
            key_p_pressed = false;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        points_system(&game);
        draw(&game);
        if (game.debug)
            debug_lines(&game);
        if (game.state != Pause)
            tick(&game);
        else
            draw_paused();

        EndDrawing();

        if (game.state == Init || game.state == Score) {
            PlaySound(game.sounds.start_score);
            sleep(3000);
            game.state = Run;
        }
    }

    UnloadSound(paddle_1);
    UnloadSound(paddle_2);
    UnloadSound(start_score);

    CloseAudioDevice();

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
#endif