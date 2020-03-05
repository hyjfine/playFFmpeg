//
//  Muxer.c
//  Media
//
//  Created by sunlubo on 2020/3/3.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#include "Muxer.h"
#include "NALUParser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libavutil/rational.h>
#include <libavutil/error.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>

struct Muxer {
  char *filename;
  NaluParser *parser;
  AVFormatContext *format_context;
  AVStream *video_stream;
  uint8_t *sps_data;
  int sps_size;
  uint8_t *pps_data;
  int pps_size;
  int video_frame_index;
  int error;
};

static int add_video_stream(Muxer *muxer) {
  muxer->video_stream = avformat_new_stream(muxer->format_context, NULL);
  if (muxer->video_stream == NULL) {
    printf("ERROR: avformat_new_stream failed");
    return -1;
  }
  
  muxer->video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
  muxer->video_stream->codecpar->codec_id = AV_CODEC_ID_H264;
  muxer->video_stream->codecpar->width = 1536;
  muxer->video_stream->codecpar->height = 1376;
  muxer->video_stream->avg_frame_rate = (AVRational){.num=15, .den=1};
  muxer->video_stream->time_base = (AVRational){.num=1, .den=15000};
  
  return 0;
}

static void add_video_stream_extradata(Muxer *muxer) {
  int extradata_size = muxer->sps_size + muxer->pps_size;
  uint8_t *extradata = av_malloc(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
  memcpy(extradata, muxer->sps_data, muxer->sps_size);
  memcpy(extradata + muxer->sps_size, muxer->pps_data, muxer->pps_size);
  muxer->video_stream->codecpar->extradata = extradata;
  muxer->video_stream->codecpar->extradata_size = extradata_size;
  muxer->sps_size = 0;
  muxer->pps_size = 0;
  free(muxer->sps_data);
  free(muxer->pps_data);
}

static void parser_callback(void *opaque, Nalu *nalu) {
  Muxer *muxer = opaque;
  
  if (nalu->type == NaluTypeSPS && muxer->video_stream->codecpar->extradata_size == 0) {
    muxer->sps_size = nalu->size;
    muxer->sps_data = malloc(nalu->size);
    memcpy(muxer->sps_data, nalu->data, nalu->size);
  }
  if (nalu->type == NaluTypePPS && muxer->video_stream->codecpar->extradata_size == 0) {
    muxer->pps_size = nalu->size;
    muxer->pps_data = malloc(nalu->size);
    memcpy(muxer->pps_data, nalu->data, nalu->size);
    
    add_video_stream_extradata(muxer);
  }
  
  if (nalu->type == NaluTypeIDR
      || nalu->type == NaluTypeSlice
      || nalu->type == NaluTypeDPA
      || nalu->type == NaluTypeDPB
      || nalu->type == NaluTypeDPC
      ) {
    AVPacket *packet = av_packet_alloc();
    av_new_packet(packet, (int)nalu->size);
    memcpy(packet->data, nalu->data, nalu->size);
    packet->stream_index = muxer->video_stream->index;
    packet->pts = muxer->video_frame_index * 1000;
    packet->dts = muxer->video_frame_index * 1000;
    packet->duration = 1000;
    packet->flags = nalu->type == NaluTypeIDR ? AV_PKT_FLAG_KEY : 0;
    
    int ret = av_interleaved_write_frame(muxer->format_context, packet);
    if (ret < 0) {
      muxer->error = ret;
      printf("ERROR: av_write_frame failed: %s", av_err2str(ret));
    }
    
    av_packet_free(&packet);
    muxer->video_frame_index += 1;
  }
  
  dump_nalu(nalu);
}

Muxer *muxer_alloc(const char *filename) {
  assert(filename != NULL);
  Muxer *muxer = malloc(sizeof(Muxer));
  muxer->filename = strdup(filename);
  muxer->parser = parser_alloc();
  muxer->parser->opaque = muxer;
  muxer->parser->callback = parser_callback;
  return muxer;
}

void muxer_free(Muxer *muxer) {
  assert(muxer != NULL);
  parser_free(muxer->parser);
  free(muxer->filename);
  free(muxer);
}

int muxer_open(Muxer *muxer) {
  assert(muxer != NULL);
  AVOutputFormat *format = av_guess_format("mp4", NULL, NULL);
  int ret = avformat_alloc_output_context2(&muxer->format_context, format, NULL, NULL);
  if (ret < 0) {
    printf("ERROR: avformat_alloc_output_context2 failed: %s", av_err2str(ret));
    return ret;
  }
  
  ret = add_video_stream(muxer);
  if (ret < 0) {
    return ret;
  }
  
  ret = avio_open2(&muxer->format_context->pb, muxer->filename, AVIO_FLAG_WRITE, NULL, NULL);
  if (ret < 0) {
    printf("ERROR: avio_open2 failed: %s", av_err2str(ret));
    return ret;
  }
  
  ret = avformat_write_header(muxer->format_context, NULL);
  if (ret < 0) {
    printf("ERROR: avformat_write_header failed: %s", av_err2str(ret));
  }
  return ret;
}

int muxer_close(Muxer *muxer) {
  assert(muxer != NULL);
  int ret = av_write_frame(muxer->format_context, NULL);
  if (ret < 0) {
    printf("ERROR: av_write_frame failed: %s", av_err2str(ret));
    return ret;
  }
  
  ret = av_write_trailer(muxer->format_context);
  if (ret < 0) {
    printf("ERROR: av_write_trailer failed: %s", av_err2str(ret));
  }
  return ret;
}

int muxer_write_audio_frame(Muxer *muxer, uint8_t *data, int size) {
  assert(muxer != NULL);
  return muxer->error;
}

int muxer_write_video_frame(Muxer *muxer, uint8_t *data, int size) {
  assert(muxer != NULL);
  parser_parse(muxer->parser, data, size);
  return muxer->error;
}
