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


#include "drmplusdemod.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "au_channel.h"
#include "graphs_callbacks.h"
#include "drmplus.h"

static int stream_id=0;


void asprintf_noret(char **strp, const char *fmt, ...)
{
     va_list argp;
     int ignore;

     va_start(argp, fmt);
     ignore = vasprintf(strp, fmt, argp);
     va_end(argp);
}


#define BYTES_TO_FEED (480*2*2)	//< 480 I/Q samples per execution.

int stream_shown[4] = {0,0,0,0};
//TODO: do fac cache by crc comparison
void fac_update(void *priv_data, void *fac_p, int callbackType, void *_not_used)
{
	dense_fac_t *f = (dense_fac_t *) fac_p;
    int i;
    int all_shown=1;

	if(!f || callbackType != FAC_CALLBACK) return;

    uint8_t a = f->services >> 2 & 0x03;
    uint8_t d = f->services      & 0x03;
    int services = a+d;

    for(i=0;i<services;i++) {
    	fprintf(stderr, "stream_shown[%d/%d]=%d\n", i, services, stream_shown[i]);
    	if(stream_shown[i]==0)
    		all_shown=0;
    }
#if 1
	if(!all_shown) {
		drm_show_fac(f);
		stream_shown[f->short_id0]=1;
		stream_shown[f->short_id1]=1;
	} else {
		//exit(1);
	}
#else
	drm_show_fac(f);
	stream_shown[f->short_id0]=1;
	stream_shown[f->short_id1]=1;
#endif
}

//TODO: do sdc cache by crc comparison
int sdc_shown = 0;
void sdc_update(void *priv_data, void *sdc_p, int callbackType, void *sdc_sz_p)
{
	uint8_t *s = (uint8_t *) sdc_p;
	mpx_info_t *sdc_info = (mpx_info_t *) sdc_sz_p;

	fprintf(stderr, "sdc UPDATE!!!!!!\n");

	if( callbackType != SDC_CALLBACK || !s || !sdc_info) return;

	uint8_t sdc_num_bytes = sdc_info->sdc_mode ? (55+3) : (113+3);

	if(sdc_shown <= s[0]) {
		drm_show_sdc(s, sdc_num_bytes);

		fprintf(stderr, "sdc's shown[%d/%u]\n", sdc_shown, s[0]);
		sdc_shown++;
	}
}

void msc_update(void *priv_data, void *msc_p, int callbackType, void *msc_sz_p)
{
	uint8_t *m = (uint8_t *) msc_p;
	const int *msc_sz = (const int *) msc_sz_p;

	if( callbackType != MSC_CALLBACK || !m || !msc_sz || *msc_sz <=0) return;

	fprintf(stderr, "Got MSC: %d bytes\n", *msc_sz);
#if 0
    int i;
    int count;
    int done = 0;

    while (*msc_sz > done) {
		if (*msc_sz-done > 32)
			count = 32;
		else
			count = *msc_sz-done;
		fprintf(stderr, "\t");
		for (i=0; i<count; i++)
	    	fprintf(stderr, "%02x ", (int)((unsigned char)m[done+i]));
		for (; i<32; i++)
	    	fprintf(stderr, "   ");
		fprintf(stderr, "\t\"");
        for (i=0; i<count; i++)
	    	fprintf(stderr, "%c", isprint(m[done+i]) ? m[done+i] : '.');
        for (; i<32; i++)
	    	fprintf(stderr, " ");
		fprintf(stderr, "\"\n");
    	done += count;
    }
#endif
}

#include "neaacdec.h"

NeAACDecHandle hAac[4] = { NULL, NULL, NULL, NULL };
FILE* pFile2[4] = { NULL, NULL, NULL, NULL };

