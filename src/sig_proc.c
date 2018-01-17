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

//#include "gnuplot_i.h"

double mag_squared(fftw_complex sample) {
    double x = sample[0];
    double y = sample[1];
    return x * x + y *y;
}

void drm_get_linear_phase_regr(fftw_complex * symbols, uint8_t symbol_id, double *a, double *b)
{
	double sum_x=0, sum_y=0, sum_x2=0, sum_xy=0, D;
	int m_pil_idx=0;

	double prev_value = 0;

	int i = ((symbol_id+3)%4)*4; //< first pilot position
	for (; i< 213; i+=16)
	{
			double val1 = atan2(symbols[i][1],symbols[i][0]);
			double val2 = 2.0 * M_PI * phases[symbol_id][m_pil_idx] / 1024.0;

			double error = val1 - val2;
			/* this is due phase error can be larger than 2*PI from first to last carrier
			 * TODO: Maybe it's not needed in real life */
			while (fabs(error - prev_value) > M_PI) {
				if(error < prev_value) error+=2*M_PI;
				else error-=2*M_PI;
			}
			prev_value = error;

			sum_x+=i;
			sum_x2+=i*i;

			sum_y+=error;
			sum_xy+=i*error;
			m_pil_idx++;
	}

	D=m_pil_idx*sum_x2-sum_x*sum_x;
	*b=(sum_x2*sum_y-sum_x*sum_xy)/D;
	*a=(m_pil_idx*sum_xy-sum_x*sum_y)/D;
	//fprintf(stderr, "linear phase error: %.2f\n", sum_y/sum_x);
}

void drm_normalize_correct_phase(fftw_complex * symbols, uint8_t symbol_id)
{
	int i;
	double a, b;
	//get a/b factor
	drm_get_linear_phase_regr(symbols, symbol_id, &a, &b);
	//fprintf(stderr, "a=%.2f, b=%.2f, symbol_id=%d\n", a, b, symbol_id);

	/* normalize correct phases */
	for (i=0; i<213; i++)
	{
			// correct linear phase distortion:
			double cRval = -1*(i*a + b);
			fftw_complex shift_signal = {cos(cRval), sin(cRval)};
			fftw_complex tmp = {symbols[i][0],  symbols[i][1]};
			symbols[i][0] = shift_signal[0]*tmp[0] - shift_signal[1]*tmp[1];
			symbols[i][1] = shift_signal[0]*tmp[1] + shift_signal[1]*tmp[0];
	}
}


