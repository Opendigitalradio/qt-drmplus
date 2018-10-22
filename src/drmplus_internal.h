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


#ifndef _DRMPLUS_INTERNAL_H_
#define _DRMPLUS_INTERNAL_H_


#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include <stdlib.h>

#include "drmplus.h"

#include "fac.h"
#include "audiotext.h"


#ifndef q31_t
#define q31_t int32_t
#define q15_t int16_t
#define q8_t int8_t
#endif

typedef int8_t iq_val[2];

typedef struct mult_desc_s {
	mpx_info_t info;

	uint8_t sdc_num;
	/* MSC processor data */
	uint16_t *msc_perm_table_A;
	uint16_t msc_len_A;
	uint16_t *msc_perm_table_B;
	uint16_t msc_len_B;

	uint8_t *str_buff[4];
	uint16_t str_buff_len[4];
	uint8_t str_flags[4];

	//streams callback handlers
	callback_t cb_str[4];
	void *cb_str_priv[4];

	//audio callback handlers
	callback_t cb_audio[4];
	void *cb_audio_priv[4];
	int16_t audio_last_frame[4][960*2*2]; //aac_samples x num_channels x sbr_multiplicator
	void *audio_decoder[4];
	int32_t channels_flags[4];
	int32_t aac_sample_rate[4];

	//text callback handlers
	callback_t cb_txt[4];
	void *cb_txt_priv[4];

	//text parsers
	audiotext_session_t audiotext[4];
} mpx_desc_t;

typedef struct {
	//various methods of equalizer here...
	drmplusConfiguration cfg;


	//channels callback handlers
	callback_t cb_fac;
	void *cb_fac_priv;
	callback_t cb_sdc;
	void *cb_sdc_priv;
	callback_t cb_msc;
	void *cb_msc_priv;

	callback_t cb_strlist;
	void *cb_strlist_priv;

	//info callback handlers
	callback_t cb_siginfo;
	void *cb_siginfo_priv;
	callback_t cb_sigeq;
	void *cb_sigeq_priv;
	callback_t cb_sigsp;
	void *cb_sigsp_priv;
	callback_t cb_sigiq;
	void *cb_sigiq_priv;


	//ofdm sync variables
	double bb_angle;
	fftw_complex frame_in[480*2];
	fftw_complex frame_shifted[480+16];
	fftw_complex symbol[432];
	//double spectrum[432];
	//int spectrum_cnt;

	siginfo_data_t siginfo;

	int cfe_freq_last;
	int cfe_freq_ok;
	double cfe_maxenergy_of4[2];
	double cfe_energyof_4[432];
	int cfe_maxenergy_cnt;
	int cfe_maxenergy_freq[2];
	int dc_freq_offset_checked;
	int cfe_freq_min;
	int symbol_id;

	double time_sync_history[40];
	double time_sync_history_inv[40];
	uint32_t symbol_num;
	int8_t frame_num;

	fftw_complex H_pilots[213];


	//FAC/SDC processor
	int8_t fac_deinterleaved[244*2];
	int fac_collected;
	dense_fac_t fac_data;


	int8_t sdc_deinterleaved[936*2]; //<< keep place for both rates 1/2 and 1/4
	int sdc_collected;
	uint8_t sdc_data[116];
	uint8_t sdc_size;

	//keeper of multiplex and service information.
	mpx_desc_t mpx_desc;
	srv_data_t services[4];

	iq_val z_history[6][7460]; //<< Z-history for MSC cells
	iq_val msc_bits[7460]; //<< Z-history for MSC cells
	int msc_collected;

	uint8_t msc_data[2330]; //<< max: qam16 1/2
} drmplus_priv_t;



/* ROM TABLES */
//const int8_t pilots[4][14];
const int16_t phases[40][14];

const int timesync_k[21];
const int timesync_phi[21];

const uint16_t fac_perm[244*2];
const uint16_t sdc_perm[936*2];
const uint16_t msc_perm[7460];

const uint8_t crc8tab[256];
const uint16_t crc16tab[256];



//debug logging functions
#if 0
#define ERRORLOG(fmt, ...)
#define WARNLOG(fmt, ...)
#define INFOLOG(fmt, ...)
#define DEBUGLOG(fmt, ...)
#define XTREMELOG(fmt, ...)
#define DUMPLOG(fmt, ...)
#else
#define ERRORLOG(fmt, ...)  fprintf(stderr, fmt"\n", ## __VA_ARGS__)
#define WARNLOG(fmt, ...)   fprintf(stderr, fmt"\n", ## __VA_ARGS__)
#define INFOLOG(fmt, ...)   fprintf(stderr, fmt"\n", ## __VA_ARGS__)
#define DEBUGLOG(fmt, ...)  fprintf(stderr, fmt"\n", ## __VA_ARGS__)
#define XTREMELOG(fmt, ...) fprintf(stderr, fmt"\n", ## __VA_ARGS__)
#endif
void print_bytes(char *bytes, int len);



//sig_sync
double drm_fine_freq_err(fftw_complex * drm_frame,int32_t fine_timeshift);
int drm_coarse_freq_sync_maxenergy(drmplus_priv_t *p, double *energy);
int drm_coarse_time_sync(drmplus_priv_t *p, fftw_complex *symbols, int *is_inverted);

//sig_proc
void drm_normalize_correct_phase(fftw_complex * symbols, uint8_t symbol_id);
void drm_equalize_symbol(fftw_complex * symbols, fftw_complex *H_pilots);

//equalizer memory feeders
void drm_equalize_feed_pilots_simple(fftw_complex * symbols, uint8_t symbol_id, fftw_complex *H_pilots, int is_initial_eq);
void drm_equalize_feed_pilots_lin_int(fftw_complex * symbols, uint8_t symbol_id, fftw_complex *H_pilots, int is_initial_eq, int use_mem);

void drm_decode_qam4(fftw_complex z, int8_t *r0, int8_t *r1);

//descramble PRBS stream
//void drm_descramble_int(int *in,int N);
int drm_prbs(int *in, size_t size);

//permutation tables
//int build_permutation(char t, size_t xin, unsigned int *perm);

//int drm_calc_crc2(char *out, char *in, char *poly, int dataLength, int crcLength);
//void drm_calc_crc(char *out, char *in, char *poly, int dataLength, int crcLength);
//void init_crc8tab(uint8_t l_code, uint8_t l_init);

//FAC processor
int drm_process_fac(int8_t *fac_deinterleaved, uint8_t *fac_data);
int drm_fill_services_from_fac(dense_fac_t *f, srv_data_t *s, mpx_info_t *info);
uint8_t crc8_fac(uint8_t l_crc, const void *lp_data, unsigned l_nb);

//SDC processor
int drm_process_sdc(int8_t *sdc_deinterleaved, uint8_t *sdc_data, uint8_t sdc_mode);
int drm_fill_services_from_sdc(uint8_t *s, uint8_t len, srv_data_t *srv, mpx_desc_t *desc);
uint16_t crc16_sdc(uint16_t l_crc, const void *lp_data, unsigned l_nb);

int drm_process_msc(iq_val *msc_bits, uint8_t *msc_data, mpx_desc_t *mpx, srv_data_t *str);

int drm_feed_aac_service(uint8_t *dataA, uint8_t *dataB, mpx_desc_t *mpx, srv_data_t *srv);


#endif //_DRMPLUS_INTERNAL_H_
