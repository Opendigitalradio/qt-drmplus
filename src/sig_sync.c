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

#if 0
fftw_complex PILOTS[40][14];
void drm_create_phaseref_tables()
{
	int Kin, k, n, s, m, p;
	double a, f;

	memset(PILOTS, 0x00, sizeof(fftw_complex)*40*14);
//    gnuplot_ctrl *h1 = gnuplot_init() ;

//	printf("ts_pil[40][213] = {\n");
	int ts_pil_idx = 0;
	for(s = 0; s < 40; s++) {
		//printf("[%3d]:", s);
//		printf(" { ");
		//int last_k = -200;

//		double re[213];
//		double im[213];
//		int reim_idx = 0;

		for (Kin = -106; Kin <= 106; Kin++) {
		//for (Kin = 0; Kin < 213; Kin++) {
			n = s % 4;
			m = s / 4;
			p = (Kin - 2 - n*4) / (4*4);

			k = 2 + 4 * (s%4) + 16 * p;

//			PILOTS[s][Kin+106][0] = 0.0;
//			PILOTS[s][Kin+106][1] = 0.0;
#if 0
			if(Kin == k) {
				if(k == -106 || k == -102 || k == 102 || k == 106) {
					//a = 2.0;
					printf("O");
				} else {
					printf("o");
				}
				//A1024[s][Kin+106] = a;
				//int phaseVal = ((p*p) * R1024[n][m] + abs(p)*Z1024[n][m] + Q1024[n][m]);
				//F1024[s][Kin+106] = phaseVal % 1024;
			} else {
				printf(".");
			}
#endif

#if 1
			if(Kin == k) {
				int phaseVal = ((p*p) * R1024[n][m] + p*Z1024[n][m] + Q1024[n][m]) % 1024;
				//if(phaseVal < 0) phaseVal += 1024;
				//F1024[s][Kin+106] = phaseVal % 1024;
				double amplitudeVal = M_SQRT2;
				if(k == -106 || k == -102 || k == 102 || k == 106) {
					amplitudeVal = 2;
				}
//				PILOTS[s][Kin+106][0] = amplitudeVal * cos((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
//				PILOTS[s][Kin+106][1] = amplitudeVal * sin((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
				//printf("%5d", phaseVal);

//				if(s==0) {
//					TIMESYNC_PILOTS[Kin+106][0] = PILOTS[s][Kin+106][0];
//					TIMESYNC_PILOTS[Kin+106][1] = PILOTS[s][Kin+106][1];
//				}
#if 0
				if(s==4) {
					AFS4_PILOTS[Kin+106][0] = PILOTS[s][Kin+106][0];
					AFS4_PILOTS[Kin+106][1] = PILOTS[s][Kin+106][1];
				}
#endif
//				printf("{%1.2f, %1.2f}, ", PILOTS[s][Kin+106][0], PILOTS[s][Kin+106][1]);
//				re[reim_idx] =  PILOTS[s][Kin+106][0];
//				im[reim_idx] =  PILOTS[s][Kin+106][1];
//				reim_idx++;
			}
#endif
//			if(s==0 && Kin == timesync_k[ts_pil_idx]) {
//				TIMESYNC_PILOTS[Kin+106][0] = M_SQRT2 * cos((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
//				TIMESYNC_PILOTS[Kin+106][1] = M_SQRT2 * sin((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
//				//TIMESYNC_PILOTS[Kin+106] = Polar2Cart(M_SQRT2, timesync_phi[ts_pil_idx]);
//				//TIMESYNC_PILOTS2[ts_pil_idx] = Polar2Cart(SQRT2, timesync_phi[ts_pil_idx]);
//				ts_pil_idx++;
//			}
#if 0
			if(s==4 && Kin == timesync_k[ts_pil_idx]) {
				TIMESYNC_PILOTS[Kin+106][0] = M_SQRT2 * cos((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				TIMESYNC_PILOTS[Kin+106][1] = M_SQRT2 * sin((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				//TIMESYNC_PILOTS[Kin+106] = Polar2Cart(M_SQRT2, timesync_phi[ts_pil_idx]);
				//TIMESYNC_PILOTS2[ts_pil_idx] = Polar2Cart(SQRT2, timesync_phi[ts_pil_idx]);
				ts_pil_idx++;
			}
#endif
		}
//		printf("}, \n");

//	    gnuplot_plot_xy(h1, re, im, reim_idx, "I/Q");
//		sleep(1);
		//gnuplot_resetplot(h1);
	}
	//gnuplot_close(h1);
#if 0
	printf("TIMESYNC_PILOTS[432] = {\n");
	for (Kin = -106; Kin <= 106; Kin++) {
		printf("{%.18e,%.18e},\n", TIMESYNC_PILOTS[Kin+106][0], TIMESYNC_PILOTS[Kin+106][1]);
	}
	printf("}\n");
#endif
}
#endif


void drm_move_to_base_band(drmplus_priv_t *p, fftw_complex *frame_out)
{
	double angIncr = 2*M_PI*(p->siginfo.dc_freq_fine)/( (double) 192000.0);
	int i;
#if 1
	for (i=0;i<480;i++) {
		p->bb_angle += angIncr;
		 //complex multiply
		fftw_complex shift_signal = {cos(-p->bb_angle), sin(-p->bb_angle)};
		p->frame_shifted[i+16][0] = shift_signal[0]*p->frame_in[480+i][0] - shift_signal[1]*p->frame_in[480+i][1];
		p->frame_shifted[i+16][1] = shift_signal[0]*p->frame_in[480+i][1] + shift_signal[1]*p->frame_in[480+i][0];
//		frame_out[i][0] = shift_signal[0]*p->frame_in[480+i][0] - shift_signal[1]*p->frame_in[480+i][1];
//		frame_out[i][1] = shift_signal[0]*p->frame_in[480+i][1] + shift_signal[1]*p->frame_in[480+i][0];
	}

	memcpy(frame_out, p->frame_shifted, sizeof(fftw_complex)*480);
	memcpy(p->frame_shifted, &p->frame_shifted[480], sizeof(fftw_complex)*16);
#else
	for (i=0;i<480;i++) {
		p->bb_angle += angIncr;
		 //complex multiply
		fftw_complex shift_signal = {cos(-p->bb_angle), sin(-p->bb_angle)};
		//p->frame_shifted[i+16][0] = shift_signal[0]*p->frame_in[480+i][0] - shift_signal[1]*p->frame_in[480+i][1];
		//p->frame_shifted[i+16][1] = shift_signal[0]*p->frame_in[480+i][1] + shift_signal[1]*p->frame_in[480+i][0];
		frame_out[i][0] = shift_signal[0]*p->frame_in[480+i][0] - shift_signal[1]*p->frame_in[480+i][1];
		frame_out[i][1] = shift_signal[0]*p->frame_in[480+i][1] + shift_signal[1]*p->frame_in[480+i][0];
	}

	//memcpy(frame_out, p->frame_shifted, sizeof(fftw_complex)*480);
	//memcpy(p->frame_shifted, &p->frame_shifted[480], sizeof(fftw_complex)*16);
#endif
}


int32_t drm_fine_time_sync(fftw_complex * frame, int prev_offset){

	long i, j, indexSmallestCorr = -1;
	double currentCorr, smallestCorr;

	smallestCorr=1e50;

	int offsetadd = (prev_offset < 8) ? 0 : -8;
		offsetadd = (prev_offset < 0) ? 1 : offsetadd;
    //int offsetsub = (prev_offset < 0) ? -480 : -8;
        int offsetsub = (prev_offset < 0) ? -432 : -8;
//	printf("lookup: %d...%d\n", prev_offset+offsetadd, prev_offset-offsetsub);
	for (i=prev_offset+offsetadd; i<prev_offset-offsetsub; i++)
	{
		currentCorr=0;
		for (j=i; j< i + 48; j+=1) //improve speed be increasing step
		{
			fftw_complex res;
			//currentCorr += fabs(samples[j] - samples[j+432]);
			res[0] = fabs(frame[j][0] - frame[j+432][0]);
			res[1] = fabs(frame[j][1] - frame[j+432][1]);
			currentCorr += sqrt(res[0]*res[0] + res[1]*res[1]);

			//currentCorr += fabs();
			if (currentCorr > smallestCorr) break;
		}
		if (currentCorr < smallestCorr)
		{
			smallestCorr = currentCorr;
			indexSmallestCorr = i;
		}
//		printf("%f ",currentCorr);
	}
//	INFOLOG("Start of symbol: %d (%.4f)", indexSmallestCorr, smallestCorr);
	//printf("%d ", indexSmallestCorr+48);
	return indexSmallestCorr;
}



double drm_fine_freq_err(fftw_complex * drm_frame,int32_t fine_timeshift)
{
  fftw_complex left[48];
  fftw_complex right[48];
  fftw_complex lr[48];
  fftw_complex angle2 = {0.0, 0.0};
  //double angle[48];
  double mean=0;
  double ffe1,ffe2;

  uint32_t i;

  for (i=0;i<48;i++) {
    left[i][0] = drm_frame[432+i+fine_timeshift][0];
    left[i][1] = drm_frame[432+i+fine_timeshift][1];
    right[i][0] = drm_frame[i+fine_timeshift][0];
    right[i][1] = drm_frame[i+fine_timeshift][1];
  }
  for (i=0;i<48;i++){
    lr[i][0] = (left[i][0]*right[i][0]-left[i][1]*(-1)*right[i][1]);
    lr[i][1] = (left[i][0]*(-1)*right[i][1]+left[i][1]*right[i][0]);

    mean += atan2(lr[i][1],lr[i][0]);
    angle2[0] += lr[i][0];
    angle2[1] += lr[i][1];
  }

//  printf("\n%f %f\n",left[0][0],left[0][1]);
//  printf("\n%f %f\n",right[0][0],right[0][1]);
//  printf("\n%f %f\n",lr[0][0],lr[0][1]);
//  printf("\n%f (~%f)\n",angle[0], mean);

  ffe1 = mean / (48 * 2 * M_PI) * (192000.0/432);

  double theta_rad =  atan2(angle2[1],angle2[0]);
  ffe2 = theta_rad / (2 * M_PI) * (192000.0/432);
  //fprintf(stderr,"theta_rad=%.4f, ffs2=%.4f | ffs1=%.4f\n", theta_rad, ffe2, ffe1);

  return ffe2;
//  return ffe1;
}



//#define IFO_DEBUG

#ifdef IFO_DEBUG
gnuplot_ctrl *maxenergy_plot = NULL;
#endif


int drm_coarse_freq_sync_4symbols(drmplus_priv_t *p, double *energy)
{
	int i;
	int startFreq;
	double maxEnergy=0.0;
	int ret = 0;


	if(p->cfe_maxenergy_cnt==0) {
		memcpy(p->cfe_energyof_4, energy, sizeof(double)*432);
	} else {
		for(i=0;i<432;i++)
			p->cfe_energyof_4[i]=+energy[i];
	}
	p->cfe_maxenergy_cnt++;

	//It's time to calculate max. energy...
	if(p->cfe_maxenergy_cnt==4) {
		p->cfe_maxenergy_cnt=0;
		p->cfe_maxenergy_freq[0] = 0;

		for(startFreq=0;startFreq<(432-213 + 16);startFreq++) {
			double currentEnergy = 0.0;
			//13 or 14 pilots
			//fprintf(stderr, "e: ");
			for(i=0; i<54 && startFreq+i*4 < 432;i++) {
				currentEnergy += energy[startFreq+i*4];
				//fprintf(stderr, "[%d]:%f ", startFreq+i*16, energy[startFreq+i*16]);
			}

			if(currentEnergy > maxEnergy) {
				maxEnergy=currentEnergy;
				p->cfe_maxenergy_freq[0] = startFreq;
			}
		}

		p->cfe_freq_last = p->cfe_maxenergy_freq[0] - (432/2 - 106);
        if(p->siginfo.dc_freq_coarse + p->cfe_freq_last > -110 && p->siginfo.dc_freq_coarse + p->cfe_freq_last < (432/2-106)) {
			p->siginfo.dc_freq_coarse += p->cfe_freq_last;
			fprintf(stderr, "new IFO: %d (last:%d)\n", p->siginfo.dc_freq_coarse, p->cfe_freq_last);
			p->cfe_freq_ok=1;
			ret=1;
		}else{
			p->cfe_freq_ok=0;
		}
	}
	return ret;
}


int drm_coarse_freq_sync_maxenergy(drmplus_priv_t *p, double *energy)
{
	int ret = 0;
//	fprintf(stderr, "drm_coarse_freq_sync_3: last_dc: %d, checked: %d\n ", p->dc_freq_offset_last, p->dc_freq_offset_checked);

	double maxEnergy2[2] = {0.0, 0.0};
	int i;
	int startFreq;
	int freqMaxEnergy2[2] = {0, 0};
#ifdef IFO_DEBUG
	double energies[432];
#endif

	for(startFreq=0;startFreq<(432-213 + 16);startFreq++) {
		double currentEnergy = 0.0;
		//13 or 14 pilots
		//fprintf(stderr, "e: ");
		for(i=0; i<14 && startFreq+i*16 < 432;i++) {
			currentEnergy += energy[startFreq+i*16];
			//fprintf(stderr, "[%d]:%f ", startFreq+i*16, energy[startFreq+i*16]);
		}
		//fprintf(stderr, "\n");
#ifdef IFO_DEBUG
		energies[startFreq] = currentEnergy;
#endif
		//fprintf(stderr, "freq[%d]: e:%f\n", startFreq, currentEnergy);
		if(currentEnergy > maxEnergy2[0]) {
			maxEnergy2[1]=maxEnergy2[0];
			freqMaxEnergy2[1]=freqMaxEnergy2[0];
			maxEnergy2[0]=currentEnergy;
			freqMaxEnergy2[0] = startFreq;
		}
	}
//	fprintf(stderr, "MAX [%d]:%f | [%d]:%f\n", freqMaxEnergy2[0], maxEnergy2[0], freqMaxEnergy2[1], maxEnergy2[1]);

//	fprintf(stderr, "prev...now: %d == %d\n", freqMaxEnergy, p->dc_freq_offset_last);

	//fprintf(stderr, "prev...now: %d == %d\n", freqMaxEnergy, p->cfe_freq_pos[p->cfe_freq_cnt%4]);
	//if(freqMaxEnergy == p->cfe_freq_pos[p->cfe_freq_cnt%4]) {
	//	fprintf(stderr, "this is ok!\n");
	//}

	//find 2 peaks (symbol number 1,2).
	if(p->cfe_maxenergy_cnt < 4) {
		if(p->cfe_maxenergy_of4[0] < maxEnergy2[0] || p->cfe_maxenergy_cnt==0) {
			p->cfe_maxenergy_of4[1]=p->cfe_maxenergy_of4[0];
			p->cfe_maxenergy_freq[1]=p->cfe_maxenergy_freq[0];
			p->cfe_maxenergy_of4[0]=maxEnergy2[0];
			p->cfe_maxenergy_freq[0]=freqMaxEnergy2[0];

		}
//		fprintf(stderr, "MAX[%d]: [%d]:%f | [%d]:%f\n", p->cfe_maxenergy_cnt,
//				p->cfe_maxenergy_freq[0], p->cfe_maxenergy_of4[0], p->cfe_maxenergy_freq[1], p->cfe_maxenergy_of4[1]);

		p->cfe_maxenergy_cnt++;
		if(p->cfe_maxenergy_cnt==4) {

			if(p->cfe_maxenergy_freq[1] > 0 && p->cfe_maxenergy_freq[0] > p->cfe_maxenergy_freq[1]) {
				int tmp=p->cfe_maxenergy_freq[0];
				double tmp2=p->cfe_maxenergy_of4[0];
				p->cfe_maxenergy_freq[0]=p->cfe_maxenergy_freq[1];
				p->cfe_maxenergy_of4[0]=p->cfe_maxenergy_of4[1];
				p->cfe_maxenergy_freq[1]=tmp;
				p->cfe_maxenergy_of4[1]=tmp2;
			}

//			fprintf(stderr, "MAX of 4: [%d]:%f | [%d]:%f\n",
//					p->cfe_maxenergy_freq[0], p->cfe_maxenergy_of4[0], p->cfe_maxenergy_freq[1], p->cfe_maxenergy_of4[1]);

			//p->dc_freq_coarse = p->cfe_maxenergy_freq[0] - (432/2 - 106);
			//p->dc_freq_coarse = p->cfe_maxenergy_freq[0] - (432/2 - 106);
#if 0
			if(p->cfe_freq_last == p->cfe_maxenergy_freq[0] - (432/2 - 106)) {
				p->cfe_freq_ok++;
				if(p->cfe_freq_ok==2) {
					p->dc_freq_coarse += p->cfe_freq_last;
					fprintf(stderr, "new IFO: %d\n", p->dc_freq_coarse);
					if(p->dc_freq_coarse < -109)
						p->dc_freq_coarse=-109;
					if(p->dc_freq_coarse > 109)
						p->dc_freq_coarse=109;
					p->cfe_freq_ok=0;
				}
			} else {
				p->cfe_freq_last = p->cfe_maxenergy_freq[0] - (432/2 - 106);
				p->cfe_freq_ok=0;
			}
#else

#if 1
			if(p->cfe_freq_last != p->cfe_maxenergy_freq[0] - (432/2 - 106)) {
				p->cfe_freq_last = p->cfe_maxenergy_freq[0] - (432/2 - 106);
				p->cfe_freq_ok=0;
			} else {
				p->cfe_freq_ok++;
				if(p->cfe_freq_ok==2) {
					if(p->siginfo.dc_freq_coarse + p->cfe_freq_last >= -110 && p->siginfo.dc_freq_coarse + p->cfe_freq_last < (432-106)) {
						//p->siginfo.dc_freq_coarse += p->cfe_freq_last;
						p->siginfo.dc_freq_coarse = p->cfe_freq_last;
						fprintf(stderr, "new IFO: %d (last:%d)\n", p->siginfo.dc_freq_coarse, p->cfe_freq_last);
						ret = 1;
					}else{
						p->cfe_freq_ok=0;
					}
				}
			}
#endif

#endif
//			fprintf(stderr, "START FREQUENCY: maxenergy:%d | +dc:%d | IFO:%d\n", p->cfe_maxenergy_freq[0], p->cfe_maxenergy_freq[0] + p->dc_freq_coarse, p->dc_freq_coarse);
			p->cfe_maxenergy_cnt=0;
		}
//		return 0;
	}


#if 0
	//when symbol number == 0,3 we have 2 identical peaks. Use only second peak!
	if(freqMaxEnergy2[0]+4 < p->cfe_maxenergy_freq[0]) {
		maxEnergy2[0]=maxEnergy2[1];
		freqMaxEnergy2[0] = freqMaxEnergy2[1];
	}

	if(freqMaxEnergy2[0] < p->cfe_freq_last) {
		if(p->cfe_freq_min == freqMaxEnergy2[0] && freqMaxEnergy2[0] + 16 == p->cfe_freq_last) {
			//p->dc_freq_coarse = p->cfe_freq_min - 106;
			fprintf(stderr, "this is START!\n");
		} else
			fprintf(stderr, "BAD START: %d --- %d\n", freqMaxEnergy2[0], p->cfe_freq_min);
		p->cfe_freq_min = freqMaxEnergy2[0];
		//p->cfe_maxenergy_cnt=0;
	} else if (p->cfe_freq_last + 4 == freqMaxEnergy2[0]) {
		fprintf(stderr, "this is ok!\n");
	} else {
		fprintf(stderr, "resetted!\n");
		p->cfe_maxenergy_cnt=0;
	}
	p->cfe_freq_last = freqMaxEnergy2[0];
#endif

#ifdef IFO_DEBUG
    if(!maxenergy_plot) {
    	maxenergy_plot = gnuplot_init();
    	//gnuplot_setstyle(maxenergy_plot, "lines") ;
    } else {
    	gnuplot_resetplot(maxenergy_plot) ;
    }
    gnuplot_plot_x(maxenergy_plot, energies, (432-213 + 16), "Energies");
    usleep(500000);
#endif
	return ret;
}




//#define CTE_DEBUG

#ifdef CTE_DEBUG
gnuplot_ctrl *cte_plot = NULL;
#endif



#if 0
int drm_coarse_time_sync_0(drmplus_priv_t *p, fftw_complex *symbols)
{

#ifdef CTE_DEBUG
	double energies[213];
	double corr0[14];
	double corr1[14];
#endif

	fftw_complex convoluted_prs[14];
	double correlates[40];

	int s,i;

#ifdef CTE_DEBUG
    if(!cte_plot) {
    	cte_plot = gnuplot_init();
    	//gnuplot_setstyle(maxenergy_plot, "lines") ;
    } else {
    	gnuplot_resetplot(cte_plot) ;
    }
#endif


	for(s=0;s<40;s++) {
		double correlation = 0.0;
		for(i=0;i<14;i++) {
			int freqIdx = (s*4+12) % 16 + 16*i;
			if(freqIdx > 213) {
				convoluted_prs[i][0] = 0.0;
				convoluted_prs[i][1] = 0.0;
				break;
			}

			double amplitudeVal=M_SQRT2;
			if(freqIdx == 0 || freqIdx == 3 || freqIdx == 212 || freqIdx == 208)
				amplitudeVal=2;
			fftw_complex val;
			val[0] = amplitudeVal * cos((double) 2.0 * M_PI * (double)phases[s][i] / 1024.0);
			val[1] = amplitudeVal * sin((double) 2.0 * M_PI * (double)phases[s][i] / 1024.0);

	        convoluted_prs[i][0] = val[0]*symbols[freqIdx][0]  -  (-1)*val[1]*symbols[freqIdx][1];
	        convoluted_prs[i][1] = val[0]*symbols[freqIdx][1]  +  (-1)*val[1]*symbols[freqIdx][0];
		}


	    fftw_complex convoluted_prs_time[14];
	    fftw_plan px;
	    px = fftw_plan_dft_1d(14, convoluted_prs, convoluted_prs_time, FFTW_BACKWARD, FFTW_ESTIMATE);
	    fftw_execute(px);
	    fftw_destroy_plan(px);

	    uint32_t maxPos=0;
	    double tempVal = 0;
	    double maxVal=-99999;
	    for (i=0;i<14;i++) {
	      tempVal = sqrt((convoluted_prs_time[i][0]*convoluted_prs_time[i][0])+(convoluted_prs_time[i][1]*convoluted_prs_time[i][1]));
	      corr0[i] = tempVal;
	      if (tempVal>maxVal) {
			maxPos = i;
			maxVal = tempVal;
	      }
	    }
#ifdef CTE_DEBUG1
		for(i=0;i<14;i++) {
			corr0[i] = convoluted_prs[i][0];
			corr1[i] = convoluted_prs[i][1];
		}
	        gnuplot_plot_xy(cte_plot, corr0, corr1, 14, "SYNC");
#endif
#ifdef CTE_DEBUG3
	        gnuplot_plot_x(cte_plot, corr0, 14, "MAX");
#endif
		correlates[s] = maxVal;
	}

#ifdef CTE_DEBUG
	        gnuplot_plot_x(cte_plot, correlates, 40, "MAX");
#endif


#ifdef CTE_DEBUG
    usleep(1500000);
#endif

	return 0;
}
#endif


#if 0
int sym_idx = 0;
double sum_xxx[40];

int drm_coarse_time_sync_debug(drmplus_priv_t *p, fftw_complex *symbols)
{

#ifdef CTE_DEBUG
	double energies[213];
	double corr0[14];
	double corr1[14];
#endif

	fftw_complex convoluted_prs[14];
	double correlates[40];

	int s,i;

#ifdef CTE_DEBUG
    if(!cte_plot) {
    	cte_plot = gnuplot_init();
    	//gnuplot_setstyle(cte_plot, "lines") ;
    } else {
    	if(sym_idx == 40) {
    		gnuplot_plot_x(cte_plot, sum_xxx, 40, "is NULL");
    		usleep(1500000);
    		gnuplot_resetplot(cte_plot);
    		sym_idx=0;
    	}
    }
#endif
    fftw_complex sum[8];
    char sumlbl[32];
    double total_sum = 0.0;
    double total_sum2 = 0.0;
    double total_sum3 = 0.0;

    /*
start at 298 | DC: cfe:108*444.4 + -0.3 + 0.1 => 47999.8
[1]: -9.71|-7.90  + 8.18|-9.39  + -8.66|9.23 == -28.33|-24.74
[2]: 8.74|9.04  + 8.52|9.21  + -8.47|-9.29 == -26.56|-26.73
[3]: -8.25|9.60  + -8.44|9.40 ==  -16.69|-19.00
[4]: -10.19|-7.52  + -10.01|-7.72  + -7.95|9.73 ==
[5]: -8.72|-9.14  + 8.43|9.21  + -8.32|-9.52 ==
[6]: 12.53|0.78  + 12.50|1.06  + 12.46|1.32 == 26.35|14.30
[7]: -4.96|-11.60  + 11.67|-4.82 ==
[8]: -11.74|4.24  + -11.82|3.98 ==
     *
     * */


    //[1]: -80, -79, -77, => 26, 27, 29
    //[1]: -9.71|-7.90  + 8.18|-9.39  + -8.66|9.23 == -28.33|-24.74
    sum[0][0] = symbols[26][0] + symbols[27][1] - symbols[29][1];
    sum[0][1] = symbols[26][1] - symbols[27][0] + symbols[29][0];
    total_sum += sqrt(sum[0][0]*sum[0][0] + sum[0][1]*sum[0][1]);
    total_sum2 += sum[0][0]*sum[0][0] + sum[0][1]*sum[0][1];
    total_sum3 += abs(sum[0][0]) + abs(sum[0][1]);

    fprintf(stderr, "[1]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[26][0],symbols[26][1], symbols[27][0],symbols[27][1], symbols[29][0],symbols[29][1],  sum[0][0], sum[0][1]);

    //[2]: -53, -52, -51, => 53, 54, 55
    //[2]: 8.74|9.04  + 8.52|9.21  + -8.47|-9.29 == -26.56|-26.73
    sum[1][0] = symbols[53][0] + symbols[54][0] - symbols[55][0];
    sum[1][1] = symbols[53][1] + symbols[54][1] - symbols[55][1];
    total_sum += sqrt(sum[1][0]*sum[1][0] + sum[1][1]*sum[1][1]);
    total_sum2 += sum[1][0]*sum[1][0] + sum[1][1]*sum[1][1];
    total_sum3 += abs(sum[1][0]) + abs(sum[1][1]);
    fprintf(stderr, "[2]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[53][0],symbols[53][1], symbols[54][0],symbols[54][1], symbols[55][0],symbols[55][1], sum[1][0], sum[1][1]);


    //[3]: -32, -31,      => 74, 75
    //[3]: -8.25|9.60  + -8.44|9.40 ==  -16.69|-19.00
    sum[2][0] = symbols[74][0] + symbols[75][0];
    sum[2][1] = symbols[74][1] + symbols[75][1];
    total_sum += sqrt(sum[2][0]*sum[2][0] + sum[2][1]*sum[2][1]);
    total_sum2 += sum[2][0]*sum[2][0] + sum[2][1]*sum[2][1];
    total_sum3 += abs(sum[2][0]) + abs(sum[2][1]);
    fprintf(stderr, "[3]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[74][0],symbols[74][1], symbols[75][0],symbols[75][1], sum[2][0], sum[2][1]);


    //[4]: 12, 13, 14,    => 118, 119, 120
    //[4]: -10.19|-7.52  + -10.01|-7.72  + -7.95|9.73 ==
    sum[3][0] = symbols[118][0] + symbols[119][0] - symbols[120][1];
    sum[3][1] = symbols[118][1] + symbols[119][1] + symbols[120][0];
    total_sum += sqrt(sum[3][0]*sum[3][0] + sum[3][1]*sum[3][1]);
    total_sum2 += sum[3][0]*sum[3][0] + sum[3][1]*sum[3][1];
    total_sum3 += abs(sum[3][0]) + abs(sum[3][1]);
    fprintf(stderr, "[4]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[118][0],symbols[118][1], symbols[119][0],symbols[119][1], symbols[120][0],symbols[120][1], sum[3][0], sum[3][1]);

    //[5]: 21, 22, 23,    => 127, 128, 129
    //[5]: -8.72|-9.14  + 8.43|9.21  + -8.32|-9.52 ==
    sum[4][0] = symbols[127][0] - symbols[128][0] + symbols[129][0];
    sum[4][1] = symbols[127][1] - symbols[128][1] + symbols[129][1];
    total_sum += sqrt(sum[4][0]*sum[4][0] + sum[4][1]*sum[4][1]);
    total_sum2 += sum[4][0]*sum[4][0] + sum[4][1]*sum[4][1];
    total_sum3 += abs(sum[4][0]) + abs(sum[4][1]);
    fprintf(stderr, "[5]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[127][0],symbols[127][1], symbols[128][0],symbols[128][1], symbols[129][0],symbols[129][1], sum[4][0], sum[4][1]);
    //[6]: 40, 41, 42,    => 146, 147, 148
    //[6]: 12.53|0.78  + 12.50|1.06  + 12.46|1.32 == 26.35|14.30
    sum[5][0] = symbols[146][0] + symbols[147][0] + symbols[148][0];
    sum[5][1] = symbols[146][1] + symbols[147][1] + symbols[148][1];
    total_sum += sqrt(sum[5][0]*sum[5][0] + sum[5][1]*sum[5][1]);
    total_sum2 += sum[5][0]*sum[5][0] + sum[5][1]*sum[5][1];
    total_sum3 += abs(sum[5][0]) + abs(sum[5][1]);
    fprintf(stderr, "[6]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[146][0],symbols[146][1], symbols[147][0],symbols[147][1], symbols[148][0],symbols[148][1], sum[5][0], sum[5][1]);

    //[7]: 67, 68,        => 173, 174
    //[7]: -4.96|-11.60  + 11.67|-4.82 ==
    sum[6][0] = symbols[173][0] + symbols[174][1];
    sum[6][1] = symbols[173][1] - symbols[174][0];
    total_sum += sqrt(sum[6][0]*sum[6][0] + sum[6][1]*sum[6][1]);
    total_sum2 += sum[6][0]*sum[6][0] + sum[6][1]*sum[6][1];
    total_sum3 += abs(sum[5][0]) + abs(sum[5][1]);
    fprintf(stderr, "[7]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[173][0],symbols[173][1], symbols[174][0],symbols[174][1], sum[6][0], sum[6][1]);

    //[8]: 79, 80         => 185, 186
    sum[7][0] = symbols[185][0] + symbols[186][0];
    sum[7][1] = symbols[185][1] - symbols[186][1];
    total_sum += sqrt(sum[7][0]*sum[7][0] + sum[7][1]*sum[7][1]);
    total_sum2 += sum[7][0]*sum[7][0] + sum[7][1]*sum[7][1];
    total_sum3 += abs(sum[7][0]) + abs(sum[7][1]);
    fprintf(stderr, "[8]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[185][0],symbols[185][1], symbols[186][0],symbols[186][1], sum[7][0], sum[7][1]);


    fprintf(stderr, "TOTAL: %.2f | %.2f | %.2f \n",total_sum,total_sum2, total_sum3);

    sum_xxx[sym_idx] = total_sum2;

#ifdef CTE_DEBUG1
    sprintf(sumlbl, "sym%d", sym_idx);

    double sum1[8];
	double sum2[8];

	for(i=0; i<9; i++) {
		sum1[i] = sum[i][0];
		sum2[i] = sum[i][1];
	}
	gnuplot_plot_xy(cte_plot, sum1, sum2, 8, sumlbl);
#endif
    sym_idx++;
	return 0;
}
#endif

/* Coarse time sync done by calculating groups of time reference cells and compare they phases */
int drm_coarse_time_sync(drmplus_priv_t *p, fftw_complex *symbols, int *is_inverted)
{
    fftw_complex sum[8];

    sum[0][0] = symbols[26][0] + symbols[27][1] - symbols[29][1];
    sum[0][1] = symbols[26][1] - symbols[27][0] + symbols[29][0];

    sum[1][0] = symbols[53][0] + symbols[54][0] - symbols[55][0];
    sum[1][1] = symbols[53][1] + symbols[54][1] - symbols[55][1];

    sum[2][0] = symbols[74][0] + symbols[75][0];
    sum[2][1] = symbols[74][1] + symbols[75][1];

    sum[3][0] = symbols[118][0] + symbols[119][0] - symbols[120][1];
    sum[3][1] = symbols[118][1] + symbols[119][1] + symbols[120][0];

    sum[4][0] = symbols[127][0] - symbols[128][0] + symbols[129][0];
    sum[4][1] = symbols[127][1] - symbols[128][1] + symbols[129][1];

    sum[5][0] = symbols[146][0] + symbols[147][0] + symbols[148][0];
    sum[5][1] = symbols[146][1] + symbols[147][1] + symbols[148][1];

    sum[6][0] = symbols[173][0] + symbols[174][1];
    sum[6][1] = symbols[173][1] - symbols[174][0];

    sum[7][0] = symbols[185][0] + symbols[186][0];
    sum[7][1] = symbols[185][1] - symbols[186][1];

    /* sample:
[1]: -9.71|-7.90  + 8.18|-9.39  + -8.66|9.23 == -28.33|-24.74
[2]: 8.74|9.04  + 8.52|9.21  + -8.47|-9.29 == -26.56|-26.73
[3]: -8.25|9.60  + -8.44|9.40 ==  -16.69|-19.00
[4]: -10.19|-7.52  + -10.01|-7.72  + -7.95|9.73 ==
[5]: -8.72|-9.14  + 8.43|9.21  + -8.32|-9.52 ==
[6]: 12.53|0.78  + 12.50|1.06  + 12.46|1.32 == 26.35|14.30
[7]: -4.96|-11.60  + 11.67|-4.82 ==
[8]: -11.74|4.24  + -11.82|3.98 ==
     * */
#if 0
    fprintf(stderr, "[1]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[26][0],symbols[26][1], symbols[27][0],symbols[27][1], symbols[29][0],symbols[29][1],  sum[0], sum[1]);
    fprintf(stderr, "[2]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[53][0],symbols[53][1], symbols[54][0],symbols[54][1], symbols[55][0],symbols[55][1], sum[0], sum[1]);
    fprintf(stderr, "[3]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[74][0],symbols[74][1], symbols[75][0],symbols[75][1], sum[0], sum[1]);
    fprintf(stderr, "[4]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[118][0],symbols[118][1], symbols[119][0],symbols[119][1], symbols[120][0],symbols[120][1], sum[0], sum[1]);
    fprintf(stderr, "[5]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[127][0],symbols[127][1], symbols[128][0],symbols[128][1], symbols[129][0],symbols[129][1], sum[0], sum[1]);
    fprintf(stderr, "[6]: %.2f|%.2f  + %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[146][0],symbols[146][1], symbols[147][0],symbols[147][1], symbols[148][0],symbols[148][1], sum[0], sum[1]);
    fprintf(stderr, "[7]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[173][0],symbols[173][1], symbols[174][0],symbols[174][1], sum[0], sum[1]);
    fprintf(stderr, "[8]: %.2f|%.2f  + %.2f|%.2f == %.2f|%.2f\n",
    		symbols[185][0],symbols[185][1], symbols[186][0],symbols[186][1], sum[0], sum[1]);
#endif

    //calc magnitude squared of added vectors
    p->time_sync_history[p->symbol_num % 40] = \
    		sum[0][0]*sum[0][0] + sum[0][1]*sum[0][1] +
    		sum[1][0]*sum[1][0] + sum[1][1]*sum[1][1] +
			sum[2][0]*sum[2][0] + sum[2][1]*sum[2][1] +
			sum[3][0]*sum[3][0] + sum[3][1]*sum[3][1] +
			sum[4][0]*sum[4][0] + sum[4][1]*sum[4][1] +
			sum[5][0]*sum[5][0] + sum[5][1]*sum[5][1] +
			sum[6][0]*sum[6][0] + sum[6][1]*sum[6][1] +
			sum[7][0]*sum[7][0] + sum[7][1]*sum[7][1];



    sum[0][0] = symbols[213-1-26][0] + symbols[213-1-27][1] - symbols[213-1-29][1];
    sum[0][1] = symbols[213-1-26][1] - symbols[213-1-27][0] + symbols[213-1-29][0];

    sum[1][0] = symbols[213-1-53][0] + symbols[213-1-54][0] - symbols[213-1-55][0];
    sum[1][1] = symbols[213-1-53][1] + symbols[213-1-54][1] - symbols[213-1-55][1];

    sum[2][0] = symbols[213-1-74][0] + symbols[213-1-75][0];
    sum[2][1] = symbols[213-1-74][1] + symbols[213-1-75][1];

    sum[3][0] = symbols[213-1-118][0] + symbols[213-1-119][0] - symbols[213-1-120][1];
    sum[3][1] = symbols[213-1-118][1] + symbols[213-1-119][1] + symbols[213-1-120][0];

    sum[4][0] = symbols[213-1-127][0] - symbols[213-1-128][0] + symbols[213-1-129][0];
    sum[4][1] = symbols[213-1-127][1] - symbols[213-1-128][1] + symbols[213-1-129][1];

    sum[5][0] = symbols[213-1-146][0] + symbols[213-1-147][0] + symbols[213-1-148][0];
    sum[5][1] = symbols[213-1-146][1] + symbols[213-1-147][1] + symbols[213-1-148][1];

    sum[6][0] = symbols[213-1-173][0] + symbols[213-1-174][1];
    sum[6][1] = symbols[213-1-173][1] - symbols[213-1-174][0];

    sum[7][0] = symbols[213-1-185][0] + symbols[213-1-186][0];
    sum[7][1] = symbols[213-1-185][1] - symbols[213-1-186][1];

    //calc magnitude squared of added vectors for inverted part
    p->time_sync_history_inv[p->symbol_num % 40] = \
    		sum[0][0]*sum[0][0] + sum[0][1]*sum[0][1] +
    		sum[1][0]*sum[1][0] + sum[1][1]*sum[1][1] +
			sum[2][0]*sum[2][0] + sum[2][1]*sum[2][1] +
			sum[3][0]*sum[3][0] + sum[3][1]*sum[3][1] +
			sum[4][0]*sum[4][0] + sum[4][1]*sum[4][1] +
			sum[5][0]*sum[5][0] + sum[5][1]*sum[5][1] +
			sum[6][0]*sum[6][0] + sum[6][1]*sum[6][1] +
			sum[7][0]*sum[7][0] + sum[7][1]*sum[7][1];

    if(p->symbol_num > 39) {
    	int curr_idx = p->symbol_num % 40;
    	int i, max_idx=0;
    	double max = 0.0;
    	for(i=0;i<40;i++) {
    		if(p->time_sync_history[(curr_idx + i) % 40] > max) {
    			max=p->time_sync_history[(curr_idx + i) % 40];
    			max_idx=i;
    			*is_inverted=0;
    		}
    		if(p->time_sync_history_inv[(curr_idx + i) % 40] > max) {
    			max=p->time_sync_history_inv[(curr_idx + i) % 40];
    			max_idx=i;
    			*is_inverted=1;

    		}
    	}
//    	fprintf(stderr, "MAG_SQ[%d]: %.2f / %.2f %s\n", curr_idx, p->time_sync_history[curr_idx], max, max_idx==0 ? "[START]" : "");

    	return max_idx == 0 ? 0 : 40 - max_idx;

    }

	return -1;
}







#if 0

void drm_create_phaseref_simple_tables(int8_t pilots[40][14], int16_t pilot_phases[40][14])
{
	int Kin, k, n, s, m, p;
	double a, f;

	memset(pilots, 127, 40*14);
	memset(pilot_phases, 232, 40*14*sizeof(int16_t));
//    gnuplot_ctrl *h1 = gnuplot_init() ;

//	printf("ts_pil[40][213] = {\n");
	for(s = 0; s < 40; s++) {
		int pil_idx = 0;

		for (Kin = -106; Kin <= 106; Kin++) {
		//for (Kin = 0; Kin < 213; Kin++) {
			n = s % 4;
			m = s / 4;
			p = (Kin - 2 - n*4) / (4*4);

			k = 2 + 4 * (s%4) + 16 * p;

			if(Kin == k) {
				int phaseVal = ((p*p) * R1024[n][m] + p*Z1024[n][m] + Q1024[n][m]) % 1024;
				//if(phaseVal < 0) phaseVal += 1024;
				//F1024[s][Kin+106] = phaseVal % 1024;
				double amplitudeVal = M_SQRT2;
				if(k == -106 || k == -102 || k == 102 || k == 106) {
					amplitudeVal = 2;
				}

				//PILOTS[s][Kin+106][0] = amplitudeVal * cos((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
				//PILOTS[s][Kin+106][1] = amplitudeVal * sin((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
				pilots[s][pil_idx] = Kin;
				pilot_phases[s][pil_idx] = phaseVal;
				pil_idx++;
			}
		}
	}
}






void drm_create_phaseref_tables(fftw_complex PILOTS[40][213], fftw_complex TIMESYNC_PILOTS[213])
{
	int Kin, k, n, s, m, p;
	double a, f;

	memset(PILOTS, 0x00, sizeof(fftw_complex)*40*213);
//    gnuplot_ctrl *h1 = gnuplot_init() ;

//	printf("ts_pil[40][213] = {\n");
	int ts_pil_idx = 0;
	for(s = 0; s < 40; s++) {
		//printf("[%3d]:", s);
//		printf(" { ");
		//int last_k = -200;

//		double re[213];
//		double im[213];
//		int reim_idx = 0;

		for (Kin = -106; Kin <= 106; Kin++) {
		//for (Kin = 0; Kin < 213; Kin++) {
			n = s % 4;
			m = s / 4;
			p = (Kin - 2 - n*4) / (4*4);

			k = 2 + 4 * (s%4) + 16 * p;

			PILOTS[s][Kin+106][0] = 0.0;
			PILOTS[s][Kin+106][1] = 0.0;
#if 0
			if(Kin == k) {
				if(k == -106 || k == -102 || k == 102 || k == 106) {
					//a = 2.0;
					printf("O");
				} else {
					printf("o");
				}
				//A1024[s][Kin+106] = a;
				//int phaseVal = ((p*p) * R1024[n][m] + abs(p)*Z1024[n][m] + Q1024[n][m]);
				//F1024[s][Kin+106] = phaseVal % 1024;
			} else {
				printf(".");
			}
#endif

#if 1
			if(Kin == k) {
				int phaseVal = ((p*p) * R1024[n][m] + p*Z1024[n][m] + Q1024[n][m]) % 1024;
				//if(phaseVal < 0) phaseVal += 1024;
				//F1024[s][Kin+106] = phaseVal % 1024;
				double amplitudeVal = M_SQRT2;
				if(k == -106 || k == -102 || k == 102 || k == 106) {
					amplitudeVal = 2;
				}
				PILOTS[s][Kin+106][0] = amplitudeVal * cos((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
				PILOTS[s][Kin+106][1] = amplitudeVal * sin((double) 2.0 * M_PI * (double)phaseVal / 1024.0);
				//printf("%5d", phaseVal);

				if(s==0) {
					TIMESYNC_PILOTS[Kin+106][0] = PILOTS[s][Kin+106][0];
					TIMESYNC_PILOTS[Kin+106][1] = PILOTS[s][Kin+106][1];
				}
#if 0
				if(s==4) {
					AFS4_PILOTS[Kin+106][0] = PILOTS[s][Kin+106][0];
					AFS4_PILOTS[Kin+106][1] = PILOTS[s][Kin+106][1];
				}
#endif
//				printf("{%1.2f, %1.2f}, ", PILOTS[s][Kin+106][0], PILOTS[s][Kin+106][1]);
//				re[reim_idx] =  PILOTS[s][Kin+106][0];
//				im[reim_idx] =  PILOTS[s][Kin+106][1];
//				reim_idx++;
			}
#endif
			if(s==0 && Kin == timesync_k[ts_pil_idx]) {
				TIMESYNC_PILOTS[Kin+106][0] = M_SQRT2 * cos((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				TIMESYNC_PILOTS[Kin+106][1] = M_SQRT2 * sin((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				//TIMESYNC_PILOTS[Kin+106] = Polar2Cart(M_SQRT2, timesync_phi[ts_pil_idx]);
				//TIMESYNC_PILOTS2[ts_pil_idx] = Polar2Cart(SQRT2, timesync_phi[ts_pil_idx]);
				ts_pil_idx++;
			}
#if 0
			if(s==4 && Kin == timesync_k[ts_pil_idx]) {
				TIMESYNC_PILOTS[Kin+106][0] = M_SQRT2 * cos((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				TIMESYNC_PILOTS[Kin+106][1] = M_SQRT2 * sin((double) 2.0 * M_PI * (double)timesync_phi[ts_pil_idx] / 1024.0);
				//TIMESYNC_PILOTS[Kin+106] = Polar2Cart(M_SQRT2, timesync_phi[ts_pil_idx]);
				//TIMESYNC_PILOTS2[ts_pil_idx] = Polar2Cart(SQRT2, timesync_phi[ts_pil_idx]);
				ts_pil_idx++;
			}
#endif
		}
//		printf("}, \n");

//	    gnuplot_plot_xy(h1, re, im, reim_idx, "I/Q");
//		sleep(1);
		//gnuplot_resetplot(h1);
	}
	//gnuplot_close(h1);
#if 0
	printf("TIMESYNC_PILOTS[432] = {\n");
	for (Kin = -106; Kin <= 106; Kin++) {
		printf("{%.18e,%.18e},\n", TIMESYNC_PILOTS[Kin+106][0], TIMESYNC_PILOTS[Kin+106][1]);
	}
	printf("}\n");
#endif
}

#if 0
uint32_t drm_coarse_time_sync(fftw_complex * symbols, double *global_max) {
	  fftw_complex convoluted_prs[213];
	  int s,p;
	  *global_max = -99999;
	  int global_max_pos=0;

	  double correlates[40];

	  for(p=0;p<40;p++) {
		double tempVal = 0;
	    double energy[213];
	    for (s=0;s<213;s++) {
	    	convoluted_prs[s][0] = PILOTS[p][s][0]*symbols[s][0]  -  (-1)*PILOTS[p][s][1]*symbols[s][1];
	    	convoluted_prs[s][1] = PILOTS[p][s][0]*symbols[s][1]  +  (-1)*PILOTS[p][s][1]*symbols[s][0];
	    	energy[s] = sqrt(fabs(convoluted_prs[s][0]*convoluted_prs[s][0])+fabs(convoluted_prs[s][1]*convoluted_prs[s][1]));
		    tempVal += energy[s];
	    }


#if 1
		if(!h3) {
			h3 = gnuplot_init();
		} else {
			//gnuplot_resetplot(h2) ;
		}
		char str[8] = {0};
		sprintf(str, "c:%d", p);
		gnuplot_plot_x(h3, energy, 213, str);
#endif
	    //fprintf(stderr,"%f ",maxVal);
	    correlates[p] = tempVal;

	    if (tempVal > *global_max) {
	      *global_max = tempVal;
	      global_max_pos = p;
	    }
	  }

      fprintf(stderr,"MAXPOS2[%d] val: %f of: ", global_max_pos, *global_max);
      for(p=0;p<40;p++) fprintf(stderr,"%.2f, ", correlates[p]);
      fprintf(stderr,"\n");
	  sleep(6);
	  gnuplot_resetplot(h3);

//#ifdef COARSE_GRAPH_DISPLAY3
#if 0
	  if(!h2) {
	  	h2 = gnuplot_init();
	  } else {
	  	//gnuplot_resetplot(h2) ;
	  }
	  char str[8] = {0};
	  sprintf(str, "c:%d", global_max_pos);
	  gnuplot_plot_x(h2, correlates, 40, str);
	  usleep(00000);

	  fprintf(stderr,"MAXPOS2[%d] val: %f\n", global_max_pos, *global_max);
	//	  sleep(100);
#endif
	  return global_max_pos;
}
#endif

#endif
