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


#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "drmplus_internal.h"

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#define ENABLE_FAAD

#ifdef ENABLE_FAAD
#include "neaacdec.h"
#endif

drmplusHandle drmplusOpen() {

#ifdef ENABLE_FAAD
    unsigned long codec_capabilities;
    codec_capabilities = NeAACDecGetCapabilities();
    if (codec_capabilities & FIXED_POINT_CAP)
    	INFOLOG("AAC Codec capability FIXED_POINT");
    if (codec_capabilities & LC_DEC_CAP)
    	INFOLOG("AAC Codec capability LC");
    if (codec_capabilities & MAIN_DEC_CAP)
    	INFOLOG("AAC Codec capability MAIN");
    if (codec_capabilities & LTP_DEC_CAP)
    	INFOLOG("AAC Codec capability LTP");
    if (codec_capabilities & LD_DEC_CAP)
    	INFOLOG("AAC Codec capability LD");
    if (codec_capabilities & ERROR_RESILIENCE_CAP)
    	INFOLOG("AAC Codec capability ERROR RESILIANCE");

    if (codec_capabilities & (ERROR_RESILIENCE_CAP | LC_DEC_CAP)) {
    	INFOLOG("This codec should do fine for DRM");
    	WARNLOG("Your MPEG4/AAC+ Codec WAY COOL");
    } else {
    	ERRORLOG("Your MPEG4/AAC+ Codec NOT COOL");
    	ERRORLOG("Error: Your version of libfaad, is");
    	ERRORLOG("missing ERROR_RESILIANCE and LC");
    	ERRORLOG("support, you need to build that in");
        return NULL;
    }
#endif

	drmplus_priv_t *p = calloc(1, sizeof(drmplus_priv_t));
	p->cfg.loglevel = LOG_DEBUG;
	//TODO: set defaults
	INFOLOG("Opened DRM+ receiver %p", p);

	p->mpx_desc.audiotext[0].curr_toggle_bit = 2;
	p->mpx_desc.audiotext[0].streamId = 0;
	p->mpx_desc.audiotext[1].curr_toggle_bit = 2;
	p->mpx_desc.audiotext[1].streamId = 1;
	p->mpx_desc.audiotext[2].curr_toggle_bit = 2;
	p->mpx_desc.audiotext[2].streamId = 2;
	p->mpx_desc.audiotext[3].curr_toggle_bit = 2;
	p->mpx_desc.audiotext[3].streamId = 3;
#if 0
	fftw_complex PILOTS[40][213];
	fftw_complex TIMESYNC_PILOTS[213];

	//drm_create_phaseref_tables(PILOTS, TIMESYNC_PILOTS);

	int8_t pilots[40][14];
	int16_t pilot_phases[40][14];

	drm_create_phaseref_simple_tables(pilots, pilot_phases);


	int s, i;

	fprintf(stderr, "pilots[40][14] = {\n");
	for(s=0;s<40;s++) {
		fprintf(stderr, "{ ");
		for(i=0;i<14;i++) {
			fprintf(stderr, "% 4d, ", pilots[s][i]);
		}
		fprintf(stderr, "},\n");
	}
	fprintf(stderr, "}\n\n");


	fprintf(stderr, "phases[40][14] = {\n");
	for(s=0;s<40;s++) {
		fprintf(stderr, "{ ");
		for(i=0;i<14;i++) {
			fprintf(stderr, "% 4d, ", pilot_phases[s][i]);
		}
		fprintf(stderr, "},\n");
	}
	fprintf(stderr, "}\n\n");
	exit(1);
#endif

#if 0
	unsigned int perm[936*2];
	build_permutation(21, 244*2, perm);


	fprintf(stderr,"const uint16_t fac_perm[244*2] = { \n");

	int i;
	for (i=0;i<244*2;i++)
		fprintf(stderr, "%s%d, %s",
				(i%16 == 0) ? "  " : "",
				perm[i],
				(i+1 == 244*2) ? "\n" : ",",
				(i%16 == 15) ? "\n" : "");

	fprintf(stderr,"};\n");


	build_permutation(21, 936*2, perm);
	fprintf(stderr,"const uint16_t sdc_perm[936*2] = { \n");

	int i;
	for (i=0;i<936*2;i++)
		fprintf(stderr, "%s%d%s %s",
				(i%16 == 0) ? "  " : "",
				perm[i],
				(i+1 == 936*2) ? "\n" : ",",
				(i%16 == 15) ? "\n" : "");

	fprintf(stderr,"};\n");
	exit(1);
#endif

#if 0
	unsigned int perm[7460];
	build_permutation(5, 7460, perm);
	fprintf(stderr,"const uint16_t msc_perm[7460] = { \n");

	int i;
	for (i=0;i<7460;i++)
		fprintf(stderr, "%s%d%s %s",
				(i%16 == 0) ? "  " : "",
				perm[i],
				(i+1 == 7460) ? "\n" : ",",
				(i%16 == 15) ? "\n" : "");

	fprintf(stderr,"};\n");
	exit(1);
#endif
	return (drmplusHandle) p;
}

