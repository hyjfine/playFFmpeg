//
//  Muxer.h
//  Media
//
//  Created by sunlubo on 2020/3/3.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#ifndef Muxer_h
#define Muxer_h

#include <stdlib.h>

typedef struct Muxer Muxer;

Muxer *muxer_alloc(const char *filename);
void muxer_free(Muxer *muxer);
int muxer_open(Muxer *muxer);
int muxer_close(Muxer *muxer);
int muxer_write_audio_frame(Muxer *muxer, uint8_t *data, int size);
int muxer_write_video_frame(Muxer *muxer, uint8_t *data, int size);

#endif /* Muxer_h */
