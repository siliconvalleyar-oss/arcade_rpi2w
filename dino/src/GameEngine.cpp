#include "GameEngine.h"
#include "Graphics.h"
#include "FrameBuffer.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

namespace GameEngine {
    // Dino state
    static int16_t dinoY;
    static int16_t dinoVy;
    static bool onGround;
    static uint8_t dinoFrame;
    static uint8_t animTimer;

    // World
    static int16_t speed;
    static uint32_t groundScroll;
    static uint32_t score;
    static uint32_t hiScore;
    static bool gameOver;

    // Cactus
    static Cactus cactus[MAX_CACTUS];
    static int16_t cactusTimer;

    // Sprites
    Sprite dinoSprites[2];
    Sprite cactusSprites[2];
    bool spritesLoaded = false;

    bool load_sprites(void) {
        if (spritesLoaded) return true;
        const char* dinoFiles[2] = {"assets/dino_00.png","assets/dino_01.png"};
        for (int i = 0; i < 2; i++) {
            if (!dinoSprites[i].load(dinoFiles[i])) {
                fprintf(stderr, "Error loading %s\n", dinoFiles[i]);
                return false;
            }
        }
        const char* cactusFiles[2] = {"assets/cactus_00.png","assets/cactus_01.png"};
        for (int i = 0; i < 2; i++) {
            if (!cactusSprites[i].load(cactusFiles[i])) {
                fprintf(stderr, "Error loading %s\n", cactusFiles[i]);
                return false;
            }
        }
        spritesLoaded = true;
        fprintf(stderr, "Sprites: dino0=%dx%d dino1=%dx%d cactus0=%dx%d cactus1=%dx%d\n",
                dinoSprites[0].w, dinoSprites[0].h,
                dinoSprites[1].w, dinoSprites[1].h,
                cactusSprites[0].w, cactusSprites[0].h,
                cactusSprites[1].w, cactusSprites[1].h);
        return true;
    }

    static int dinoH() { return dinoSprites[dinoFrame].h; }
    static int dinoW() { return dinoSprites[dinoFrame].w; }
    static int cactusH(int t) { return cactusSprites[t].h; }
    static int cactusW(int t) { return cactusSprites[t].w; }
    static int dinoDrawY() { return GROUND_Y - dinoH(); }

    void init_game(void) {
        dinoY = GROUND_Y - dinoSprites[0].h;
        dinoVy = 0;
        onGround = true;
        dinoFrame = 0;
        animTimer = 0;
        speed = GAME_SPEED_INIT;
        groundScroll = 0;
        score = 0;
        gameOver = false;
        cactusTimer = 60;
        for (int i = 0; i < MAX_CACTUS; i++) cactus[i].active = false;
    }

    void update(void) {
        if (gameOver) {
            if (BTN_FIRE()) {
                if (score > hiScore) hiScore = score;
                init_game();
            }
            return;
        }

        // Jump
        if (BTN_FIRE() && onGround) {
            dinoVy = JUMP_VEL;
            onGround = false;
        }

        // Gravity
        if (!onGround) {
            dinoVy += GRAVITY;
            dinoY += dinoVy;
            int groundY = GROUND_Y - dinoH();
            if (dinoY >= groundY) {
                dinoY = groundY;
                dinoVy = 0;
                onGround = true;
            }
        }

        // Animation
        animTimer++;
        if (animTimer >= 8) {
            animTimer = 0;
            dinoFrame = (dinoFrame + 1) % 2;
        }

        // Scroll ground
        groundScroll += speed;

        // Score
        score += 1;

        // Speed increase every 500 points
        if (score > 0 && score % 500 == 0 && speed < 15) speed++;

        // Cactus spawn
        cactusTimer--;
        if (cactusTimer <= 0) {
            cactusTimer = 40 + rand() % 40;
            if (speed > 10) cactusTimer = 25 + rand() % 25;
            for (int i = 0; i < MAX_CACTUS; i++) {
                if (!cactus[i].active) {
                    cactus[i].active = true;
                    cactus[i].x = TFT_W + 10;
                    cactus[i].type = (rand() % 2 == 0) ? 0 : 1;
                    break;
                }
            }
        }

        // Move cactus
        for (int i = 0; i < MAX_CACTUS; i++) {
            if (!cactus[i].active) continue;
            cactus[i].x -= speed;
            if (cactus[i].x + cactusW(cactus[i].type) < 0)
                cactus[i].active = false;
        }

        // Collision: AABB with 4px margin
        int cx = DINO_X + 6;
        int cy = dinoDrawY() + 4;
        int cw = dinoW() - 12;
        int ch = dinoH() - 8;
        if (cw < 4) cw = 4;
        if (ch < 4) ch = 4;

        for (int i = 0; i < MAX_CACTUS; i++) {
            if (!cactus[i].active) continue;
            int t = cactus[i].type;
            int ox = cactus[i].x + 4;
            int oy = GROUND_Y - cactusH(t) + 4;
            int ow = cactusW(t) - 8;
            int oh = cactusH(t) - 8;
            if (ow < 4) ow = 4;
            if (oh < 4) oh = 4;

            if (cx < ox + ow && cx + cw > ox &&
                cy < oy + oh && cy + ch > oy) {
                gameOver = true;
                break;
            }
        }
    }

