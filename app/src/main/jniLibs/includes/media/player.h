//
//  player.h
//  WKMedia
//
//  Created by sunlubo on 2020/3/5.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#ifndef player_h
#define player_h

#include <stdint.h>

typedef enum PixelFormat {
  PixelFormatYUV420P,
  PixelFormatRGBA,
} PixelFormat;

typedef struct PlayerConfig {
  PixelFormat pixel_format;
  void *opaque;
  void (*audio_callback)(void *opaque, uint8_t *const data[8], const int size[8], int64_t pts);
  void (*video_callback)(void *opaque, uint8_t *const data[8], const int size[8], int64_t pts, int width, int height);
} PlayerConfig;

typedef struct Player Player;

Player *player_alloc(PlayerConfig **config);
void player_free(Player **player);
int player_open(Player *player);
void player_close(Player *player);
int player_write_audio_frame(Player *player, const uint8_t *data, int size);
int player_write_video_frame(Player *player, const uint8_t *data, int size);

#endif /* player_h */
