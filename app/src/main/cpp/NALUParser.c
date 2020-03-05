//
//  NALUParser.c
//  Media
//
//  Created by sunlubo on 2020/3/3.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "NALUParser.h"

void dump_nalu(Nalu *nalu) {
  char *type = NULL;
  switch (nalu->type) {
    case NaluTypeSlice:
      type = "Slice";
      break;
    case NaluTypeDPA:
      type = "DPA";
      break;
    case NaluTypeDPB:
      type = "DPB";
      break;
    case NaluTypeDPC:
      type = "DPC";
      break;
    case NaluTypeIDR:
      type = "IDR";
      break;
    case NaluTypeSEI:
      type = "SEI";
      break;
    case NaluTypeSPS:
      type = "SPS";
      break;
    case NaluTypePPS:
      type = "PPS";
      break;
    case NaluTypeAUD:
      type = "AUD";
      break;
    case NaluTypeEndSequence:
      type = "EndSequence";
      break;
    case NaluTypeEndStream:
      type = "EndStream";
      break;
    case NaluTypeFillerData:
      type = "FillerData";
      break;
    case NaluTypeSPSExt:
      type = "SPSExt";
      break;
    case NaluTypePrefix:
      type = "Prefix";
      break;
    case NaluTypeSubSPS:
      type = "SubSPS";
      break;
    case NaluTypeDPS:
      type = "DPS";
      break;
    case NaluTypeAuxiliarySlice:
      type = "AuxiliarySlice";
      break;
    case NaluTypeExtenSlice:
      type = "ExtenSlice";
      break;
    case NaluTypeDepthExtenSlice:
      type = "DepthExtenSlice";
      break;
    case NaluTypeReserved17:
    case NaluTypeReserved18:
    case NaluTypeReserved22:
    case NaluTypeReserved23:
      type = "Reserved";
      break;
    default:
      type = "Unspecified";
  }
  printf("type: %s, size: %d\n", type, nalu->size);
}

NaluParser *parser_alloc(void) {
  NaluParser *context = malloc(sizeof(NaluParser));
  return context;
}

void parser_free(NaluParser *parser) {
  free(parser);
}

static void parse(NaluParser *parser, const uint8_t *data, int start, int end) {
  start = start - 4;
  Nalu nalu = {
    .type=data[start + 4] & 0b00011111,
    .data=data + start,
    .size=end - start
  };
  parser->callback(parser->opaque, &nalu);
}

void parser_parse(NaluParser *parser, const uint8_t *data, int size) {
  assert(parser != NULL);
  assert(size > 4);
  assert(data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x01);
  
  uint32_t p = 0;
  uint32_t q = 0;
  while (q < size - 3) {
    if (data[q] == 0x00 && data[q + 1] == 0x00 && data[q + 2] == 0x00 && data[q + 3] == 0x01) {
      if (p != 0) {
        parse(parser, data, p, q);
      }
      p = q + 4;
    }
    q += 1;
  }
  if (p != 0) {
    parse(parser, data, p, size);
  }
}
