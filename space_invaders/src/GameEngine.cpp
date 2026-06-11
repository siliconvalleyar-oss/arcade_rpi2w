#include "GameEngine.h"
#include "Graphics.h"
#include "Sound.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

namespace GameEngine {
    int16_t player_x = 90;
    int selectedShip = 0;
    bool game_active = true;
    uint16_t score = 0, high_score = 0;
    uint8_t lives = 3, level = 1, invaders_remaining = 0;

    Invader invaders[NUM_INVADERS_X][NUM_INVADERS_Y];
    Bullet bullets[10];
    Explosion explosions[20];

    static int8_t invader_dir = 1;
    static uint16_t step_timer = 0, shoot_timer = 0;
    static const int START_X = 5, START_Y = 2, SPACING_X = 45, SPACING_Y = 48;

    // ========== Sprites ==========
    Sprite playerSprites[5];
    Sprite enemySprites[3];
    Sprite bulletSprite;
    Sprite impactSprite;
    bool spritesLoaded = false;

    bool load_sprites(void) {
        if (spritesLoaded) return true;
        const char* playerFiles[5] = {"assets/nave_00.png","assets/nave_01.png","assets/nave_02.png","assets/nave_03.png","assets/nave_04.png"};
        for (int i = 0; i < 5; i++) {
            if (!playerSprites[i].load(playerFiles[i])) {
                fprintf(stderr, "Error: no se pudo cargar %s\n", playerFiles[i]);
                return false;
            }
        }
        const char* enemyFiles[3] = {"assets/enemy_00.png","assets/enemy_01.png","assets/enemy_02.png"};
        for (int i = 0; i < 3; i++) {
            if (!enemySprites[i].load(enemyFiles[i])) {
                fprintf(stderr, "Error: no se pudo cargar %s\n", enemyFiles[i]);
                return false;
            }
        }
        if (!bulletSprite.load("assets/disparo_de_nave_00.png")) {
            fprintf(stderr, "Error: no se pudo cargar disparo_de_nave_00.png\n");
            return false;
        }
        if (!impactSprite.load("assets/impact_00.png")) {
            fprintf(stderr, "Error: no se pudo cargar impact_00.png\n");
            return false;
        }
        spritesLoaded = true;
        fprintf(stderr, "Sprites loaded: player=%dx%d enemy=%dx%d bullet=%dx%d impact=%dx%d\n",
                playerSprites[0].w, playerSprites[0].h,
                enemySprites[0].w, enemySprites[0].h,
                bulletSprite.w, bulletSprite.h,
                impactSprite.w, impactSprite.h);
        return true;
    }

    void init_game() {
        player_x = TFT_W/2 - playerSprites[selectedShip].w / 2;
        invaders_remaining = 0;
        for(int i=0;i<NUM_INVADERS_X;i++) {
            for(int j=0;j<NUM_INVADERS_Y;j++) {
                invaders[i][j].x = START_X + i*SPACING_X;
                invaders[i][j].y = START_Y + j*SPACING_Y;
                invaders[i][j].active = true;
                invaders[i][j].type = (j<1)?1:2;
                invaders_remaining++;
            }
        }
        for(int i=0;i<10;i++) bullets[i].active = false;
        for(int i=0;i<20;i++) explosions[i].active = false;
        invader_dir = 1; step_timer = 0; shoot_timer = 0;
    }

    void update_player() {
        int pw = playerSprites[selectedShip].w;
        int ph = playerSprites[selectedShip].h;
        if(BTN_LEFT() && player_x > 0) player_x -= PLAYER_SPEED;
        if(BTN_RIGHT() && player_x < TFT_W - pw) player_x += PLAYER_SPEED;
        static bool last_fire=false;
        bool fire = BTN_FIRE();
        if(fire && !last_fire) {
            for(int i=0;i<10;i++) {
                if(!bullets[i].active) {
                    bullets[i].x = player_x + (pw - bulletSprite.w) / 2;
                    bullets[i].y = PLAYER_Y + PLAYER_HEIGHT - ph - bulletSprite.h;
                    bullets[i].active = true;
                    bullets[i].is_player = true;
                    sound_shoot();
                    break;
                }
            }
        }
        last_fire = fire;
    }

