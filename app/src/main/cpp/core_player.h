//
//  core_player.h
//  Media
//
//  Created by sunlubo on 2020/3/5.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#ifndef core_player_h
#define core_player_h

#include <stdlib.h>

typedef struct PlayerContext {
    void *opaque;
    void (*audio_callback)(void *opaque, uint8_t *const data[8], const int size[8]);
    void (*video_callback)(void *opaque, uint8_t *const data[8], const int size[8]);
} PlayerContext;

typedef struct Player Player;

Player *player_alloc(PlayerContext **context);
void player_free(Player **player);
int  player_open(Player *player);
void player_close(Player *player);
int  player_write_audio_frame(Player *player, const uint8_t *data, int size);
int  player_write_video_frame(Player *player, const uint8_t *data, int size);

#endif /* core_player_h */
