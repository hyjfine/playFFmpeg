//
//  NALUParser.h
//  Media
//
//  Created by sunlubo on 2020/3/3.
//  Copyright © 2020 wuuklabs. All rights reserved.
//

#ifndef NALUParser_h
#define NALUParser_h

#include <stdlib.h>

typedef enum NaluType {
    /// Unspecified
            NaluTypeUnspecified = 0,
    /// Coded slice of a non-IDR picture
            NaluTypeSlice = 1,
    /// Coded slice data partition A
            NaluTypeDPA = 2,
    /// Coded slice data partition B
            NaluTypeDPB = 3,
    /// Coded slice data partition C
            NaluTypeDPC = 4,
    /// Coded slice of an IDR picture
            NaluTypeIDR = 5,
    /// Supplemental enhancement information (SEI)
            NaluTypeSEI = 6,
    /// Sequence parameter set
            NaluTypeSPS = 7,
    /// Picture parameter set
            NaluTypePPS = 8,
    /// Access unit delimiter
            NaluTypeAUD = 9,
    /// End of sequence
            NaluTypeEndSequence = 10,
    /// End of stream
            NaluTypeEndStream = 11,
    /// Filler data
            NaluTypeFillerData = 12,
    /// Sequence parameter set extension
            NaluTypeSPSExt = 13,
    /// Prefix NAL unit
            NaluTypePrefix = 14,
    /// Subset sequence parameter set
            NaluTypeSubSPS = 15,
    /// Depth parameter set
            NaluTypeDPS = 16,
    /// Reserved
            NaluTypeReserved17 = 17,
    NaluTypeReserved18 = 18,
    /// Coded slice of an auxiliary coded picture without partitioning
            NaluTypeAuxiliarySlice = 19,
    /// Coded slice extension
            NaluTypeExtenSlice = 20,
    /// Coded slice extension for depth view components
            NaluTypeDepthExtenSlice = 21,
    /// Reserved
            NaluTypeReserved22 = 22,
    NaluTypeReserved23 = 23,
    /// Unspecified
            NaluTypeReserved24 = 24,
    NaluTypeReserved25 = 25,
    NaluTypeReserved26 = 26,
    NaluTypeReserved27 = 27,
    NaluTypeReserved28 = 28,
    NaluTypeReserved29 = 29,
    NaluTypeReserved30 = 30,
    NaluTypeReserved31 = 31,
} NaluType;

typedef struct Nalu {
    NaluType type;
    const uint8_t *data;
    int size;
} Nalu;

void dump_nalu(Nalu *nalu);

typedef struct NaluParser {
    void *opaque;

    void (*callback)(void *opaque, Nalu *nalu);
} NaluParser;

NaluParser *parser_alloc(void);

void parser_free(NaluParser *parser);

void parser_parse(NaluParser *parser, const uint8_t *data, int size);

#endif /* NALUParser_h */
