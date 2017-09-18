/* -*-mode:c; c-style:k&r; c-basic-offset:5; -*- */
/**
 * @file
 * @brief Text message application
 *
 * DRM stations can broadcast short text services in
 * addition to audio streams. This application uses up
 * 4 bytes of the audio frame, corresponding to 80bit/s
 *
 * @author Anders Mørk-Pedersen
 *
 * @Date: 2009-11-19 11:03:00 +0100 (Thu, 19 Nov 2009)
 * @Revision: 1
 *
 */

#ifndef __AUDIOTEXT_H
#define __AUDIOTEXT_H

#include <stdint.h>
#include "drmplus.h"

#define AUDIOTEXT_CHUNKLEN     4   /* byte */
#define AUDIOTEXT_MAX_SEGS     8   /* segments*/
#define AUDIOTEXT_BODY_LEN     16  /* bytes */
#define AUDIOTEXT_MAX_CHUNKS   ((AUDIOTEXT_BODY_LEN + 4) / AUDIOTEXT_CHUNKLEN)
#define AUDIOTEXT_MAX_STRLEN   (AUDIOTEXT_MAX_SEGS * AUDIOTEXT_BODY_LEN)

#define AUDIOTEXT_IS_MAGIC_WORD(a) (*((uint32_t *) a) == 0xFFFFFFFF)

#define ATXT_COMMAND_CLEARSCREEN 0b0001

typedef struct __attribute__ ((__packed__))
{
     uint8_t cmd_or_len : 4; // bytes in the body minus 1 (or command)
     uint8_t cmd_flag   : 1;
     uint8_t last_flag  : 1;
     uint8_t first_flag : 1;
     uint8_t togglebit  : 1;

     uint8_t rfa        : 4;
     uint8_t segmentID  : 3; // 111 when cmd_flag
     uint8_t _rfa       : 1; //   1 when cmd_flag
} audiotext_header_t;

typedef struct __attribute__ ((__packed__))
{
     // actual header
     audiotext_header_t head;
     uint8_t body[AUDIOTEXT_BODY_LEN];
     uint8_t crc[2];
     // own additions
     uint8_t num_chunks;
} segment_segment_t;
typedef struct __attribute__ ((__packed__))
{
     uint8_t chunk[AUDIOTEXT_MAX_CHUNKS][AUDIOTEXT_CHUNKLEN];
     uint8_t num_chunks;
} segment_data_t;

typedef union
{
     segment_segment_t segment;
     segment_data_t data;
} audiotext_segment_t;

typedef struct
{
     // incomming
     uint8_t curr_toggle_bit;
     uint8_t last_toggle_bit;
     uint8_t num_segments;
     unsigned int valid_segments;
     audiotext_segment_t segments[AUDIOTEXT_MAX_SEGS];
     audiotext_segment_t temp;

 	 callback_t cb_txt;
 	 void *cb_txt_priv;

 	 int streamId;

} audiotext_session_t;

void audiotext_init(audiotext_session_t *text);

void audiotext_decode(audiotext_session_t *text, char *data);

typedef enum
{
     ATEXT_PREFERED_LINEBREAK = 0x0A,
     ATEXT_PREFERED_WORDBREAK = 0x1F,
     ATEXT_END_OF_HEADLINE    = 0x0B
} ATEXT_SPECIAL_CHARS_T;

#endif