drmplusConfiguration *drmplusGetConfiguration(drmplusHandle hp)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;
	drmplusConfiguration *cfg = calloc(1, sizeof(drmplusConfiguration));
	memcpy(cfg, &p->cfg, sizeof(drmplusConfiguration));
	return cfg;
}

int drmplusSetConfiguration(drmplusHandle hp, drmplusConfiguration *cfg)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;
	memcpy(&p->cfg, cfg, sizeof(drmplusConfiguration));
	return DRMPLUS_OK;
}


int drmplusSetCallback(drmplusHandle hp, int callbackType, callback_t callbackFunction, void *privData)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;

	switch(callbackType) {
	case FAC_CALLBACK:
		p->cb_fac = callbackFunction;
		p->cb_fac_priv = privData;
		break;
	case SDC_CALLBACK:
		p->cb_sdc = callbackFunction;
		p->cb_sdc_priv = privData;
		break;
	case MSC_CALLBACK:
		p->cb_msc = callbackFunction;
		p->cb_msc_priv = privData;
		break;

	case STRLIST_CALLBACK:
		p->cb_strlist = callbackFunction;
		p->cb_strlist_priv = privData;
		break;
	case STREAM0_CALLBACK:
	case STREAM1_CALLBACK:
	case STREAM2_CALLBACK:
	case STREAM3_CALLBACK:
		p->mpx_desc.cb_str[callbackType-STREAM0_CALLBACK] = callbackFunction;
		p->mpx_desc.cb_str_priv[callbackType-STREAM0_CALLBACK] = privData;
		break;
	case AUDIO0_CALLBACK:
	case AUDIO1_CALLBACK:
	case AUDIO2_CALLBACK:
	case AUDIO3_CALLBACK:
		p->mpx_desc.cb_audio[callbackType-AUDIO0_CALLBACK] = callbackFunction;
		p->mpx_desc.cb_audio_priv[callbackType-AUDIO0_CALLBACK] = privData;
		break;
	case TEXT0_CALLBACK:
	case TEXT1_CALLBACK:
	case TEXT2_CALLBACK:
	case TEXT3_CALLBACK:
		p->mpx_desc.audiotext[callbackType-TEXT0_CALLBACK].cb_txt = callbackFunction;
		p->mpx_desc.audiotext[callbackType-TEXT0_CALLBACK].cb_txt_priv = privData;
		break;

	case SIGINFO_CALLBACK:
		p->cb_siginfo = callbackFunction;
		p->cb_siginfo_priv = privData;
		break;
	case SIGEQ_CALLBACK:
		p->cb_sigeq = callbackFunction;
		p->cb_sigeq_priv = privData;
		break;
	case SIGSP_CALLBACK:
		p->cb_sigsp = callbackFunction;
		p->cb_sigsp_priv = privData;
		break;
	case SIGIQ_CALLBACK:
		p->cb_sigiq = callbackFunction;
		p->cb_sigiq_priv = privData;
		break;
	default:
		ERRORLOG("Unknown callback type %d", callbackType);
		return DRMPLUS_ERR;
		break;
	}

	return DRMPLUS_OK;
}

int drmplusResetSync(drmplusHandle hp, uint32_t flags)
{
    drmplus_priv_t *p = (drmplus_priv_t *) hp;
    if(!p) return 0;

    p->siginfo.sync_state = flags;
    p->symbol_id=0;
    p->frame_num=-1;
    p->fac_collected=0;
    p->sdc_collected=0;
    p->msc_collected=0;
    p->siginfo.dc_freq_coarse=0;
    p->siginfo.dc_freq_fine=0;
    p->siginfo.fac_crc_ok=0;
    p->siginfo.sdc_crc_ok=0;
    p->sdc_size=0;

    if(flags < 10) {
        p->siginfo.dc_freq_coarse=0;
        p->cfe_freq_last=0;
        p->cfe_freq_ok=0;
        p->cfe_maxenergy_cnt=0;
        p->cfe_maxenergy_of4[0]=0;p->cfe_maxenergy_of4[1]=0;
        p->cfe_maxenergy_freq[0]=-1;p->cfe_maxenergy_freq[1]=-1;
    }

    if(flags == 0) {
        memset(&p->mpx_desc, 0x00, sizeof(mpx_desc_t));
        memset(p->services, 0x00, sizeof(srv_data_t)*4);
    }

}

