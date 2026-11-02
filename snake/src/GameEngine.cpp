#include "GameEngine.h"
#include "Graphics.h"
#include "FrameBuffer.h"
#include "Sound.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

namespace GameEngine {
    static const int GRID = 12;
    static const int COLS = TFT_W / GRID;
    static const int ROWS = TFT_H / GRID;
    static const int MAXLEN = 400;

    static int sx[MAXLEN], sy[MAXLEN];
    static int slen;
    static int dx, dy;
    static int fx, fy;
    static uint32_t score, hiScore;
    static bool gameOver;
    static int moveTimer, moveDelay;
    static bool started;

    static void place_food(void) {
        bool ok;
        do {
            ok = true;
            fx = rand() % COLS;
            fy = rand() % ROWS;
            for (int i = 0; i < slen; i++) {
                if (sx[i] == fx && sy[i] == fy) { ok = false; break; }
            }
        } while (!ok);
    }

    void init_game(void) {
        slen = 4;
        for (int i = 0; i < slen; i++) {
            sx[i] = 10 - i;
            sy[i] = 10;
        }
        dx = 1; dy = 0;
        score = 0;
        gameOver = false;
        moveTimer = 0;
        moveDelay = 8;
        place_food();
    }

    void update(void) {
        if (gameOver) {
            if (BTN_FIRE()) {
                if (score > hiScore) hiScore = score;
                init_game();
                sound_start();
            }
            return;
        }

        // Input
        if (BTN_UP() && dy == 0) { dx = 0; dy = -1; }
        if (BTN_DOWN() && dy == 0) { dx = 0; dy = 1; }
        if (BTN_LEFT() && dx == 0) { dx = -1; dy = 0; }
        if (BTN_RIGHT() && dx == 0) { dx = 1; dy = 0; }

        moveTimer++;
        if (moveTimer < moveDelay) return;
        moveTimer = 0;

        int nx = sx[0] + dx;
        int ny = sy[0] + dy;

        // Wrap
        if (nx < 0) nx = COLS - 1;
        if (nx >= COLS) nx = 0;
        if (ny < 0) ny = ROWS - 1;
        if (ny >= ROWS) ny = 0;

        // Self collision
        for (int i = 0; i < slen; i++) {
            if (sx[i] == nx && sy[i] == ny) {
                gameOver = true;
                sound_death();
                return;
            }
        }

        // Move
        for (int i = slen; i > 0; i--) {
            sx[i] = sx[i - 1];
            sy[i] = sy[i - 1];
        }
        sx[0] = nx;
        sy[0] = ny;

        // Eat
        if (nx == fx && ny == fy) {
            slen++;
            if (slen > MAXLEN) slen = MAXLEN;
            score += 10;
            sound_beep(1200, 30);

            place_food();
            if (score % 50 == 0 && moveDelay > 3) moveDelay--;
        }
    }

    static void draw_cell(int gx, int gy, uint16_t color) {
        Graphics::fill_rect(gx * GRID + 1, gy * GRID + 1, GRID - 2, GRID - 2, color);
    }

    void draw(void) {
        static FrameBuffer& fb = FrameBuffer::get();
        fb.clear(BLACK);

        // Food
        draw_cell(fx, fy, RED);

        // Snake
        for (int i = 0; i < slen; i++) {
            draw_cell(sx[i], sy[i], (i == 0) ? GREEN : DARK_GRAY);
        }

        // HUD
        Graphics::draw_string(5, 5, "SCORE", WHITE, BLACK, 1);
        Graphics::draw_number(50, 5, score, YELLOW, BLACK, 1);
        Graphics::draw_string(120, 5, "HI", WHITE, BLACK, 1);
        Graphics::draw_number(140, 5, hiScore, YELLOW, BLACK, 1);

        if (gameOver) {
            Graphics::draw_string(70, 110, "GAME OVER", RED, BLACK, 2);
            Graphics::draw_string(40, 140, "PRESS FIRE", WHITE, BLACK, 1);
        }

        fb.flush();
    }

    void game_loop(void) {
        srand(time(NULL));
        hiScore = 0;
        init_game();

        // Title
        Graphics::fill_screen(BLACK);
        Graphics::draw_string(50, 80, "SNAKE", GREEN, BLACK, 2);
        Graphics::draw_string(20, 160, "PRESS FIRE TO START", WHITE, BLACK, 1);
        FrameBuffer::get().flush();

        sound_start();
        while (!BTN_FIRE()) delay_ms(20);
        delay_ms(200);
        init_game();

        struct timespec last, now;
        clock_gettime(CLOCK_MONOTONIC, &last);

        while (true) {
            update();
            draw();

            clock_gettime(CLOCK_MONOTONIC, &now);
            long elapsed = (now.tv_sec - last.tv_sec) * 1000000L +
                           (now.tv_nsec - last.tv_nsec) / 1000L;
            if (elapsed < 16666L) delay_us(16666L - elapsed);
            clock_gettime(CLOCK_MONOTONIC, &last);
        }
    }
}