    void update_bullets() {
        for(int i=0;i<10;i++) {
            if(bullets[i].active) {
                if(bullets[i].is_player) {
                    bullets[i].y -= BULLET_SPEED;
                    if(bullets[i].y < 0) bullets[i].active = false;
                } else {
                    bullets[i].y += BULLET_SPEED;
                    if(bullets[i].y > TFT_H) bullets[i].active = false;
                }
            }
        }
    }

    void update_invaders() {
        step_timer++;
        if(step_timer >= (30/level)) {
            step_timer = 0;
            bool change = false;
            for(int i=0;i<NUM_INVADERS_X;i++)
                for(int j=0;j<NUM_INVADERS_Y;j++)
                    if(invaders[i][j].active) {
                        invaders[i][j].x += INVADER_SPEED * invader_dir;
                        if(invaders[i][j].x <=5 || invaders[i][j].x >= TFT_W-INVADER_WIDTH-5) change = true;
                    }
            if(change) {
                invader_dir *= -1;
                for(int i=0;i<NUM_INVADERS_X;i++)
                    for(int j=0;j<NUM_INVADERS_Y;j++)
                        if(invaders[i][j].active) {
                            invaders[i][j].y += 8;
                            invaders[i][j].x += INVADER_SPEED * invader_dir * 2;
                            if(invaders[i][j].y + INVADER_HEIGHT >= PLAYER_Y) game_active = false;
                        }
            }
        }
        shoot_timer++;
        if(shoot_timer >= (40 - level*2)) {
            shoot_timer = 0;
            int list[50][2], cnt=0;
            for(int i=0;i<NUM_INVADERS_X;i++)
                for(int j=0;j<NUM_INVADERS_Y;j++)
                    if(invaders[i][j].active) { list[cnt][0]=i; list[cnt][1]=j; cnt++; }
            if(cnt>0) {
                int idx = rand()%cnt;
                int i = list[idx][0], j = list[idx][1];
                for(int b=0;b<10;b++) {
                    if(!bullets[b].active) {
                        bullets[b].x = invaders[i][j].x + INVADER_WIDTH/2 - BULLET_WIDTH/2;
                        bullets[b].y = invaders[i][j].y + INVADER_HEIGHT;
                        bullets[b].active = true;
                        bullets[b].is_player = false;
                        break;
                    }
                }
            }
        }
    }

    void update_explosions() {
        for(int i=0;i<20;i++) {
            if(explosions[i].active) {
                explosions[i].frame++;
                if(explosions[i].frame > 4) explosions[i].active = false;
            }
        }
    }

