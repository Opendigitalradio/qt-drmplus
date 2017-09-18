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

/* up to 8 segments
 * segment: header, body, crc - body shall be 16 bytes, unlesss it
 * is the last segment
 * if no text: 00 00 00 00 */


#include <string.h>
#include <stdio.h>
#include "audiotext.h"
#include "drmplus_internal.h"

static const int VALID_SEGMENTS_MASK[] =
{
     0b0000000000000000,
     0b0000000000000001,
     0b0000000000000011,
     0b0000000000000111,
     0b0000000000001111,
     0b0000000000011111,
     0b0000000000111111,
     0b0000000001111111,
     0b0000000011111111,
     0b0000000111111111,
     0b0000001111111111,
     0b0000011111111111,
     0b0000111111111111,
     0b0001111111111111,
     0b0011111111111111,
     0b0111111111111111,
     0b1111111111111111
};


static void audiotext_process(audiotext_session_t *session)
{
     int i,j;
     char buf[AUDIOTEXT_MAX_STRLEN+1];
     int bufcnt = 0;
     char chr;
     int curr_toggle;
     audiotext_segment_t *seg;
     curr_toggle = session->segments[0].segment.head.togglebit;

//	 INFOLOG("toggle: last:%d, this:%d, segments:%d valid:%d|%d",
//			 session->last_toggle_bit, curr_toggle, session->num_segments, session->valid_segments, VALID_SEGMENTS_MASK[session->num_segments]);

	 if (session->num_segments > 0 &&
         session->num_segments <= AUDIOTEXT_MAX_SEGS &&
         session->valid_segments ==
         VALID_SEGMENTS_MASK[session->num_segments] &&
		 session->last_toggle_bit != curr_toggle
     	 )
     {
		  session->last_toggle_bit = curr_toggle;
          for (i = 0; i < session->num_segments; i++) {
               seg = &session->segments[i];
               for (j = 0; j < seg->segment.head.cmd_or_len + 1; j++) {
                    chr = seg->segment.body[j];
                    switch(chr) {
                         case ATEXT_PREFERED_LINEBREAK:
                              buf[bufcnt++] = '\n';
                              break;
                         case ATEXT_PREFERED_WORDBREAK:
                              buf[bufcnt++] = '\n';
                              break;
                         case ATEXT_END_OF_HEADLINE:
                              buf[bufcnt++] = '\n';
                              break;
                         default:
                              buf[bufcnt++] = chr;
                              break;
                    }
               }
          }
          buf[bufcnt++] = '\0';
          INFOLOG("text: %s", buf);
    	  if(session->cb_txt)
    		  session->cb_txt(session->cb_txt_priv, buf,  session->streamId+TEXT0_CALLBACK, NULL);
     }
}

void print_segment(audiotext_segment_t *seg)
{
     int i,j;
     char text[2*((AUDIOTEXT_BODY_LEN + 4) * AUDIOTEXT_CHUNKLEN+1)];
     int count = 0;
     unsigned char *s = seg->data.chunk[0];
     int len = (seg->segment.head.cmd_flag) ? 0 :
                  (seg->segment.head.cmd_or_len + 1);

     for (i = 2; i < len + 2; i++)
     {
          j = s[i] & 0xff;
          count += sprintf(text+count, "%c,", j);
     }
     DEBUGLOG("sgmt: %s", text);
}

const char *dummytxt = "";

void audiotext_decode(audiotext_session_t *session, char *data)
{
     int segidx;
     int len;
     uint16_t crcsum;
     audiotext_segment_t *seg = &session->temp;

     /* A new text message is initiated by the magick sequence
        presented by the 4-tupple {0xff 0xff 0xff 0xff}         */

     if (AUDIOTEXT_IS_MAGIC_WORD(data)) {
          /* current segment is ready -- process it */
          len = (seg->segment.head.cmd_flag) ? 0 :
                      (seg->segment.head.cmd_or_len + 1);
          segidx = seg->segment.head.segmentID;
          if (seg->segment.head.first_flag) {
        	   //session->valid_segments = 0;
               segidx = 0;
          }
          if (seg->segment.head.segmentID > AUDIOTEXT_MAX_SEGS)
               return;

          crcsum = ((uint16_t) seg->segment.body[len] << 8) +
                              (seg->segment.body[len+1] & 0xFF);
          //x16 + x12 + x5 + 1.
          if (crcsum == crc16_sdc(0x0000, seg->data.chunk, len + sizeof(audiotext_header_t))) {
//               print_segment(seg);
               if (seg->segment.head.cmd_flag) {
                    // command? - what command?
                    switch (seg->segment.head.cmd_or_len) {
                         case ATXT_COMMAND_CLEARSCREEN:
                        	  if(session->cb_txt)
                        		  session->cb_txt(session->cb_txt_priv, (void*)dummytxt,  session->streamId+TEXT0_CALLBACK, NULL);
                              break;
                         default:
                              break;
                    }
               } else {
                    if (session->curr_toggle_bit != seg->segment.head.togglebit) { // new message
                         session->curr_toggle_bit = seg->segment.head.togglebit;
                         session->valid_segments = 0;
                         session->num_segments = 0;
                    } else { // compare, if not equal - reset all (new message)
                         if (session->valid_segments & (1 << segidx)) {
                              if (memcmp(seg->data.chunk,
                                   (session->segments[segidx]).data.chunk,
                                    len + sizeof(audiotext_header_t)) != 0) {
                                   session->valid_segments = 0;
                                   session->num_segments = 0;
                              }
                         }
                    }
               }
               session->valid_segments |= (1 << segidx);
               if (session->num_segments < (segidx + 1 + (seg->segment.head.last_flag ? 0 : 1)))
                    session->num_segments = segidx + 1 + (seg->segment.head.last_flag ? 0 : 1);
               memcpy(session->segments[segidx].data.chunk, seg->data.chunk,
                      len + 2 + sizeof(audiotext_header_t));

               //DEBUGLOG("text: %s, tb:%d, f:%d, l:%d", seg->segment.head.cmd_flag ? "cmd" : "",
         	   //	  seg->segment.head.togglebit,  seg->segment.head.first_flag,  seg->segment.head.last_flag);

               audiotext_process(session);

          } else {
        	  DEBUGLOG("text segment CRC FAILED");
        	  //session->temp.data.num_chunks = 0; // reset temp segment buffer
        	  //session->curr_toggle_bit = 2; // non-existing
        	  //session->valid_segments = 0;
        	  //session->num_segments = 0;
          }
          seg->data.num_chunks = 0; // reset temp segment buffer
     } else {
          if (seg->data.num_chunks >= AUDIOTEXT_MAX_CHUNKS)
               return;

          memcpy(&seg->data.chunk[seg->data.num_chunks][0], data, AUDIOTEXT_CHUNKLEN);
          seg->data.num_chunks++;
     }
}