void strX_update(void *priv_data, void *aac_p, int callbackType, void *srv_p)
{
	uint8_t *aacFrame = (uint8_t *) aac_p;
	srv_data_t *srv = (srv_data_t *) srv_p;

	if(callbackType < STREAM0_CALLBACK || callbackType > STREAM3_CALLBACK
			|| !aacFrame || !srv || srv->aacFrameLen <=0 || callbackType-STREAM0_CALLBACK != srv->streamId) return;

	if(stream_id != callbackType-STREAM0_CALLBACK )  return;

#if 0
	fprintf(stderr, "Got STREAM[%d] %d bytes\n", callbackType-STREAM0_CALLBACK, srv->aacFrameLen);
	fprintf(stderr, "%02x|", aacFrame[0]);
    int i;
    for(i=1;i<srv->aacFrameLen;i++)
    	fprintf(stderr, "%02x", aacFrame[i]);
    fprintf(stderr, "\n");
#endif

	if(!hAac[srv->streamId]) {
		hAac[srv->streamId]=NeAACDecOpen();
		NeAACDecConfigurationPtr conf;
		conf = NeAACDecGetCurrentConfiguration(hAac[srv->streamId]);
		conf->defObjectType=DRM_ER_LC;
		conf->defSampleRate=24000;
		NeAACDecSetConfiguration(hAac[srv->streamId], conf);

		//audioSampleRate111=48000;
		//audioChannelsMode111 m, ps, s
		int chanFlags = 0;
		if(srv->sbrFlag) {
			switch(srv->audioMode) {
			case 0:
				chanFlags = DRMCH_SBR_MONO;
				break;
			case 1:
				chanFlags = DRMCH_SBR_PS_STEREO;
				break;
			case 2:
				chanFlags = DRMCH_SBR_STEREO;
				break;
			}
		} else {
			switch(srv->audioMode) {
			case 0:
				chanFlags = DRMCH_MONO;
				break;
			case 1:
				chanFlags = DRMCH_SBR_PS_STEREO;
				break;
			case 2:
				chanFlags = DRMCH_STEREO;
				break;
			}
		}

		//audioSampleRate111=48000;
		//chanFlags = DRMCH_MONO;
		//chanFlags = DRMCH_MONO;
		char initErr = NeAACDecInitDRM(&hAac[srv->streamId], (srv->audioSamplingRate == 3) ? 24000 : 48000, chanFlags);
		if (initErr != 0) { fprintf(stderr, "NeAACDecInitDRM() returned error: %d\n", initErr); exit(1);}
	}

	NeAACDecFrameInfo hInfo;
	short *audioSamplesTemp=(short*) NeAACDecDecode(hAac[srv->streamId], &hInfo, aacFrame, srv->aacFrameLen);

	if(hInfo.error)
		fprintf(stderr,"FAAD error: %d samples:%lu, sbr:%d, ps:%d, ch:%d sr:%lu bytes_left:%lu\n",
			hInfo.error, hInfo.samples, hInfo.sbr, hInfo.ps, hInfo.channels, hInfo.samplerate, srv->aacFrameLen - hInfo.bytesconsumed);
	else
		fprintf(stderr, ".");

	if(audioSamplesTemp && hInfo.samples > 0) {
		if(!pFile2[srv->streamId]) {
			char outFile[64];
			snprintf(outFile, 64, "out_aac%d.pcm", srv->streamId);
			pFile2[srv->streamId] = fopen(outFile, "wb");
		}
		fwrite((void*) audioSamplesTemp, sizeof(short), hInfo.samples, pFile2[srv->streamId]); // frame length
		//fflush(pFile2[srv->streamId]);
	}
}

