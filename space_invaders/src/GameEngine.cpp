#include "GameEngine.h"
#include "Graphics.h"
#include "Sound.h"
#include <cstdlib>
#include <ctime>

namespace GameEngine {
    int16_t player_x = TFT_W/2 - PLAYER_WIDTH/2;
    bool game_active = true;
    uint16_t score = 0, high_score = 0;
    uint8_t lives = 3, level = 1, invaders_remaining = 0;
    
    Invader invaders[NUM_INVADERS_X][NUM_INVADERS_Y];
    Bullet bullets[10];
    Explosion explosions[20];
    Shield shields[4];
    
    static int8_t invader_dir = 1;
    static uint16_t step_timer = 0, shoot_timer = 0;
    static const int START_X = 20, START_Y = 30, SPACING_X = 24, SPACING_Y = 18;
    
    void init_game() {
        player_x = TFT_W/2 - PLAYER_WIDTH/2;
        invaders_remaining = 0;
        for(int i=0;i<NUM_INVADERS_X;i++) {
            for(int j=0;j<NUM_INVADERS_Y;j++) {
                invaders[i][j].x = START_X + i*SPACING_X;
                invaders[i][j].y = START_Y + j*SPACING_Y;
                invaders[i][j].active = true;
                invaders[i][j].type = (j<2)?1:(j<4)?2:3;
                invaders_remaining++;
            }
        }
        for(int i=0;i<10;i++) bullets[i].active = false;
        for(int i=0;i<20;i++) explosions[i].active = false;
        int pos[4] = {30,90,150,210};
        for(int i=0;i<4;i++) shields[i].x = pos[i], shields[i].y = TFT_H-60;
        invader_dir = 1; step_timer = 0; shoot_timer = 0;
    }
    
    void update_player() {
        if(BTN_LEFT() && player_x > 5) player_x -= PLAYER_SPEED;
        if(BTN_RIGHT() && player_x < TFT_W-PLAYER_WIDTH-5) player_x += PLAYER_SPEED;
        static bool last_fire=false;
        bool fire = BTN_FIRE();
        if(fire && !last_fire) {
            for(int i=0;i<10;i++) {
                if(!bullets[i].active) {
                    bullets[i].x = player_x + PLAYER_WIDTH/2 - BULLET_WIDTH/2;
                    bullets[i].y = PLAYER_Y - BULLET_HEIGHT;
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
                            switch(invaders[i][j].type) {
                                case 1: score+=30; break;
                                case 2: score+=20; break;
                                case 3: score+=10; break;
                            }
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
                if(bullets[b].x+BULLET_WIDTH > player_x && bullets[b].x < player_x+PLAYER_WIDTH &&
                   bullets[b].y+BULLET_HEIGHT > PLAYER_Y && bullets[b].y < PLAYER_Y+PLAYER_HEIGHT) {
                    bullets[b].active = false;
                    lives--;
                    sound_player_explosion();
                    if(lives==0) {
                        game_active = false;
                        if(score>high_score) high_score = score;
                        sound_game_over();
                    } else {
                        player_x = TFT_W/2 - PLAYER_WIDTH/2;
                        for(int i=0;i<10;i++) bullets[i].active = false;
                        delay_ms(1000);
                    }
                }
            }
            for(int s=0;s<4;s++) {
                if(bullets[b].active &&
                   bullets[b].x+BULLET_WIDTH > shields[s].x && bullets[b].x < shields[s].x+16 &&
                   bullets[b].y+BULLET_HEIGHT > shields[s].y && bullets[b].y < shields[s].y+12) {
                    bullets[b].active = false;
                }
            }
        }
    }
    
    void draw_game() {
        Graphics::draw_background();
        for(int s=0;s<4;s++) Graphics::draw_shield(shields[s].x, shields[s].y);
        for(int i=0;i<NUM_INVADERS_X;i++)
            for(int j=0;j<NUM_INVADERS_Y;j++)
                if(invaders[i][j].active) {
                    if(invaders[i][j].type==1) Graphics::draw_invader_type1(invaders[i][j].x, invaders[i][j].y);
                    else if(invaders[i][j].type==2) Graphics::draw_invader_type2(invaders[i][j].x, invaders[i][j].y);
                    else Graphics::draw_invader_type3(invaders[i][j].x, invaders[i][j].y);
                }
        for(int i=0;i<10;i++) if(bullets[i].active) Graphics::draw_bullet(bullets[i].x, bullets[i].y);
        for(int i=0;i<20;i++) if(explosions[i].active) Graphics::draw_explosion(explosions[i].x, explosions[i].y, explosions[i].frame);
        Graphics::draw_player(player_x, PLAYER_Y);
        Graphics::draw_string(5,5,"SCORE:",WHITE,BLACK,1);
        Graphics::draw_number(50,5,score,YELLOW,BLACK,1);
        Graphics::draw_string(130,5,"HIGH:",WHITE,BLACK,1);
        Graphics::draw_number(170,5,high_score,CYAN,BLACK,1);
        Graphics::draw_string(5,TFT_H-12,"LIVES:",WHITE,BLACK,1);
        for(int i=0;i<lives && i<5;i++) Graphics::draw_player(45+i*18, TFT_H-15);
        Graphics::draw_string(200,TFT_H-12,"LVL:",WHITE,BLACK,1);
        Graphics::draw_number(230,TFT_H-12,level,GREEN,BLACK,1);
    }
    
    void show_title() {
        Graphics::fill_screen(BLACK);
        Graphics::draw_string(40,50,"SPACE",GREEN,BLACK,3);
        Graphics::draw_string(25,85,"INVADERS",RED,BLACK,3);
        Graphics::draw_invader_type1(20,130);
        Graphics::draw_invader_type2(80,130);
        Graphics::draw_invader_type3(140,130);
        Graphics::draw_invader_type1(200,130);
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
    
    void game_loop() {
        srand(time(NULL));
        show_title();
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
