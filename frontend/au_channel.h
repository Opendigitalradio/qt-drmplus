#include <stdio.h>
#include <string.h>

#define WAV_HEADER_SIZE 64

#define WAV_FORMAT_FLOAT 0xFFFE

typedef struct {
	int	sampleRate;
	int	nChannels;
	long	nSamples;
	int	aFmt;
} WavInfo;

inline FILE* AuChannelOpen (const char* filename, WavInfo* info)
{
	unsigned char header[12];
	unsigned char data[WAV_HEADER_SIZE];
	FILE *handle;
	unsigned int   chunksize;
	
	
	if (!strcmp(filename,"-"))
		handle = stdin;
	else
		handle = fopen(filename, "rb");

	if(!handle) return NULL;

	if(fread(header, 1, 12, handle) != 12) return NULL;
	info->nSamples		= (header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24)) + 8;

	while (memcmp(header, "data", 4) != 0){
	    if(fread(header, 1, 8, handle) != 8) return NULL;
	    chunksize             = (header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24));
	    //fprintf(stderr, "%c%c%c%c %d", header[0],  header[1], header[2], header[3], chunksize);
            if(!memcmp(header, "fmt ", 4)) {
                if(chunksize > WAV_HEADER_SIZE) return NULL;
	        if(fread(data, 1, chunksize, handle) != chunksize) return NULL;
                info->aFmt		= data[0] | data[1] << 8;
                info->nChannels		= data[2] | data[3] << 8;
                info->sampleRate	= data[4] | data[5] << 8 | data[6] << 12 | data[7] << 16;
	    } else if(memcmp(header, "data", 4) != 0) {
                if(fseek(handle, chunksize, SEEK_CUR) != 0) return NULL;
	    }
	}

	return handle;
}

inline void AuChannelClose (FILE *audioChannel)
{
	fclose(audioChannel);
}

inline size_t AuChannelReadShort(FILE *audioChannel, short *samples, int nSamples, int *readed)
{
	*readed = fread(samples, 2, nSamples, audioChannel);
	return *readed <= 0;
}

inline size_t AuChannelReadFloat(FILE *audioChannel, float *samples, int nSamples, int *readed)
{
	*readed = fread(samples, 4, nSamples, audioChannel);
	return *readed <= 0;
}