    static void draw_ground(void) {
        Graphics::fill_rect(0, GROUND_Y, TFT_W, 3, DARK_GRAY);
        Graphics::fill_rect(0, GROUND_Y + 3, TFT_W, 2, GRAY);

        // Pebbles scrolling
        for (int i = 0; i < 10; i++) {
            int px = ((i * 27 + 7) * 3 - (int)groundScroll) % (TFT_W + 20);
            if (px < 0) px += TFT_W + 20;
            if (px > TFT_W) continue;
            uint16_t c = (i % 3 == 0) ? LIGHT_GRAY : DARK_GRAY;
            Graphics::draw_pixel(px, GROUND_Y + 4, c);
            if (i % 2 == 0)
                Graphics::draw_pixel(px + 1, GROUND_Y + 5, c);
        }
    }

    void draw(void) {
        static FrameBuffer& fb = FrameBuffer::get();
        fb.clear(BLACK);

        draw_ground();

        // Cactus
        for (int i = 0; i < MAX_CACTUS; i++) {
            if (!cactus[i].active) continue;
            int t = cactus[i].type;
            Graphics::drawSprite(cactus[i].x, GROUND_Y - cactusH(t),
                                 cactusSprites[t], BLACK);
        }

        // Dino
        Graphics::drawSprite(DINO_X, dinoDrawY(),
                             dinoSprites[dinoFrame], BLACK);

        // HUD
        Graphics::draw_string(5, 5, "HI", GRAY, BLACK, 1);
        Graphics::draw_number(20, 5, hiScore, GRAY, BLACK, 1);
        Graphics::draw_number(100, 5, score, WHITE, BLACK, 1);

        char buf[16];
        snprintf(buf, sizeof(buf), "SPD:%d", speed);
        Graphics::draw_string(180, 5, buf, GRAY, BLACK, 1);

        if (gameOver) {
            Graphics::draw_string(65, 110, "GAME OVER", RED, BLACK, 2);
            Graphics::draw_string(35, 140, "PRESS FIRE", YELLOW, BLACK, 1);
        }

        fb.flush();
    }

    void game_loop(void) {
        srand(time(NULL));
        if (!load_sprites()) {
            fprintf(stderr, "FATAL: could not load sprites\n");
            return;
        }

        hiScore = 0;
        init_game();

        // Title screen
        Graphics::fill_screen(BLACK);
        Graphics::draw_string(30, 70, "DINO RUNNER", GREEN, BLACK, 2);
        Graphics::drawSprite((TFT_W - dinoSprites[0].w) / 2, 100,
                             dinoSprites[0], BLACK);
        Graphics::draw_string(25, 170, "PRESS FIRE TO START", YELLOW, BLACK, 1);
        FrameBuffer::get().flush();
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
