/* 
 * This file is part of the libdrmplus distribution.
 *   (https://github.com/Opendigitalradio/qt-drmplus)
 * Copyright (c) 2017 OpenDigitalRadio.
 * 
 * libdrmplus is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 2.1.
 *
 * libdrmplus is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <string.h>

#include "drmplus_internal.h"
#include "neaacdec.h"

int drm_feed_aac_service(uint8_t *dataA, uint8_t *dataB, mpx_desc_t *mpx, srv_data_t *srv)
{
	int f;
	uint8_t *msc_data;
	uint8_t *dataA_p1;
	uint8_t *dataA_p2;
	uint8_t *dataB_p1;
	uint8_t *dataB_p2;

	uint16_t len_A = mpx->info.length_A[srv->streamId];
	uint16_t len_B = mpx->info.length_B[srv->streamId];

	if(srv->textFlag && len_B >=4) {
		len_B -= 4;
		if(mpx->str_flags[srv->streamId] > 0) {
			audiotext_decode(&mpx->audiotext[srv->streamId], &dataB[len_B]);
		}
		//INFOLOG("text data: %02x%02x%02x%02x", dataB[len_B], dataB[len_B+1], dataB[len_B+2], dataB[len_B+3]);
		//	//memcpy(srv->textData[text_pos], &dataB[len_B], 4);
	}

	int msc2x_len = (len_A+len_B)*2;

	if(!msc2x_len)
		return 0;

	if(mpx->str_buff_len[srv->streamId] != msc2x_len) {
		if(!mpx->str_buff[srv->streamId])
			mpx->str_buff[srv->streamId] = malloc(msc2x_len);
		else
			mpx->str_buff[srv->streamId] = realloc(mpx->str_buff[srv->streamId], msc2x_len);
		mpx->str_buff_len[srv->streamId] = msc2x_len;
	}

	msc_data = mpx->str_buff[srv->streamId];
	dataA_p1 = &msc_data[0];
	dataA_p2 = &msc_data[len_A];
	dataB_p1 = &msc_data[len_A*2];
	dataB_p2 = &msc_data[len_A*2 + len_B];


	//shift right saved A/B parts of buffers
	if(len_A) {
		memcpy(dataA_p1, dataA_p2, len_A);
		memcpy(dataA_p2, dataA, len_A);
	}

	memcpy(dataB_p1, dataB_p2, len_B);
	memcpy(dataB_p2, dataB, len_B);

	//if this is 1st part of superframe, ignore it...
	if(mpx->str_flags[srv->streamId]==2) {
		mpx->str_flags[srv->streamId]=1;
		return 0;
	}

	int num_aac_frames = (srv->audioSamplingRate == 3) ? 5 : 10;
	int audio_payload_length = msc2x_len - num_aac_frames - ((num_aac_frames==5) ? 6 : 14);

	int higher_protect = 0;
	int msc_ptr = (num_aac_frames==5) ? 6 : 14;


	if(len_A)
		higher_protect = (len_A*2 - msc_ptr - num_aac_frames) / num_aac_frames;

	//fprintf(stderr, "msc2x_len|audio_payload_length|higher_protect => %d|%d|%d\n", msc2x_len,audio_payload_length,higher_protect);

	int frame_b[9] = {0,0,0,0,0,0,0,0,0};
	int frame_l[10] = {0,0,0,0,0,0,0,0,0,0};
	//fprintf(stderr, "aac frames: %d\n", num_aac_frames);
	//aac super frame header:
	frame_b[0] = (((uint16_t)msc_data[0]) << 4) | (msc_data[1] >> 4);
	frame_b[1] = (((uint16_t)msc_data[1] & 0x0F) << 8) | msc_data[2];
	frame_b[2] = (((uint16_t)msc_data[3]) << 4) | (msc_data[4] >> 4);
	frame_b[3] = (((uint16_t)msc_data[4] & 0x0F) << 8) | msc_data[5];

	if(num_aac_frames!=5) {
		frame_b[4] = (((uint16_t)msc_data[6]) << 4) | (msc_data[7] >> 4);
		frame_b[5] = (((uint16_t)msc_data[7] & 0x0F) << 8) | msc_data[8];
		frame_b[6] = (((uint16_t)msc_data[9]) << 4) | (msc_data[10] >> 4);
		frame_b[7] = (((uint16_t)msc_data[10] & 0x0F) << 8) | msc_data[11];
		frame_b[8] = (((uint16_t)msc_data[12]) << 4) | (msc_data[13] >> 4);
	}

	int is_ok = 1;
	int previous_border = 0;
	int i;
	for(i=0;i<num_aac_frames-1;i++) {

		if(frame_b[i] < 0 || frame_b[i] > audio_payload_length)
			is_ok = 0;

		//this will never happen, due total max len of audio super frame in qam4 == 932*2 bytes
		//if(frame_b[i+1] < frame_b[i]) frame_b[i+1] += 4096;

		frame_l[i] = frame_b[i] - previous_border;
		previous_border = frame_b[i];

		if(frame_l[i] <= 0 || frame_l[i] > audio_payload_length)
			is_ok = 0;

//		fprintf(stderr, "frame_len[%d]: [...%5d...]|b:%u|\n", i, frame_l[i], frame_b[i]);
	}
	frame_l[num_aac_frames-1] = audio_payload_length - previous_border;
//	fprintf(stderr, "frame_len[%d]: [...%5d...]|b:%u.\n", num_aac_frames-1, frame_l[num_aac_frames-1], audio_payload_length);

	if(!is_ok) {
		mpx->str_flags[srv->streamId]=0;
		fprintf(stderr, "Frame Boundary is not OK!\n");
		for (f = 0; f < num_aac_frames; f++) {
	        if(mpx->cb_str[srv->streamId])
	        	mpx->cb_str[srv->streamId](mpx->cb_str_priv[srv->streamId],  NULL, STREAM0_CALLBACK + srv->streamId, (void *) srv);
	        if(mpx->cb_audio[srv->streamId]) {
	        	mpx->cb_audio[srv->streamId](mpx->cb_audio[srv->streamId],  mpx->audio_last_frame[srv->streamId], AUDIO0_CALLBACK + srv->streamId, (void *) srv);
	        	//TODO: attenuate echo here...
	        }
		}

		return 0;
	}
	mpx->str_flags[srv->streamId]=2;


	//display frame data
#if 0
	int b;
	uint8_t audio_frame[10][1024];

	// higher_protected_part
	for (f = 0; f < num_aac_frames; f++) {
		fprintf(stderr, "mscA[%d]:",f);
		for (b = 0; b < higher_protect; b++) {
			audio_frame[f][b+1] = msc_data[msc_ptr + b];
			fprintf(stderr, "%02x", msc_data[msc_ptr + b]);
		}
		msc_ptr += higher_protect;
		audio_frame[f][0] = msc_data[msc_ptr++];
		fprintf(stderr, "|%02x\n", audio_frame[f][0]);
	}

	//fprintf(stderr, "%p == %p\n", &msc_data[msc_ptr], msc_uep_lo_data);



	//lower_protected_part
	for (f = 0; f < num_aac_frames; f++) {
		fprintf(stderr, "mscB[%d]:",f);
		int lpp_len = frame_l[f] - higher_protect;
		for (b = 0; b < lpp_len; b++) {
			audio_frame[f][1 + higher_protect + b] = msc_data[msc_ptr + b];
			fprintf(stderr, "%02x", msc_data[msc_ptr + b]);
		}
		msc_ptr += lpp_len;
		fprintf(stderr, "\n");
	}
#endif

	msc_ptr = (num_aac_frames==5) ? 6 : 14;
	int lpp_pos = msc_ptr + num_aac_frames*(higher_protect+1);
	int hpp_pos = msc_ptr;
	// higher_protected_part
	for (f = 0; f < num_aac_frames; f++) {
		uint8_t a_frame[1024];
		//fprintf(stderr, "frame[%d]:\n",f);
		int lpp_len = frame_l[f] - higher_protect;

		//a_frame[0] = msc_data[msc_ptr + (f+1)*(higher_protect+1)];
		a_frame[0] = msc_data[hpp_pos + higher_protect]; //move CRC to beginning
		memcpy(&a_frame[1], &msc_data[hpp_pos], higher_protect);
		memcpy(&a_frame[higher_protect+1], &msc_data[lpp_pos], lpp_len);
		//print_bytes(0, a_frame, higher_protect + 1 + lpp_len);

		srv->aacFrameLen = higher_protect + 1 + lpp_len;
        if(mpx->cb_str[srv->streamId])
			mpx->cb_str[srv->streamId](mpx->cb_str_priv[srv->streamId],  (void *) a_frame, STREAM0_CALLBACK + srv->streamId, (void *) srv);

        if(mpx->cb_audio[srv->streamId]) {
            int newChanFlags=0;
            switch(srv->audioMode) {
            case 0:
                newChanFlags = srv->sbrFlag ? DRMCH_SBR_MONO : DRMCH_MONO;
                break;
            case 1:
                newChanFlags = DRMCH_SBR_PS_STEREO;
                break;
            case 2:
                newChanFlags = srv->sbrFlag ? DRMCH_SBR_STEREO : DRMCH_STEREO;
                break;
            }


            int8_t is_new=0;
            if(newChanFlags != mpx->channels_flags[srv->streamId] || srv->audioSamplingRate != mpx->aac_sample_rate[srv->streamId]) {
                if(mpx->audio_decoder[srv->streamId]) NeAACDecClose(mpx->audio_decoder[srv->streamId]);
                mpx->audio_decoder[srv->streamId]=NULL;
                fprintf(stderr, "New AAC channels flags: %d", newChanFlags);
                char initErr = NeAACDecInitDRM(&mpx->audio_decoder[srv->streamId], (srv->audioSamplingRate == 3) ? 24000 : 48000, newChanFlags);
                if (initErr != 0) {
                	mpx->audio_decoder[srv->streamId]=NULL;
                    fprintf(stderr, "NeAACDecInitDRM() returned error: %d\n", initErr);
                } else {
                   mpx->channels_flags[srv->streamId] = newChanFlags;
                   mpx->aac_sample_rate[srv->streamId] = srv->audioSamplingRate;
                   //aac_params_shown=false;
                   //audio_prebuff=0;
                }
                is_new=1;
            }

            //int16_t buff[3072*2];
            NeAACDecFrameInfo hInfo;
            hInfo.error=1;
            int16_t *audioSamplesTemp=NULL;

            audioSamplesTemp=(int16_t*) NeAACDecDecode(mpx->audio_decoder[srv->streamId], &hInfo, a_frame, srv->aacFrameLen);
            if(is_new || hInfo.error)
            	fprintf(stderr,"FAAD %s: %d samples:%lu, sbr:%d, ps:%d, ch:%d sr:%lu bytes_left:%lu\n",
            			hInfo.error ? "error" : "",
            	hInfo.error, hInfo.samples, hInfo.sbr, hInfo.ps, hInfo.channels, hInfo.samplerate, srv->aacFrameLen - hInfo.bytesconsumed);

            if(!hInfo.error) {
            	memcpy(mpx->audio_last_frame[srv->streamId], audioSamplesTemp, sizeof(int16_t)*hInfo.samples);
            }

            mpx->cb_audio[srv->streamId](mpx->cb_audio[srv->streamId],  mpx->audio_last_frame[srv->streamId], AUDIO0_CALLBACK + srv->streamId, (void *) srv);

        	int j;
            if(!hInfo.error) {
            	//add echo fadein/fadeout at the beginning and end of the frame
            	for(j=0;j<16;j++) {
            		mpx->audio_last_frame[srv->streamId][j*2] = mpx->audio_last_frame[srv->streamId][j*2]/16 * j;
            		mpx->audio_last_frame[srv->streamId][j*2+1] = mpx->audio_last_frame[srv->streamId][j*2+1]/16 * j;

            		mpx->audio_last_frame[srv->streamId][(hInfo.samples - 1) - (j*2)] = mpx->audio_last_frame[srv->streamId][(hInfo.samples - 1) - (j*2)]/16 * j;
            		mpx->audio_last_frame[srv->streamId][(hInfo.samples - 1) - (j*2+1)] = mpx->audio_last_frame[srv->streamId][(hInfo.samples - 1) - (j*2+1)]/16 * j;
            	}
            }

            //decrease volume of saved echo
            float aVol = 0.7;
            for(j=0;j<hInfo.samples;j++)
            	mpx->audio_last_frame[srv->streamId][j] = (((float)mpx->audio_last_frame[srv->streamId][j]) * aVol);


        }

		lpp_pos += lpp_len;
		hpp_pos += higher_protect+1;
	}

	return 1;
}