int drmplusAddSamplesShort(drmplusHandle hp, int16_t *inputBuffer, unsigned int samplesInput)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;

	if(samplesInput!=480) return DRMPLUS_ERR;

        double tmpbuff[480*2];
	int i;
	for(i=0;i<samplesInput*2;i++) {
		tmpbuff[i] = ((double)inputBuffer[i]) / 32768.0;
		//tmpbuff[480+i*2+1] = ((double)inputBuffer[2*i + 1]) / 32768.0;
	}
	return drmplusAddSamplesDouble(hp, tmpbuff, samplesInput);
}


int drmplusAddSamplesDouble(drmplusHandle hp, double *inputBuffer, unsigned int samplesInput)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;

	if(samplesInput!=480) return DRMPLUS_ERR;
	int i;
#if 0
	for(i=0;i<samplesInput;i++) {
		//NOTE: if we have 4 symblos ifo estimation, we use different method of detecting and using of spectrum inversion...
		if(!p->siginfo.spectrum_inverted || p->cfg.ifoEstimationType == IFO_EST_4SYMBOLS) {
			p->frame_in[480+i][0] = inputBuffer[2*i];
			p->frame_in[480+i][1] = inputBuffer[2*i + 1];
		} else {
			//I/Q swapped in case of inverted spectrum
			p->frame_in[480+i][1] = inputBuffer[2*i];
			p->frame_in[480+i][0] = inputBuffer[2*i + 1];
		}
	}
#else
    memcpy(&p->frame_in[480][0], inputBuffer, samplesInput*sizeof(fftw_complex));
#endif
	drm_move_to_base_band(p, &p->frame_in[480]);

	int old_ts = p->siginfo.fine_timeshift;
	p->siginfo.fine_timeshift=drm_fine_time_sync(&p->frame_in, p->siginfo.sync_state > SYNC_STATE_FTO_TRY ? old_ts : -1);
	if(abs(p->siginfo.fine_timeshift - old_ts) <= 8 ||
			//NOTE: after coarse frequency sync done, it's possible that this offset can be higher.
			p->siginfo.sync_state == SYNC_STATE_IFO_TRY) {
		if(p->siginfo.sync_state < SYNC_STATE_FTO_TRY) {

			if(p->cb_siginfo && p->siginfo.sync_state == SYNC_STATE_NULL)
				p->cb_siginfo(p->cb_siginfo_priv, &p->siginfo, SIGINFO_CALLBACK, NULL);

			p->siginfo.sync_state+=2;
		} else if(p->siginfo.sync_state == SYNC_STATE_FTO_TRY) {
			INFOLOG("TIME SYNC DONE!");
			//p->siginfo.sync_state++;
			p->siginfo.sync_state = SYNC_STATE_FTO_DONE;
			p->cfe_maxenergy_cnt=0;
			p->cfe_freq_ok=0;
			p->cfe_freq_last = -1;
			p->cfe_maxenergy_of4[0]=0;p->cfe_maxenergy_of4[1]=0;
			p->cfe_maxenergy_freq[0]=-1;p->cfe_maxenergy_freq[1]=-1;
			//p->frame_position = 0;
			//p->force_timesync = 1;
			//p->coarse_freq_max=0.0;
			//p->coarse_freq_max_frame_pos=0;

			if(p->cb_siginfo)
				p->cb_siginfo(p->cb_siginfo_priv, &p->siginfo, SIGINFO_CALLBACK, NULL);
		}
	} else {
		p->siginfo.sync_state=SYNC_STATE_NULL;
		p->cfe_maxenergy_cnt=0;
		INFOLOG("TIME SYNC FAIL!");
		if(p->cb_siginfo && p->siginfo.sync_state != SYNC_STATE_NULL)
			p->cb_siginfo(p->cb_siginfo_priv, &p->siginfo, SIGINFO_CALLBACK, NULL);
	}

	double ffe = drm_fine_freq_err(p->frame_in, p->siginfo.fine_timeshift);
#if 0
	fprintf(stderr,"start at %3d | DC: ifo:%d*444.4 + ffo:%.1f + ffo2:%.1f => %.1f\n", p->siginfo.fine_timeshift,
			p->siginfo.dc_freq_coarse, p->siginfo.dc_freq_fine, ffe,
			p->siginfo.dc_freq_coarse*192000.0/432.0 + p->siginfo.dc_freq_fine + ffe);
#endif

	 //+ p->siginfo.dc_freq_coarse*192000.0/432.0;


//	if(ffe > 444)
//		ffe=444.44444;
//	if(ffe < -444)
//		ffe=-444.44444;

	if(p->siginfo.sync_state==SYNC_STATE_IFO_TRY && p->symbol_num==1)
		fprintf(stderr,"ignore initial freq. drift, DC: %.1f Hz\n", p->siginfo.dc_freq_coarse*192000.0/432.0 + p->siginfo.dc_freq_fine);
	else {
		p->siginfo.dc_freq_fine += 0.5*ffe;
	}
