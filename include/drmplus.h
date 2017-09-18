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


#ifndef _DRMPLUS_H_
#define _DRMPLUS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DRMPLUS_ERR 0
#define DRMPLUS_OK 1

typedef void *drmplusHandle;



enum {
	SYNC_STATE_NULL=0,
	SYNC_STATE_FTO_TRY=10,		//< trying to sync fine time offset
	SYNC_STATE_FTO_DONE=11,		//< fine time sync done
	SYNC_STATE_IFO_TRY=12,		//< trying to sync integer frequency offset
	SYNC_STATE_IFO_DONE=13,		//< integet frequency sync done
	SYNC_STATE_DONE=14
};


typedef struct service_data_s {
	/* info got from FAC */
	uint32_t serviceId;
	uint8_t shortId;
	uint8_t audioCaFlag;
	uint8_t language;
	char language2[3];
	char country[2];
	uint8_t audioDataFlag;
	uint8_t serviceDesc;
	uint8_t dataCaFlag;
	/* info got from SDC fig1 */
	char    label[64];

	/* info got from SDC fig9 */
	uint8_t streamId;
	uint8_t audioCoding;
	uint8_t sbrFlag;
	uint8_t audioMode;
	uint8_t audioSamplingRate;
	uint8_t textFlag;
	uint8_t enhancementFlag;
	uint8_t coderField;
	uint8_t extradata[24]; //<< space for xHE-AAC + mpeg-surround
	uint8_t extradataLen;
	//last AAC frame bytes
	uint16_t aacFrameLen;
	uint8_t flags;
} srv_data_t;


typedef struct mpx_info_s {
	/* info got from FAC */
	uint8_t sdc_mode;                 //<< modulation mode for SDC
	uint8_t msc_mode;                 //<< modulation mode for MSC
	uint8_t service_num_a;              //<< amount of provided audio services
	uint8_t service_num_d;              //<< amount of provided data services

	/* info got from SDC fig0 */
	uint8_t prot_A;
	uint8_t prot_B;
	uint16_t length_A[4]; //total lengths in bytes
	uint16_t length_B[4];
	/* info got from SDC fig8 */
	long Year;
	long Month;
	long Day;
	uint8_t Hours;
	uint8_t Minutes;

} mpx_info_t;




typedef struct siginfo_data_s {
	int sync_state;
	int16_t fine_timeshift;


	double dc_freq_fine; // -222.2222 ... 222.2222
	int dc_freq_coarse;

	uint8_t fac_crc_ok;
	uint8_t sdc_crc_ok;
	uint8_t msc_crc_ok;

	uint8_t fac_errors;

	int8_t spectrum_inverted;

	double snr_signal;			//signal/noise level for FAC/SDC/MSC.
	double snr_noise;			//no carrier levels.
} siginfo_data_t;


enum {
	FAC_CALLBACK=0,		//< channel data callbacks, good for creating RSCI stream.
	SDC_CALLBACK,
	MSC_CALLBACK,

	STREAM0_CALLBACK,	//< actual streams callbacks (not decoded)
	STREAM1_CALLBACK,
	STREAM2_CALLBACK,
	STREAM3_CALLBACK,

	AUDIO0_CALLBACK,	//< actual sound for stream N callbacks
	AUDIO1_CALLBACK,
	AUDIO2_CALLBACK,
	AUDIO3_CALLBACK,

	TEXT0_CALLBACK,		//< actual text for stream N callbacks
	TEXT1_CALLBACK,
	TEXT2_CALLBACK,
	TEXT3_CALLBACK,

	STRLIST_CALLBACK,	//< list of avaliable stations callbacks
	SIGINFO_CALLBACK,	//< signal status info callbacks

	/* this callbacks used for drawing signal graphs */
	SIGEQ_CALLBACK,		//< signal equalizer state callback
	SIGSP_CALLBACK,		//< signal spectrum callback
	SIGIQ_CALLBACK,		//< signal I/Q constellation callback

	ENDLIST_CALLBACK
};

enum {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_XTREME,
    LOG_DUMP
};

enum {
	CELL_TYPE_NONE=0,		//< MSC cell
	CELL_TYPE_PIL,			//< pilot cell
	CELL_TYPE_PIL2X,		//< scattered pilot cell
	CELL_TYPE_AFS,
	CELL_TYPE_FAC,
	CELL_TYPE_SDC,
	CELL_TYPE_MSC,

	//this type of cessl used only on initial synchronisation
	CELL_TYPE_SDC_OR_MSC,	//< this cells can be SDC or MSC
	CELL_TYPE_AFS_OR_MSC,	//< this cells can be AFS or MSC
	CELL_TYPE_ENDLIST
};

enum {
	EQ_TYPE_SIMPLE = 0,			//< simple memorized equalizer
	EQ_TYPE_LIN_INTERP,			//< linear interpolation after each pilots received
	EQ_TYPE_LIN_INTERP_MEM,			//< linear interpolation with memory effect
};

enum {
	IFO_EST_MAXENERGY = 0,		//< simple estimation, based on max energy per symbol
	IFO_EST_4SYMBOLS,			//< estimation, based on max summarized energy of 4 symbols
};

typedef struct {
	//various methods of equalizer here...
	int equalizerType;
	int ifoEstimationType;
	int callOnEveryFACandSDC;
	int callIfBadCrc;
	int audioEchoDisabled;
	int loglevel;
} drmplusConfiguration;

typedef void (* callback_t)(void *privData, void *channelData, int callbackType, void *additionalInfo);

drmplusConfiguration *drmplusGetConfiguration(drmplusHandle h);
int drmplusSetConfiguration(drmplusHandle h, drmplusConfiguration *cfg);

drmplusHandle drmplusOpen();

int drmplusSetCallback(drmplusHandle h, int callbackType, callback_t callbackFunction, void *privData);

int drmplusAddSamplesShort(drmplusHandle h, int16_t *inputBuffer, unsigned int samplesInput);
int drmplusAddSamplesDouble(drmplusHandle h, double *inputBuffer, unsigned int samplesInput);

int drmplusResetSync(drmplusHandle h, uint32_t flags);

int drmplusClose(drmplusHandle h);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRMPLUS_H_ */