    void check_collisions() {
        for(int b=0;b<10;b++) {
            if(bullets[b].active && bullets[b].is_player) {
                for(int i=0;i<NUM_INVADERS_X;i++) {
                    for(int j=0;j<NUM_INVADERS_Y;j++) {
                        if(invaders[i][j].active &&
                           bullets[b].x+BULLET_WIDTH > invaders[i][j].x &&
                           bullets[b].x < invaders[i][j].x+INVADER_WIDTH &&
                           bullets[b].y+BULLET_HEIGHT > invaders[i][j].y &&
                           bullets[b].y < invaders[i][j].y+INVADER_HEIGHT) {
                            bullets[b].active = false;
                            invaders[i][j].active = false;
                            invaders_remaining--;
                            int pts = (invaders[i][j].type==1)?30:(invaders[i][j].type==2)?20:10;
                            score += pts;
                            for(int e=0;e<20;e++) {
                                if(!explosions[e].active) {
                                    explosions[e].x = invaders[i][j].x + INVADER_WIDTH/2;
                                    explosions[e].y = invaders[i][j].y + INVADER_HEIGHT/2;
                                    explosions[e].frame = 0;
                                    explosions[e].active = true;
                                    break;
                                }
                            }
                            sound_invader_explosion();
                            if(invaders_remaining == 0) {
                                level++; sound_level_up(); init_game();
                            }
                            break;
                        }
                    }
                }
            }
            if(bullets[b].active && !bullets[b].is_player) {
                int pw = playerSprites[selectedShip].w;
                int ph = playerSprites[selectedShip].h;
                int pTop = PLAYER_Y + PLAYER_HEIGHT - ph;
                if(bullets[b].x+BULLET_WIDTH > player_x && bullets[b].x < player_x+pw &&
                   bullets[b].y+BULLET_HEIGHT > pTop && bullets[b].y < pTop+ph) {
                    bullets[b].active = false;
                    lives--;
                    sound_player_explosion();
                    if(lives==0) {
                        game_active = false;
                        if(score>high_score) high_score = score;
                        sound_game_over();
                    } else {
                        player_x = TFT_W/2 - playerSprites[selectedShip].w / 2;
                        for(int i=0;i<10;i++) bullets[i].active = false;
                        delay_ms(1000);
                    }
                }
            }
        }
    }

    // ========== Drawing ==========
    void draw_game() {
        Graphics::draw_background();

        // Draw invaders as PNG sprites
        for(int i=0;i<NUM_INVADERS_X;i++)
            for(int j=0;j<NUM_INVADERS_Y;j++)
                if(invaders[i][j].active) {
                    int t = invaders[i][j].type - 1; // 0,1,2
                    Graphics::drawSprite(invaders[i][j].x, invaders[i][j].y,
                                         enemySprites[t], BLACK);
                }

        // Bullets
        for(int i=0;i<10;i++)
            if(bullets[i].active)
                Graphics::drawSprite(bullets[i].x, bullets[i].y, bulletSprite, BLACK);

        // Explosions with impact sprite
        for(int i=0;i<20;i++)
            if(explosions[i].active)
                Graphics::drawSprite(explosions[i].x - impactSprite.w/2,
                                     explosions[i].y - impactSprite.h/2,
                                     impactSprite, BLACK);

        // Player (selected ship, static)
        int playerDrawY = PLAYER_Y + PLAYER_HEIGHT - playerSprites[selectedShip].h;
        Graphics::drawSprite(player_x, playerDrawY, playerSprites[selectedShip], BLACK);

        // HUD
        Graphics::draw_string(5,5,"SCORE:",WHITE,BLACK,1);
        Graphics::draw_number(50,5,score,YELLOW,BLACK,1);
        Graphics::draw_string(130,5,"HIGH:",WHITE,BLACK,1);
        Graphics::draw_number(170,5,high_score,CYAN,BLACK,1);
        Graphics::draw_string(5,TFT_H-12,"LIVES:",WHITE,BLACK,1);
        Graphics::draw_number(52,TFT_H-12,lives,WHITE,BLACK,1);
        Graphics::draw_string(170,TFT_H-12,"LVL:",WHITE,BLACK,1);
        Graphics::draw_number(200,TFT_H-12,level,GREEN,BLACK,1);
    }

    void show_title() {
        Graphics::fill_screen(BLACK);
        Graphics::draw_string(40,50,"SPACE",GREEN,BLACK,3);
        Graphics::draw_string(25,85,"INVADERS",RED,BLACK,3);
        Graphics::drawSprite(20,130,enemySprites[0],BLACK);
        Graphics::drawSprite(75,130,enemySprites[1],BLACK);
        Graphics::drawSprite(130,130,enemySprites[2],BLACK);
        Graphics::drawSprite(185,130,enemySprites[0],BLACK);
        Graphics::draw_string(30,170,"PRESS ANY BUTTON",YELLOW,BLACK,2);
        Graphics::draw_string(30,200,"LEFT/RIGHT MOVE",WHITE,BLACK,1);
        Graphics::draw_string(50,215,"FIRE = SHOOT",WHITE,BLACK,1);
        Graphics::flush_buffer();
        sound_start();
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t start = ts.tv_sec;
        while(1) {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            if(ts.tv_sec - start >= 3) break;
            if(BTN_LEFT() || BTN_RIGHT() || BTN_FIRE()) break;
            delay_ms(50);
        }
        delay_ms(500);
    }