int main(int argc, char *argv[])
{

  WavInfo inputInfo;
  FILE *inputFile = NULL;
  FILE *outputFile = stdout;

  int opt;

  int  error;
  int frmCnt = 0;

  while ((opt = getopt(argc, argv, "S:")) != -1) {
      switch (opt) {
      case 'S': stream_id = atoi(optarg); break;
      default:
          fprintf(stderr, "Usage: %s [-S <stream_id>] [file...]\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }


  if(stream_id < 0 || stream_id > 3) {
	    fprintf(stderr, "\nBAD stream id: %d\n", stream_id);
	    exit(EXIT_FAILURE);
  }

   /* parse command line arguments */
  if (argc < 2) {
    fprintf(stderr, "\nUsage:   %s <in_wav_file> <out_wav_file>\n", argv[0]);
    fprintf(stderr, "\nExample: %s input.wav out.wav\n", argv[0]);
    exit(EXIT_FAILURE);
  }

#if 0
  inputFile = AuChannelOpen (argv[1], &inputInfo);

  if(inputFile == NULL){
    fprintf(stderr,"could not open %s\n",argv[1]);
    exit(10);
  }

  if (inputInfo.nChannels!=2) {
	  fprintf(stderr,"Need stereo (I/Q) input!\n");
	  exit(10);
  }
#endif

  if(optind>=argc) {
	  fprintf(stderr, "Input file is not setted!\n");
	  exit(EXIT_FAILURE);
  }

  fprintf(stderr, "opening file %s for reading\n", argv[optind]);
  inputFile = fopen(argv[optind], "rb");
  if(!inputFile) {
	  fprintf(stderr, "Input file opening failed!\n");
	  exit(EXIT_FAILURE);
  }

  optind++;


  if (optind < argc && strcmp(argv[optind],"-")!=0) {
	  fprintf(stderr, "Opening file %s for writing\n", argv[optind]);
	  outputFile = fopen(argv[optind], "wb");
  }

  if(!outputFile) {
	  fprintf(stderr, "Output file opening failed!\n");
	  exit(EXIT_FAILURE);
  }

  unsigned long inputSamples=0;

  unsigned long cap;
  cap = NeAACDecGetCapabilities();
  fprintf(stderr, "NeAACDecGetCapabilities() = %lu\n",cap);



  drmplusHandle h = drmplusOpen();

  drmplusConfiguration *cfg = drmplusGetConfiguration(h);
  cfg->callOnEveryFACandSDC = (argc > 3) ? atoi(argv[3]) : 0;
  cfg->equalizerType = EQ_TYPE_LIN_INTERP;

#if 0
  fprintf(stdout,"input file %s: \nsr = %d, nc = %d fmt = %d\n\n",
          argv[1], inputInfo.sampleRate, inputInfo.nChannels, inputInfo.aFmt);
  fprintf(stdout,"output file %s: \nbr = %d inputSamples = %lu  maxOutputBytes = %lu nc = %d m = %d\n\n",
            argv[2], cfg->bitRate, inputSamples, maxOutputBytes, cfg->nChannelsOut, bodeMono);
  fflush(stdout);
#endif

  int ret = 0;
  if((ret = drmplusSetConfiguration(h, cfg)) == DRMPLUS_ERR) {
      fprintf(stdout, "setting cfg failed %d!\n", ret);
      return -1;
  }


  short *IQ_data = calloc(480, sizeof(short)*2);

  drmplusSetCallback(h, FAC_CALLBACK, fac_update, NULL);
  drmplusSetCallback(h, SDC_CALLBACK, sdc_update, NULL);
  drmplusSetCallback(h, MSC_CALLBACK, msc_update, NULL);

  drmplusSetCallback(h, STREAM0_CALLBACK + stream_id, strX_update, NULL);

  do {
	  if(!fread(IQ_data, sizeof(short)*2*480, 1, inputFile)) {
		  printf("end of file\n");
		  break;
	  }

	  if(drmplusAddSamplesShort(h, IQ_data, 480)!=DRMPLUS_OK) {
		  printf("decoding failed\n");
		  break;
	  }

      frmCnt++;
      //fprintf(stderr,"[%d]\r",frmCnt); fflush(stderr);

  } while (1);

  fprintf(stderr,"\n");
  fflush(stderr);

  printf("\ndecoding finished\n");
  drmplusClose(h);
  fclose(inputFile);
  free(IQ_data);

  int i;
  for (i=0;i<4;i++) {
	if(pFile2[i]) {
		fflush(pFile2[i]);
		fclose(pFile2[i]);
		pFile2[i]=NULL;
	}
  }

  return 0;
}