void drm_equalize_feed_pilots_simple(fftw_complex * symbols, uint8_t symbol_id, fftw_complex *H_pilots, int is_initial_eq)
{
	int carrier, arrayCarrierIndex;
	int symbolsWithRepeat, firstCarrierIndex, carriersLeftofPilot;


	fftw_plan p;
	fftw_complex *H_pilots_fftw, *h_fftw, *h_padded_fftw, *H_interpolated;
	int totalFreqs, halfZeros;

	fftw_complex pil;


	int i = ((symbol_id+3)%4)*4; //< first pilot position
	int pil_id=0;
	for (; i< 213; i+=16)
	{
		double pil_div=M_SQRT1_2;
		if(i==0||i==4||i==208||i==212)
			pil_div = 1;

		pil[0] = cos((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);
		pil[1] = sin((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);

		fftw_complex pil_diff;
		pil_diff[0] = (symbols[i][0] * pil[0] + symbols[i][1] * pil[1]) / pil_div;
		pil_diff[1] = (symbols[i][1] * pil[0] - symbols[i][0] * pil[1]) / pil_div;

		H_pilots[i][0] = pil_diff[0];
		H_pilots[i][1] = pil_diff[1];

		//initial equalizer creation.
		//due time references can be used too, there is higher amount of pilots to use there.
		//can be enchanced by AFC pilots as well.

		int j;
		j=i-1;
		// -2 carriers <---- this pilot
		while(j >=0 && i-j <= 2) {
			H_pilots[j][0] = H_pilots[i][0];
			H_pilots[j][1] = H_pilots[i][1];
			j--;
		}

		j=i+1;
		// this pilot ---> +2 carriers
		while(j < 213 && j-i <= 2) {
			H_pilots[j][0] = H_pilots[i][0];
			H_pilots[j][1] = H_pilots[i][1];
			j++;
		}
		pil_id++;
	}
}


void drm_equalize_feed_pilots_lin_int(fftw_complex * symbols, uint8_t symbol_id, fftw_complex *H_pilots, int is_initial_eq, int use_mem)
{
	int carrier, arrayCarrierIndex;
	int symbolsWithRepeat, firstCarrierIndex, carriersLeftofPilot;


	fftw_plan p;
	fftw_complex *H_pilots_fftw, *h_fftw, *h_padded_fftw, *H_interpolated;
	int totalFreqs, halfZeros;

	fftw_complex pil;


	int i = ((symbol_id+3)%4)*4; //< first pilot position
	int pil_id=0;
	for (; i< 213; i+=16)
	{
		double pil_div=M_SQRT1_2;
		if(i==0||i==4||i==208||i==212)
			pil_div = 1;

		pil[0] = cos((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);
		pil[1] = sin((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);

		fftw_complex pil_diff;
		pil_diff[0] = (symbols[i][0] * pil[0] + symbols[i][1] * pil[1]) / pil_div;
		pil_diff[1] = (symbols[i][1] * pil[0] - symbols[i][0] * pil[1]) / pil_div;

		if(!use_mem || is_initial_eq) {
			H_pilots[i][0] = pil_diff[0];
			H_pilots[i][1] = pil_diff[1];
		} else {
			H_pilots[i][0] = (H_pilots[i][0] + pil_diff[0])/2;
			H_pilots[i][1] = (H_pilots[i][1] + pil_diff[1])/2;
		}

		//initial equalizer creation.
		//due time references can be used too, there is higher amount of pilots to use there.
		//can be enchanced by AFC pilots as well.
		if(is_initial_eq) {
			int j;
			j=i-1;
			while(j >=0 && (pil_id==0 || i-j <= 8)) {
			//while(j >=0 && i-j <= 2) {
				H_pilots[j][0] = H_pilots[i][0];
				H_pilots[j][1] = H_pilots[i][1];
				j--;
			}

			j=i+1;
			while(j < 213 && ((pil_id==13 || (pil_id==12 && phases[symbol_id][pil_id+1] == 32767)) || j-i <= 8)) {
			//while(j < 213 && j-i <= 2) {
				H_pilots[j][0] = H_pilots[i][0];
				H_pilots[j][1] = H_pilots[i][1];
				j++;
			}
		} else {
			//do linearization
			if(i>0) {
				int cmp_num = i>=16 ? 16 : i;
				//int cmp_num = i>=4 ? 4 : i;

				pil_diff[0] = (H_pilots[i][0] - H_pilots[i-cmp_num][0])/cmp_num;
				pil_diff[1] = (H_pilots[i][1] - H_pilots[i-cmp_num][1])/cmp_num;

				int j;
				for(j=0;j<cmp_num;j++) {
					if(!use_mem) {
						H_pilots[i-j][0] = (q31_t)H_pilots[i][0] - j*pil_step[0];
						H_pilots[i-j][1] = (q31_t)H_pilots[i][1] - j*pil_step[1];
					} else {
						H_pilots[i-j][0] = ((q31_t)H_pilots[i-j][0] + H_pilots[i][0] - j*pil_step[0])/2;
						H_pilots[i-j][1] = ((q31_t)H_pilots[i-j][1] + H_pilots[i][1] - j*pil_step[1])/2;
					}
				}

				if(pil_id==13 || (pil_id==12 && phases[symbol_id][pil_id+1] == 32767)) {
					//enchance tail...
					cmp_num = Ncarr - i;
					int j;
					for(j=0;j<cmp_num;j++) {
						if(!use_mem) {
							H_pilots[i+j][0] = (q31_t)H_pilots[i][0] + j*pil_step[0];
							H_pilots[i+j][1] = (q31_t)H_pilots[i][1] + j*pil_step[1];
						} else {
							H_pilots[i+j][0] = ((q31_t)H_pilots[i+j][0] + H_pilots[i][0] + j*pil_step[0])/2;
						H_pilots[i+j][1] = ((q31_t)H_pilots[i+j][1] + H_pilots[i][1] + j*pil_step[1])/2;
						}
					}
				}

			}

		}
		pil_id++;
	}
}




void drm_equalize_feed_pilots_2222(fftw_complex * symbols, uint8_t symbol_id, fftw_complex *H_pilots, int is_initial_eq)
{
	int carrier, arrayCarrierIndex;
	int symbolsWithRepeat, firstCarrierIndex, carriersLeftofPilot;


	fftw_plan p;
	fftw_complex *H_pilots_fftw, *h_fftw, *h_padded_fftw, *H_interpolated;
	int totalFreqs, halfZeros;

	fftw_complex pil;


	int i = ((symbol_id+3)%4)*4; //< first pilot position
	int pil_id=0;
	for (; i< 213; i+=16)
	{
		double pil_div=M_SQRT1_2;
		if(i==0||i==4||i==208||i==212)
			pil_div = 1;

		pil[0] = cos((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);
		pil[1] = sin((double)2.0 * M_PI * (double)phases[symbol_id][pil_id] / 1024.0);

		fftw_complex pil_diff;
		pil_diff[0] = (symbols[i][0] * pil[0] + symbols[i][1] * pil[1]) / pil_div;
		pil_diff[1] = (symbols[i][1] * pil[0] - symbols[i][0] * pil[1]) / pil_div;

		if(1 || is_initial_eq) {
			H_pilots[i][0] = pil_diff[0];
			H_pilots[i][1] = pil_diff[1];
		} else {
			H_pilots[i][0] = (H_pilots[i][0] + pil_diff[0])/2;
			H_pilots[i][1] = (H_pilots[i][1] + pil_diff[1])/2;
		}


#if 1
		//initial equalizer creation.
		//due time references can be used too, there is higher amount of pilots to use there.
		//can be enchanced by AFC pilots as well.
		if(1 || is_initial_eq) {
			int j;
			j=i-1;
			//while(j >=0 && (pil_id==0 || i-j <= 8)) {
			while(j >=0 && i-j <= 2) {
				H_pilots[j][0] = H_pilots[i][0];
				H_pilots[j][1] = H_pilots[i][1];
				j--;
			}

			j=i+1;
			//while(j < 213 && ((pil_id==13 || (pil_id==12 && phases[symbol_id][pil_id+1] == 32767)) || j-i <= 8)) {
			while(j < 213 && j-i <= 2) {
				H_pilots[j][0] = H_pilots[i][0];
				H_pilots[j][1] = H_pilots[i][1];
				j++;
			}
#else
			if(is_initial_eq) {
				int j;
				j=i-1;
				while(j >=0 && (pil_id==0 || i-j <= 8)) {
					H_pilots[j][0] = H_pilots[i][0];
					H_pilots[j][1] = H_pilots[i][1];
					j--;
				}

				j=i+1;
				while(j < 213 && ((pil_id==13 || (pil_id==12 && phases[symbol_id][pil_id+1] == 32767)) || j-i <= 8)) {
					H_pilots[j][0] = H_pilots[i][0];
					H_pilots[j][1] = H_pilots[i][1];
					j++;
				}
#endif
		} else {
#if 0
			if(i >= 16) {
				fftw_complex pil_diff;
				pil_diff[0] = (H_pilots[i][0] - H_pilots[i-16][0])/16;
				pil_diff[1] = (H_pilots[i][1] - H_pilots[i-16][1])/16;

				int j;
				for(j=0;j<16;j++) {
					H_pilots[i-j][0] = (H_pilots[i-j][0] + H_pilots[i][0] - j*pil_diff[0])/2;
					H_pilots[i-j][1] = (H_pilots[i-j][1] + H_pilots[i][1] - j*pil_diff[1])/2;
				}
			} else if(i > 3) {
				fftw_complex pil_diff;
				pil_diff[0] = (H_pilots[i][0] - H_pilots[i-4][0])/4;
				pil_diff[1] = (H_pilots[i][1] - H_pilots[i-4][1])/4;

				H_pilots[i-1][0] = (H_pilots[i-1][0] + H_pilots[i][0] - pil_diff[0]) / 2;
				H_pilots[i-1][1] = (H_pilots[i-1][1] + H_pilots[i][1] - pil_diff[1]) / 2;

				H_pilots[i-2][0] = (H_pilots[i-2][0] + H_pilots[i][0] - 2*pil_diff[0]) / 2;
				H_pilots[i-2][1] = (H_pilots[i-2][1] + H_pilots[i][1] - 2*pil_diff[1]) / 2;

				H_pilots[i-3][0] = (H_pilots[i-3][0] + H_pilots[i][0] - 3*pil_diff[0]) / 2;
				H_pilots[i-3][1] = (H_pilots[i-3][1] + H_pilots[i][1] - 3*pil_diff[1]) / 2;
			}
#else

			if(i>0) {
				int cmp_num = i>=16 ? 16 : i;
				fftw_complex pil_diff;
				pil_diff[0] = (H_pilots[i][0] - H_pilots[i-cmp_num][0])/cmp_num;
				pil_diff[1] = (H_pilots[i][1] - H_pilots[i-cmp_num][1])/cmp_num;

				int j;
				for(j=0;j<cmp_num;j++) {
					H_pilots[i-j][0] = (H_pilots[i-j][0] + H_pilots[i][0] - j*pil_diff[0])/2;
					H_pilots[i-j][1] = (H_pilots[i-j][1] + H_pilots[i][1] - j*pil_diff[1])/2;
				}
			}

			if(pil_id==13 || (pil_id==12 && phases[symbol_id][pil_id+1] == 32767)) {
				//TODO: enchance tail...
			}
#endif

		}

		pil_id++;
	}
}

void drm_equalize_symbol(fftw_complex * symbols, fftw_complex *H_pilots)
{
	int i;
	/* do actual equalization */
	for (i=0; i<213; i++) //do the equalizing
	{
		fftw_complex tmp = { symbols[i][0], symbols[i][1] };
		double magsq = mag_squared(H_pilots[i]);
		symbols[i][0] = (tmp[0] * H_pilots[i][0] + tmp[1] * H_pilots[i][1]) / magsq;
		symbols[i][1] = (tmp[1] * H_pilots[i][0] - tmp[0] * H_pilots[i][1]) / magsq;
	}
}





void drm_decode_qam4(fftw_complex z, int8_t *r0, int8_t *r1)
{
	int8_t i_dev, q_dev;
	//char *result;
	double a =0.70711;

	if (z[0]==0 && z[1]==0) {
		fprintf(stderr, "Error: QAM4 trying to decode zero!\n");
		return;
	}

	i_dev=abs(fmod(abs(z[0])-a,a))*127;
	q_dev=abs(fmod(abs(z[1])-a,a))*127;

	*r0 = z[0] < 0 ? 127-i_dev : -127+i_dev;
	*r1 = z[1] < 0 ? 127-i_dev : -127+q_dev;
}


#define PRBS_DEGREE 9

int drm_prbs(int *in, size_t size)
{
     int i;

     unsigned short reg = 0x01ff, xor_val;

     for ( i = 0; i < size; i++ ){
          xor_val = ((reg&(1<<4))&&1)^((reg&(1<<8))&&1);
          reg <<=1;
          reg |= xor_val&1;
          //reg &= 0x01ff;
          in[i] ^= xor_val;
     }
     return 0;
}

void drm_descramble_int(int *in,int N) {
	#define DESCRAMBLE_WORDLENGTH 9
	uint8_t registers[DESCRAMBLE_WORDLENGTH];
	uint8_t prbs;
	long i, j;
	// set initialization word:
	for (i=0; i<DESCRAMBLE_WORDLENGTH; i++)
		registers[i]=1;
	for (j=0; j<N; j++) {
		prbs=registers[4]^registers[8] ? 1 : 0;
		for (i=DESCRAMBLE_WORDLENGTH-1; i>0; i--)
			registers[i]=registers[i-1];
		registers[0]=prbs;
		in[j] ^=  (int) prbs;
	}
	j=0;
}
