#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_beep(uint16_t freq, uint16_t ms);
void sound_shoot(void);
void sound_invader_explosion(void);
void sound_player_explosion(void);
void sound_start(void);
void sound_game_over(void);
void sound_level_up(void);

#endif
