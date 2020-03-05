//
//  core_player.c
//  Media
//
//  Created by sunlubo on 2020/3/5.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#include "core_player.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/error.h>
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>

struct Player {
    PlayerContext *context;
    int video_frame_index;
    AVCodecContext *audio_decoder_context;
    AVCodecContext *video_decoder_context;
    int audio_frame_index;
};

static AVCodecContext *create_decoder(enum AVCodecID codec_id) {
  AVCodecContext *codec_context = NULL;
  AVCodec *codec = avcodec_find_decoder(codec_id);
  if (codec == NULL) {
    printf("ERROR: avcodec_find_decoder failed");
    return NULL;
  }

  codec_context = avcodec_alloc_context3(codec);
  if (codec_context == NULL) {
    printf("ERROR: avcodec_alloc_context3 failed");
    return NULL;
  }
  return codec_context;
}

static int open_audio_decoder(Player *player) {
  player->audio_decoder_context = create_decoder(AV_CODEC_ID_PCM_ALAW);
  if (player->audio_decoder_context == NULL) {
    return AVERROR(ENOMEM);
  }

  player->audio_decoder_context->channels = 1;
  player->audio_decoder_context->channel_layout = AV_CH_LAYOUT_MONO;
  player->audio_decoder_context->sample_rate = 8000;
  player->audio_decoder_context->sample_fmt = AV_SAMPLE_FMT_S16;
  player->audio_decoder_context->time_base = (AVRational){.num = 1, .den = 8000};

  int ret = avcodec_open2(player->audio_decoder_context, NULL, NULL);
  if (ret < 0) {
    printf("ERROR: avcodec_open2 failed: %s", av_err2str(ret));
  }
  return ret;
}

static int open_video_decoder(Player *player) {
  player->video_decoder_context = create_decoder(AV_CODEC_ID_H264);
  if (player->video_decoder_context == NULL) {
    return AVERROR(ENOMEM);
  }

  int ret = avcodec_open2(player->video_decoder_context, NULL, NULL);
  if (ret < 0) {
    printf("ERROR: avcodec_open2 failed: %s", av_err2str(ret));
  }
  return ret;
}

static int convert_video_frame(Player *player, AVFrame *frame) {
  struct SwsContext *scale_context = sws_getContext(1536, 1376, frame->format,
                                                    1536, 1376, AV_PIX_FMT_RGBA,
                                                    SWS_BILINEAR, NULL, NULL, NULL);
  if (scale_context == NULL) {
    printf("ERROR: sws_getContext failed");
    return AVERROR(ENOMEM);
  }

  uint8_t *dst_data[8] = {NULL};
  int dst_linesize[8] = {0};
  int ret = av_image_alloc(dst_data, dst_linesize, frame->width, frame->height, AV_PIX_FMT_RGBA, 1);
  if (ret < 0) {
    printf("ERROR: av_image_alloc failed: %s\n", av_err2str(ret));
    sws_freeContext(scale_context);
    return ret;
  }

  ret = sws_scale(scale_context,
                  (const uint8_t * const*)frame->data, frame->linesize,
                  0, frame->height,
                  dst_data, dst_linesize);
  if (ret < 0) {
    printf("ERROR: sws_scale failed: %s\n", av_err2str(ret));
  } else {
    player->context->video_callback(player->context->opaque, dst_data, dst_linesize);
  }

  av_freep(&dst_data[0]);
  sws_freeContext(scale_context);

  return ret;
}

static int decode(Player *player, AVCodecContext *codec_context, AVPacket *packet) {
  AVFrame *frame = av_frame_alloc();

  int ret = avcodec_send_packet(codec_context, packet);
  if (ret < 0) {
    printf("ERROR: avcodec_send_packet failed: %s\n", av_err2str(ret));
  }
  while (ret >= 0) {
    ret = avcodec_receive_frame(codec_context, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      ret = 0;
      break;
    } else if (ret < 0) {
      printf("ERROR: avcodec_receive_frame failed: %s\n", av_err2str(ret));
      break;
    }

    printf("index: %d pts: %lld dts: %lld duration: %lld\n", codec_context->frame_number, frame->pts, frame->pkt_dts, frame->pkt_duration);
    if (codec_context->codec_type == AVMEDIA_TYPE_AUDIO) {
      player->context->audio_callback(player->context->opaque, frame->data, frame->linesize);
    } else {
      convert_video_frame(player, frame);
    }
    av_frame_unref(frame);
  }

  av_frame_free(&frame);
  return ret;
}

Player *player_alloc(PlayerContext **context) {
  Player *player = malloc(sizeof(Player));
  player->context = malloc(sizeof(PlayerContext));
  *context = player->context;
  return player;
}

void player_free(Player **player) {
  if (player == NULL || *player == NULL) {
    return;
  }
  free((*player)->context);
  free(*player);
  *player = NULL;
}

int player_open(Player *player) {
  int ret = open_audio_decoder(player);
  if (ret < 0) {
    return ret;
  }

  ret = open_video_decoder(player);
  return ret;
}

void player_close(Player *player) {
  if (player == NULL) {
    return;
  }

  avcodec_free_context(&player->audio_decoder_context);
  avcodec_free_context(&player->video_decoder_context);
}

int player_write_audio_frame(Player *player, const uint8_t *data, int size) {
  AVPacket *packet = av_packet_alloc();
  av_new_packet(packet, size);
  memcpy(packet->data, data, size);
  packet->pts = player->audio_frame_index * 1024;
  packet->dts = player->audio_frame_index * 1024;
  packet->duration = 1024;
  packet->flags = AV_PKT_FLAG_KEY;
  player->audio_frame_index += 1;

  int ret = decode(player, player->audio_decoder_context, packet);
  av_packet_free(&packet);
  return ret;
}

int player_write_video_frame(Player *player, const uint8_t *data, int size) {
  AVPacket *packet = av_packet_alloc();
  av_new_packet(packet, size);
  memcpy(packet->data, data, size);
  packet->pts = player->video_frame_index * 1000;
  packet->dts = player->video_frame_index * 1000;
  packet->duration = 1000;
  player->video_frame_index += 1;

  int ret = decode(player, player->video_decoder_context, packet);
  av_packet_free(&packet);
  return ret;
}