    void show_game_over() {
        Graphics::fill_screen(BLACK);
        Graphics::draw_string(50,80,"GAME OVER",RED,BLACK,3);
        Graphics::draw_string(30,130,"FINAL SCORE:",WHITE,BLACK,2);
        Graphics::draw_number(180,130,score,YELLOW,BLACK,2);
        if(score == high_score && score > 0) Graphics::draw_string(40,170,"NEW HIGH SCORE!",CYAN,BLACK,1);
        Graphics::draw_string(35,200,"PRESS FIRE",YELLOW,BLACK,1);
        Graphics::flush_buffer();
        while(!BTN_FIRE()) delay_ms(50);
        delay_ms(300);
    }

    void show_ship_select() {
        Graphics::fill_screen(BLACK);
        int lastLeft = 0, lastRight = 0;
        bool confirmed = false;
        while (!confirmed) {
            Graphics::fill_screen(BLACK);
            Graphics::draw_string(20, 10, "SELECCIONA NAVE", CYAN, BLACK, 2);
            int sx = (TFT_W - playerSprites[selectedShip].w) / 2;
            int sy = 54;
            Graphics::drawSprite(sx, sy, playerSprites[selectedShip], BLACK);
            Graphics::draw_string(55, TFT_H-70, "NAVE", WHITE, BLACK, 2);
            Graphics::draw_number(105, TFT_H-70, selectedShip+1, YELLOW, BLACK, 2);
            Graphics::draw_string(130, TFT_H-70, "/5", WHITE, BLACK, 2);
            Graphics::draw_string(20, TFT_H-45, "<- ANTERIOR", YELLOW, BLACK, 1);
            Graphics::draw_string(155, TFT_H-45, "SIGUIENTE ->", YELLOW, BLACK, 1);
            Graphics::draw_string(60, TFT_H-28, "FIRE = SELECCIONAR", GREEN, BLACK, 1);
            Graphics::flush_buffer();
            int left = BTN_LEFT() ? 1 : 0;
            int right = BTN_RIGHT() ? 1 : 0;
            if (left && !lastLeft) selectedShip = (selectedShip + 4) % 5;
            if (right && !lastRight) selectedShip = (selectedShip + 1) % 5;
            if (BTN_FIRE()) { confirmed = true; delay_ms(200); }
            lastLeft = left; lastRight = right;
            delay_ms(50);
        }
    }

    void game_loop() {
        srand(time(NULL));
        if (!load_sprites()) {
            fprintf(stderr, "FATAL: No se pudieron cargar los sprites.\n");
            return;
        }
        show_title();
        show_ship_select();
        while(1) {
            score=0; lives=3; level=1; game_active=true;
            init_game();
            struct timespec last, now;
            clock_gettime(CLOCK_MONOTONIC, &last);
            while(game_active && lives>0) {
                update_player();
                update_bullets();
                update_invaders();
                update_explosions();
                check_collisions();
                draw_game();
                Graphics::flush_buffer();
                clock_gettime(CLOCK_MONOTONIC, &now);
                long elapsed = (now.tv_sec-last.tv_sec)*1000000L + (now.tv_nsec-last.tv_nsec)/1000L;
                if(elapsed < 16666L) delay_us(16666L - elapsed);
                clock_gettime(CLOCK_MONOTONIC, &last);
            }
            show_game_over();
        }
    }
}