#if 0
	if( p->dc_freq_fine > 192000.0/(432.0*2) ) {
		p->dc_freq_fine -= 192000.0/(432.0*2);
		p->dc_freq_coarse++;
	} else if( p->dc_freq_fine < -192000.0/(432.0*2) ) {
		p->dc_freq_fine += 192000.0/(432.0*2);
		p->dc_freq_coarse--;
	}
#endif
    if(p->siginfo.sync_state <= SYNC_STATE_FTO_TRY && !p->cb_sigsp)
        goto away_10;


       //now do FFT for CFE estimation and processing...
	fftw_plan fftPlan = fftw_plan_dft_1d(432, &p->frame_in[p->siginfo.fine_timeshift+48], p->symbol, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(fftPlan);
	fftw_destroy_plan(fftPlan);
	//positive/negative freq. reorder
	for (i = 0; i < 432/2; i++) {
		fftw_complex tmp;
		tmp[0]     = p->symbol[i][0];
		tmp[1]     = p->symbol[i][1];
		p->symbol[i][0]    = p->symbol[i+432/2][0];
		p->symbol[i][1]    = p->symbol[i+432/2][1];
		p->symbol[i+432/2][0] = tmp[0];
		p->symbol[i+432/2][1] = tmp[1];
	}

	double spectrum[432];
	if(p->cb_sigsp || p->siginfo.sync_state == SYNC_STATE_FTO_DONE) {
		for (i = 0; i < 432; i++)
			spectrum[i] = p->symbol[i][0]*p->symbol[i][0] + p->symbol[i][1]*p->symbol[i][1];
	}

	//send spectrum to drawing function
	if(p->cb_sigsp)
		p->cb_sigsp(p->cb_sigsp_priv, spectrum, SIGSP_CALLBACK, NULL);

	if(p->siginfo.sync_state <= SYNC_STATE_FTO_TRY)
		goto away_10;

	if(p->siginfo.sync_state == SYNC_STATE_FTO_DONE) {
		int ret;
		switch(p->cfg.ifoEstimationType) {
		case IFO_EST_MAXENERGY:
			ret = drm_coarse_freq_sync_maxenergy(p, spectrum);
			break;
		case IFO_EST_4SYMBOLS:
			ret = drm_coarse_freq_sync_4symbols(p, spectrum);
			break;
		default:
			fprintf(stderr, "Unknown IFO estimation type: %d\n", p->cfg.ifoEstimationType);
			return DRMPLUS_ERR;
			break;
		}
		if(ret) {
			//p->cfe_freq_last

			p->siginfo.sync_state=SYNC_STATE_IFO_TRY;
			if(p->cb_siginfo)
				p->cb_siginfo(p->cb_siginfo_priv, &p->siginfo, SIGINFO_CALLBACK, NULL);
		}
		p->symbol_num=0;
	}


	if(p->siginfo.sync_state < SYNC_STATE_IFO_TRY)
		goto away_11;



	if(p->siginfo.dc_freq_fine >= (192000.0/432.0)/2) {
		p->siginfo.dc_freq_fine -= 192000.0/432.0;
		p->siginfo.dc_freq_coarse++;
		if(p->siginfo.dc_freq_coarse >= 110) {
			fprintf(stderr, "IFO: %d overflow, resetting!\n", p->siginfo.dc_freq_coarse);
			p->siginfo.sync_state = SYNC_STATE_FTO_DONE; //recheck IFO
			p->siginfo.dc_freq_coarse=0;
			goto away_10;
		}


	} else if(p->siginfo.dc_freq_fine < -(192000.0/432.0)/2) {
		p->siginfo.dc_freq_fine += 192000.0/432.0;
		p->siginfo.dc_freq_coarse--;
		if(p->siginfo.dc_freq_coarse <= -110) {
			fprintf(stderr, "IFO: %d underflow, resetting!\n", p->siginfo.dc_freq_coarse);
			p->siginfo.sync_state = SYNC_STATE_FTO_DONE; //recheck IFO
			p->siginfo.dc_freq_coarse=0;
			goto away_10;
		}
	}

#if 0
	fprintf(stderr,"DC: ifo:%d*444.4 + ffo:%.1f + ffo2:%.1f => %.1f + %.1f => %.1f\n",
			p->siginfo.dc_freq_coarse, p->siginfo.dc_freq_fine, ffe,
			192000.0/432.0 * (double)p->siginfo.dc_freq_coarse, p->siginfo.dc_freq_fine + ffe, 192000.0/432.0 * (double)p->siginfo.dc_freq_coarse + p->siginfo.dc_freq_fine + ffe);
#endif

	int symbol_id = -1;
	int is_inverted = -1;
	if((symbol_id = drm_coarse_time_sync(p, &p->symbol[p->siginfo.dc_freq_coarse + 110], &is_inverted)) >= 0) {
		if(p->siginfo.sync_state == SYNC_STATE_IFO_TRY && p->symbol_num >= 40) {
			fprintf(stderr, "initial coarse time sync done!\n");
	    	p->siginfo.spectrum_inverted=is_inverted;

			p->siginfo.sync_state=SYNC_STATE_IFO_DONE;
			p->symbol_id=symbol_id == 0 ? 39 : symbol_id-1;
			p->frame_num=-1;
			p->fac_collected=0;
			p->sdc_collected=0;
			p->msc_collected=0;
		}
	}


	//check that we have needed symbol
	if(p->siginfo.sync_state > SYNC_STATE_IFO_TRY && symbol_id != (p->symbol_id+1)%40) {
		//fprintf(stderr, "symbol id mismatch: %d != %d\n", symbol_id, (p->symbol_id+1)%40);
		//don't do anything here, just wait for FAC CRC error...
		//p->time_synced_count=11;
		p->symbol_id = (p->symbol_id+1)%40;
		//force possibly correct symbol id
		symbol_id = p->symbol_id;
	} else {
		p->symbol_id = symbol_id;
	}

	if(p->siginfo.sync_state < SYNC_STATE_IFO_DONE)
		goto away_12;


	//calculate SNR based on C/N ratio.
	//p->siginfo.snr_signal=0;
	//p->siginfo.snr_noise=0;
	//TODO: remove this, do calculation in spectrum routine (little bit upper)
	int signal_calcs=0;
	int noise_calcs=0;
	for(i=0;i<432;i++) {
		double val = sqrt(p->symbol[i][0]*p->symbol[i][0] + p->symbol[i][1]*p->symbol[i][1]);
		if(i>=p->siginfo.dc_freq_coarse + 110 && i<p->siginfo.dc_freq_coarse + 110+213) {
			p->siginfo.snr_signal += val;
			signal_calcs++;
		} else {
			p->siginfo.snr_noise += val;
			noise_calcs++;
		}
	}
	p->siginfo.snr_signal =  p->siginfo.snr_signal / (signal_calcs*2);
	p->siginfo.snr_noise = p->siginfo.snr_noise / (noise_calcs*2);

	//swap symbols if spectrum inversion is detected.
	//TODO: swap only part of spectrum: 110...110+213
	if(p->siginfo.spectrum_inverted > 0 && p->cfg.ifoEstimationType == IFO_EST_4SYMBOLS) {
		fftw_complex tmp;
		for (i=0;i<106;i++) {
			tmp[0] = p->symbol[p->siginfo.dc_freq_coarse + 110+i][0];
			tmp[1] = p->symbol[p->siginfo.dc_freq_coarse + 110+i][1];
			p->symbol[p->siginfo.dc_freq_coarse + 110+i][1] = p->symbol[p->siginfo.dc_freq_coarse + 322-i][0];
			p->symbol[p->siginfo.dc_freq_coarse + 110+i][0] = p->symbol[p->siginfo.dc_freq_coarse + 322-i][1];
			p->symbol[p->siginfo.dc_freq_coarse + 322-i][1] = tmp[0];
			p->symbol[p->siginfo.dc_freq_coarse + 322-i][0] = tmp[1];

		}
	}


	drm_normalize_correct_phase(&p->symbol[p->siginfo.dc_freq_coarse + 110], symbol_id);

	switch(p->cfg.equalizerType) {
	case EQ_TYPE_SIMPLE:
		drm_equalize_feed_pilots_simple(&p->symbol[p->siginfo.dc_freq_coarse + 110], symbol_id, p->H_pilots, p->frame_num==-1);
		break;
	case EQ_TYPE_LIN_INTERP:
	case EQ_TYPE_LIN_INTERP_MEM:
		drm_equalize_feed_pilots_lin_int(&p->symbol[p->siginfo.dc_freq_coarse + 110], symbol_id, p->H_pilots, p->frame_num==-1, p->cfg.equalizerType==EQ_TYPE_LIN_INTERP_MEM ? 1 : 0);
		break;
	default:
		fprintf(stderr, "equalizer type is not supported: %d\n", p->cfg.equalizerType);
		break;
	}

	drm_equalize_symbol(&p->symbol[p->siginfo.dc_freq_coarse + 110], p->H_pilots);

	if(p->cb_sigeq)
		p->cb_sigeq(p->cb_sigeq_priv, &p->H_pilots[0][0], SIGEQ_CALLBACK, NULL);

	uint8_t markers[215];
	markers[213] = symbol_id;
	markers[214] = p->frame_num;
	memset(markers, CELL_TYPE_NONE, 213);
	int ts_idx=0;
	int pil_idx = ((symbol_id+3)%4)*4;
	int cell_id;
	for(cell_id=0; cell_id < 213; cell_id++) {

		if(cell_id==0 && symbol_id==0) {
			p->fac_collected=0;
			p->sdc_collected=0;
			if(p->frame_num < 1)
				p->msc_collected=0;
		}

		//mark cell depends on it's position+symbol_id+frame_num...
		if(cell_id==pil_idx) {
			if(pil_idx==0 || pil_idx==4 || pil_idx==208 ||pil_idx==212)
				markers[pil_idx] = CELL_TYPE_PIL2X;
			else
				markers[pil_idx] = CELL_TYPE_PIL;
			pil_idx+=16;
		} else if(symbol_id==0 && ts_idx < 21 && timesync_k[ts_idx] + 106 == cell_id) {
			markers[cell_id] = CELL_TYPE_PIL;
			ts_idx++;
		} else if(((p->frame_num==0 && symbol_id==4) || (p->frame_num==3 && symbol_id==39)) && (cell_id % 4 == 0)) {
			markers[cell_id] = CELL_TYPE_AFS;
		} else if((p->frame_num==-1 && symbol_id==4) && (cell_id % 4 == 0)) {
			markers[cell_id] = CELL_TYPE_AFS_OR_MSC;
		} else if(p->frame_num==0 && symbol_id <= 4) {
			markers[cell_id] = CELL_TYPE_SDC;
		} else if(p->frame_num==-1 && symbol_id <= 4) {
			markers[cell_id] = CELL_TYPE_SDC_OR_MSC;
		} else if(symbol_id > 4 && (symbol_id < 26 || (symbol_id==26 && cell_id <= 48)) && cell_id >= 16 && cell_id <= 196 && cell_id+4 == pil_idx) {
			markers[cell_id] = CELL_TYPE_FAC;
		} else {
			markers[cell_id] = CELL_TYPE_MSC;
		}


		int8_t r0, r1;
		//TODO: add support for QAM16
		if(markers[cell_id] > CELL_TYPE_AFS)
			drm_decode_qam4(p->symbol[p->siginfo.dc_freq_coarse + 110 + cell_id], &r0, &r1);

		switch(markers[cell_id]) {
		case CELL_TYPE_FAC:
			if(p->fac_collected<244) {
				p->fac_deinterleaved[fac_perm[p->fac_collected*2]] = -r0;
				p->fac_deinterleaved[fac_perm[p->fac_collected*2+1]] = -r1;
			}

			p->fac_collected++;
			if (p->fac_collected==244){
				//fprintf(stderr, "Processing FAC %lu bytes...\n", sizeof(dense_fac_t));
				p->siginfo.fac_crc_ok = drm_process_fac(p->fac_deinterleaved, (uint8_t *) &p->fac_data);
				uint8_t fac_num_bytes = 15;

				if(p->cb_siginfo)
					p->cb_siginfo(p->cb_siginfo_priv, (void *) &p->siginfo, SIGINFO_CALLBACK, NULL);

				if(p->cb_fac && (p->siginfo.fac_crc_ok || p->cfg.callIfBadCrc))
					p->cb_fac(p->cb_fac_priv, (void *) &p->fac_data, FAC_CALLBACK, (void *) &fac_num_bytes);

				if(p->siginfo.fac_crc_ok) {
					//fprintf(stderr, "FAC OK!\n");
					p->mpx_desc.info.msc_mode = p->fac_data.msc_mode;
					p->mpx_desc.info.sdc_mode = p->fac_data.sdc_mode;
					uint8_t frame_num = 0;
					if(p->fac_data.id==2)
						frame_num = 3;
					else if(p->fac_data.id==1)
						frame_num = 2 - p->fac_data.toggle_flag;
					
					//fprintf(stderr, "frame_num=%d\n", frame_num);
					//fprintf(stderr, "msc_collected[%d][%d]=%d\n", symbol_id, frame_num, p->msc_collected);
					int srv_num_changed = drm_fill_services_from_fac(&p->fac_data, p->services, &p->mpx_desc.info);
					if(srv_num_changed) {
						p->services[0].flags=0;
						p->services[1].flags=0;
						p->services[2].flags=0;
						p->services[3].flags=0;
					}

					if(p->frame_num == -1) {
						//TODO: If known frame_id == 0, then:
						// * SDC can be parsed.
						// * MSC cells must be moved backward by SDC+AFS amount of cells.
						const uint16_t msc_initial_for_fac[4] = {3991, 4246, 4501, 4756};
						p->msc_collected=msc_initial_for_fac[frame_num];
						//msc_collected[26][0]=3991
						//PROCESS MSC at frame:1, symbol:3, cell:154
						//msc_collected[26][1]=4246
						//PROCESS MSC at frame:2, symbol:2, cell:94
						//msc_collected[26][2]=4501
						//PROCESS MSC at frame:3, symbol:1, cell:35
						//msc_collected[26][3]=4756
						//PROCESS MSC at frame:3, symbol:39, cell:209

					} else if(p->frame_num != frame_num) {
						fprintf(stderr, "FAC discontinuity %d != %d!\n", p->frame_num, frame_num);
					}
					p->frame_num=frame_num;

					p->siginfo.fac_errors=0;
				} else {
					fprintf(stderr, "FAC CRC Error, resetting synchronisation????\n");

					if(p->cfg.ifoEstimationType == IFO_EST_MAXENERGY) {
						//p->time_synced_count=11;
						p->siginfo.sync_state=SYNC_STATE_NULL; // FIXME: too hard reset?
						//p->siginfo.sync_state=SYNC_STATE_IFO_TRY;
						//p->siginfo.sync_state=SYNC_STATE_FTO_DONE;
						p->frame_num = -1;

						if(p->siginfo.fac_errors < 16) {
							p->siginfo.fac_errors++;
						} else {
							fprintf(stderr, "Trying spectrum inversion...\n");
							p->siginfo.spectrum_inverted = !p->siginfo.spectrum_inverted;
							p->siginfo.fac_errors=0;
						}

						goto away_10;
					} else {
#if 1
						if(p->siginfo.fac_errors < 4) {
							p->siginfo.fac_errors++;
						} else {
							p->siginfo.sync_state=SYNC_STATE_NULL; // FIXME: too hard reset?
							p->frame_num = -1;
							p->siginfo.fac_errors=0;
							goto away_10;
						}
#else
						p->siginfo.sync_state=SYNC_STATE_FTO_DONE; // FIXME: too hard reset?
#endif

					}
					//drmplusResetSync(p, 0);
				}

				//exit(1);
			} else if (p->fac_collected>244) {
				fprintf(stderr, "ERROR: too much FAC cells %d/%d\n", p->fac_collected, 244);
			}


			//it's time to parse SDC....
			if(p->frame_num==0 && p->sdc_collected == 936) {
				p->siginfo.sdc_crc_ok = drm_process_sdc(p->sdc_deinterleaved, p->sdc_data, p->fac_data.sdc_mode);
				uint8_t sdc_num_bytes = p->fac_data.sdc_mode ? (55+3) : (113+3);

				if(p->cb_sdc && (p->siginfo.sdc_crc_ok || p->cfg.callIfBadCrc))
					p->cb_sdc(p->cb_sdc_priv, (void *) &p->sdc_data, SDC_CALLBACK, (void *) &sdc_num_bytes);

				if(p->siginfo.sdc_crc_ok) {
					p->sdc_size = sdc_num_bytes;
					if(drm_fill_services_from_sdc(p->sdc_data, sdc_num_bytes, p->services, &p->mpx_desc)==1) {
						//TODO: run service list callback...
						if(p->cb_strlist)
							p->cb_strlist(p->cb_strlist_priv, (void *) p->services, STRLIST_CALLBACK, (void *) &p->mpx_desc.info);

					}
				}
				p->sdc_collected=0;
			}


			break;
		case CELL_TYPE_SDC:
			if(p->sdc_collected<936) {
				p->sdc_deinterleaved[sdc_perm[p->sdc_collected*2]] = -r0;
				p->sdc_deinterleaved[sdc_perm[p->sdc_collected*2+1]] = -r1;
			}

			p->sdc_collected++;
			if (p->sdc_collected>936)
				fprintf(stderr, "ERROR: too much SDC cells %d/%d\n", p->sdc_collected, 936);
			break;
		case CELL_TYPE_SDC_OR_MSC:
			//add this cells to both: MSC and SDC buffers
			if(p->sdc_collected<936) {
				p->sdc_deinterleaved[sdc_perm[p->sdc_collected*2]] = -r0;
				p->sdc_deinterleaved[sdc_perm[p->sdc_collected*2+1]] = -r1;
			}
			p->sdc_collected++;
			if (p->sdc_collected>936)
				fprintf(stderr, "ERROR: too much SDC|MSC cells %d/%d\n", p->sdc_collected, 936);
			//no break
		case CELL_TYPE_AFS_OR_MSC:
			if(p->frame_num == -1) {
				//fprintf(stderr, "Not known exactly which is cell[%i] SDC/AFS or MSC...\n", cell_id);
			};
			//no break
		case CELL_TYPE_MSC:
			if(p->msc_collected<7460) {
				//fill cell history
				p->z_history[5][p->msc_collected][0] = -r0;
				p->z_history[5][p->msc_collected][1] = -r1;

				//request cell from history
				p->msc_bits[msc_perm[p->msc_collected]][0] = p->z_history[p->msc_collected % 6][p->msc_collected][0];
				p->msc_bits[msc_perm[p->msc_collected]][1] = p->z_history[p->msc_collected % 6][p->msc_collected][1];
			}
			p->msc_collected++;

			if(p->msc_collected==7460) {
				/* shift history pointers */
				int xx;
				for ( xx = 0; xx < 5; xx++ ) {
					int yy;
					for ( yy = 0; yy < 7460; yy++) {
						p->z_history[xx][yy][0] = p->z_history[xx+1][yy][0];
						p->z_history[xx][yy][1] = p->z_history[xx+1][yy][1];
					}
					//memcpy(&p->z_history[i][0][0], &p->z_history[i+1][0][0], sizeof(int8_t)*2*7460);
				}
				//fprintf(stderr, "PROCESS MSC at frame:%d, symbol:%d, cell:%d\n", p->frame_num, symbol_id, cell_id);
				if(p->fac_data.msc_mode != 3) {
					fprintf(stderr, "MSC mode %u is not supported!\n",  p->fac_data.msc_mode);
				} else {

					int got_bytes = drm_process_msc(p->msc_bits, p->msc_data, &p->mpx_desc, p->services);
					//fprintf(stderr, "MSC processed: got %d bytes\n", got_bytes);

					if(got_bytes && p->cb_msc)
						p->cb_msc(p->cb_msc_priv, (void *) &p->msc_data, MSC_CALLBACK, (void *) &got_bytes);

					//split streams
					int srv;
					int srv_pos_A=0;
					int srv_pos_B=p->mpx_desc.msc_len_A;
					for(srv=0;got_bytes && srv<p->mpx_desc.info.service_num_a+p->mpx_desc.info.service_num_d;srv++) {

						//look if this service is needed by any callback...
						if(p->mpx_desc.cb_str[srv] || p->mpx_desc.cb_txt[srv] ||  p->mpx_desc.cb_audio[srv]) {
							//get AAC frames...
							if(p->services[srv].audioDataFlag==0 &&
							   p->services[srv].audioCoding == 0) {
								int crc_ok=drm_feed_aac_service(&p->msc_data[srv_pos_A], &p->msc_data[srv_pos_B], &p->mpx_desc, &p->services[srv]);

								//inform app, that we have crc error
								if(p->cb_siginfo && p->siginfo.msc_crc_ok != crc_ok)
									p->cb_siginfo(p->cb_siginfo_priv, &p->siginfo, SIGINFO_CALLBACK, NULL);
								p->siginfo.msc_crc_ok = crc_ok;
							}
						}
						srv_pos_A+=p->mpx_desc.info.length_A[srv];
						srv_pos_B+=p->mpx_desc.info.length_B[srv];
					}
				}
				p->msc_collected=0;
			}

			break;
		default:
			break;
		}
	}

	if(p->cb_sigiq)
		p->cb_sigiq(p->cb_sigiq_priv, &p->symbol[p->siginfo.dc_freq_coarse + 110][0], SIGIQ_CALLBACK, (void *)markers);

	if(p->siginfo.sync_state == SYNC_STATE_IFO_DONE)
		p->siginfo.sync_state = SYNC_STATE_DONE;

away_12:
away_11:
	//increase frame number
	if(p->siginfo.sync_state == SYNC_STATE_DONE && symbol_id==39 && p->frame_num >= 0) {
		p->frame_num = (p->frame_num+1) % 4;
	}
	p->symbol_num++;

away_10:
    //use this saved memory for better cp correlation

	/* TODO: if sync_state > SYNC_STATE_FTO_TRY and there is drift in position
	 * of start symbol and it becames lower or higher than specific range,
	 * then move frame left or right and fix fine_timeshift.
	 * p->siginfo.fine_timeshift= ... something lower or bigger than real gotten
	 * p->frame_in moved left or right.
	 * drm_fine_time_sync(&p->frame_in, p->siginfo.sync_state > SYNC_STATE_FTO_TRY ? old_ts : -1);
	 */
    memcpy(p->frame_in, &p->frame_in[480], sizeof(fftw_complex)*480);

	return DRMPLUS_OK;
}





int drmplusClose(drmplusHandle hp)
{
	drmplus_priv_t *p = (drmplus_priv_t *) hp;

	//free permutation tables if needed...
	if(p->mpx_desc.msc_perm_table_A) {
		free(p->mpx_desc.msc_perm_table_A);
		p->mpx_desc.msc_perm_table_A=NULL;
	}

	if(p->mpx_desc.msc_perm_table_B) {
		free(p->mpx_desc.msc_perm_table_B);
		p->mpx_desc.msc_perm_table_B=NULL;
	}

	int i;
	for (i=0;i<4;i++) {
		if(p->mpx_desc.str_buff[i] && p->mpx_desc.str_buff_len[i] > 0) {
			free(p->mpx_desc.str_buff[i]);
			p->mpx_desc.str_buff[i]=NULL;
		}
	}


	free(p);
	return DRMPLUS_OK;
}
