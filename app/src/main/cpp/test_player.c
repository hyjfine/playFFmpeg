//
//  test_player.c
//  FFmpegResearch
//
//  Created by sunlubo on 2020/3/5.
//  Copyright © 2020 sunlubo. All rights reserved.
//

#include <stdio.h>
#include "core_player.h"
#include "NALUParser.h"

int frame_number = 1;

static void save_video_frame(void *opaque, uint8_t *data[8], int size[8]) {
  char filename[1024];
  snprintf(filename, sizeof(filename), "/Users/sun/AV/img/%d.pgm", frame_number);
  FILE *file = fopen(filename, "w");
  
  fprintf(file, "P5\n%d %d\n%d\n", 1536, 1376, 255);
  for (int i = 0; i < 1376; i++) {
    fwrite(data[0] + i * 1536, 1, 1536, file);
  }
  
  fclose(file);
  
  frame_number += 1;
}

static void parser_callback(void *opaque, Nalu *nalu) {
  dump_nalu(nalu);
  Player *player = opaque;
  player_write_video_frame(player, nalu->data, nalu->size);
}

int main(void) {
  FILE *file = fopen("/Users/sun/AV/xm_sd.h264", "r");
  uint8_t *data = malloc(749683);
  fread(data, 749683, 1, file);
  fclose(file);
  
  PlayerContext *context = NULL;
  Player *player = player_alloc(&context);
  context->video_callback = save_video_frame;
  player_open(player);
  
  NaluParser *parser = parser_alloc();
  parser->opaque = player;
  parser->callback = parser_callback;
  parser_parse(parser, data, 749683);
  
  player_close(player);
  player_free(&player);
  
  return 0;
}
