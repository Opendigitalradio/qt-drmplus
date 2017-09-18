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


#include "drmplus_internal.h"
#include "midiviterbi.h"
#include "fac.h"

#include <string.h>

int msc_is_superframe_saved = 0;
int msc_frame_id = -1;

const int MSCpolynomials[] = { 0x5b, 0x79, 0x65, 0x5b };

int calcMSCmetric(int n, int8_t *symbol, int *encoderoutput) {
    int i, d, dtot;
    dtot = 0;
    for(i = 0; i < n; i++)
    {
//        d = symbol[i] - (128-256*encoderoutput[i]);

        if (symbol[i] == 0)
            d = 0;
        else
            //d = (symbol[i] < 0 ? -128 : 128) - (128-256*encoderoutput[i]);
			d = (symbol[i] < 0 ? -128 : 128) - (128-(encoderoutput[i] << 8));

        if (d > 0)
            dtot += d;
        else
            dtot -= d;
    }
    return dtot;
}


//code rates:
//qam4:  0,25 0,333 0,4 0,5
//qam4:  1/4, 1/3, 2/5, 1/2
//qam16: 0,33 0,411 0,5 0,625
//number of bits:
//qam4:  3727 4969 5962 7454
//qam16: 9938 12243 14907 18635

const int puncturing_patterns[4][9] = {
	/* 1/4  index   0 */
	{  1, 1, 1, 1,  -1,               },
	/* 1/3  index   1 */
	{  1, 1, 1, 0,  -1,               },
	/* 2/5  index   2 */
	{  1, 1, 1, 0,   1, 1, 0, 0,  -1, },
	/* 1/2  index   3 */
	{  1, 1, 0, 0,  -1,               }
};


const int tail_patterns[12][25] = {
/*ok*/		{  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 0,  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 0, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 0, 0,  -1,               },
/*ok*/		{  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  -1,               },
/*ok*/		{  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 0,  -1,               },
/*ok*/		{  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 0,  -1,               },
/*ok*/		{  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 0,  -1,               },
/*ok*/		{  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 1,  -1,               },
/*ok*/		{  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 1,  -1,               },
};

const int pp_num[4] = {1, 1, 2, 1};
const int pp_den[4] = {4, 3, 5, 2};

//
const int qam4_sizes[4] = {3727, 4969, 5962, 7454};

#if 1
int drm_process_msc(iq_val *msc_bits, uint8_t *msc_data, mpx_desc_t *mpx, srv_data_t *srv)
{
	//only QAM4 for now...
	//if(msc_mode != 3) return 0;

	int i;
	uint16_t msc_lenA = mpx->info.length_A[0] + mpx->info.length_A[1] + mpx->info.length_A[2] + mpx->info.length_A[3];
	uint16_t msc_lenB = mpx->info.length_B[0] + mpx->info.length_B[1] + mpx->info.length_B[2] + mpx->info.length_B[3];

	fprintf(stderr, "MSC FINAL: len: %d+%d=%d levels:%d|%d type: %s\n",
			msc_lenA, msc_lenB, msc_lenA+msc_lenB, mpx->info.prot_A, mpx->info.prot_B, msc_lenA==0 ? "EEP" : "UEP");

	if(msc_lenA == 0 && msc_lenB  == 0)
		return 0;

	int8_t mscsDeinter[7460*2];
	unsigned int perm[7460*2];
    int metrics[64];
	int MSCdecoded[7460*2];
	memset(MSCdecoded, 0x00, sizeof(int)*7460*2);

	int msc_len = (msc_lenA+msc_lenB)*8;

    int uep_a_cells = (8 * msc_lenA  / (2*pp_num[mpx->info.prot_A])) * pp_den[mpx->info.prot_A];
    int uep_b_cells = 7460 - uep_a_cells;
    fprintf(stderr, "MSC cells A: %d, B: %d\n", uep_a_cells, uep_b_cells);

    //int uep_a_len = 2 * uep_a_cells * pp_num[msc_protA] / pp_den[msc_protA];
    //int uep_b_len = pp_num[msc_protB] * ((2 * uep_b_cells - 12) / pp_den[msc_protB]);
    int uep_a_len = 2 * uep_a_cells;
    int uep_b_len = (2 * uep_b_cells - 12);
    fprintf(stderr, "MSC lengths A: %d, B: %d\n", uep_a_len, uep_b_len);

#if 0
	uep_b_len = 7460*2 - uep_a_len;//8 * msc_lenB * pp_den[msc_protB] / pp_num[msc_protB];
#else
	//if(uep_a_len)
	//	uep_b_len -= 6*3;
#endif


	int uep_a_len_out = pp_num[mpx->info.prot_A] * uep_a_len / pp_den[mpx->info.prot_A];
	int uep_b_len_out = pp_num[mpx->info.prot_B] * (uep_b_len / pp_den[mpx->info.prot_B]);
	int tail_bits = uep_b_len - pp_den[mpx->info.prot_B] * (uep_b_len/pp_den[mpx->info.prot_B]);

	fprintf(stderr, "MSC A: %dbits, B: %dbits, left: %d\n", uep_a_len, uep_b_len, 7460*2 - (uep_a_len+uep_b_len));
	fprintf(stderr, "MSC A OUT: %dbits, B: %dbits, total: %d (%d bytes)\n", uep_a_len_out, uep_b_len_out,
			(uep_a_len_out+uep_b_len_out), (uep_a_len_out+uep_b_len_out)/8);
	fprintf(stderr, "MSC TAIL: %d\n", tail_bits);

	//build new permutation table A if needed...
	if(msc_lenA > 0 && msc_lenA != mpx->msc_len_A) {
		if(!mpx->msc_perm_table_A)
			mpx->msc_perm_table_A = malloc(uep_a_len*sizeof(uint16_t));
		else
			mpx->msc_perm_table_A=realloc(mpx->msc_perm_table_A, uep_a_len*sizeof(uint16_t));
		build_permutation(21, uep_a_len, mpx->msc_perm_table_A);
		fprintf(stderr, "Created perm. table A: %d\n", uep_a_len);
	}

	//build new permutation table B if needed...
	if(msc_lenB > 0 && msc_lenB != mpx->msc_len_B) {
		if(!mpx->msc_perm_table_B)
			mpx->msc_perm_table_B = malloc((7460*2 - uep_a_len)*sizeof(uint16_t));
		else
			mpx->msc_perm_table_B=realloc(mpx->msc_perm_table_B, (7460*2 - uep_a_len)*sizeof(uint16_t));
		build_permutation(21, 7460*2 - uep_a_len, mpx->msc_perm_table_B);
		fprintf(stderr, "Created perm. table B: %d\n", 7460*2 - uep_a_len);
	}

	mpx->msc_len_A=msc_lenA;
	mpx->msc_len_B=msc_lenB;

#if 0
	for(i=0;i<uep_a_len;i++)
		mscsDeinter[mpx->msc_perm_table_A[i]]=msc_bits[i/2][i%2];
	for(i=0;i<7460*2 - uep_a_len;i++)
		mscsDeinter[uep_a_len + mpx->msc_perm_table_B[i]]=msc_bits[(uep_a_len + i)/2][(uep_a_len + i)%2];
#else
	for(i=0;i<7460*2;i++) {
		if(i<uep_a_len)
			mscsDeinter[mpx->msc_perm_table_A[i]]=msc_bits[i/2][i%2];
		else
			mscsDeinter[uep_a_len + mpx->msc_perm_table_B[i-uep_a_len]]=msc_bits[i/2][i%2];
	}
#endif
	if(msc_lenA)
		SoftDecisionViterbitDecoder(4, 7, MSCpolynomials, 8 * msc_lenA, &mscsDeinter[0], &MSCdecoded[0],
				puncturing_patterns[mpx->info.prot_A], metrics, calcMSCmetric);
	SoftDecisionViterbitDecoder(4, 7, MSCpolynomials, 8 * msc_lenB, &mscsDeinter[uep_a_len], &MSCdecoded[8 * msc_lenA],
			puncturing_patterns[mpx->info.prot_B], metrics, calcMSCmetric);

	//drm_descramble_int(MSCdecoded, (msc_lenA+msc_lenB)*8);
	drm_prbs(MSCdecoded, (msc_lenA+msc_lenB)*8);

	for ( i = 0;  i< msc_len; i++ )
		msc_data[i/8] = msc_data[i/8] << 1 | MSCdecoded[i];

	//feed_aac_frame_data(msc_data, msc_lenA, msc_lenB, (msc_SR == 3) ? 5 : 10);
	//feed_aac_frame_data(msc_data, msc_lenA, msc_lenB, (msc_SR == 3) ? 5 : 10);
	return msc_lenA+msc_lenB;
}

#else



typedef struct {
	int RXp0;
	int RYp0;
	int RXp1;
	int RYp1;
	int RYlcm;
} rate_desc_t;

rate_desc_t ratesQAM4[4] = {
	/* 1/4  index   0 */
	{  1, 4, 0, 4, 4 },
	/* 1/3  index   1 */
	{  1, 3, 0, 3, 3 },
	/* 2/5  index   2 */
	{  2, 5, 0, 5, 5 },
	/* 1/2  index   3 */
	{  1, 2, 0, 2, 2 }
};

rate_desc_t ratesQAM16[4] = {
	/* 1/6+1/2  index   0 */
	{  1, 6, 1, 2, 6 },
	/* 1/4+4/7  index   1 */
	{  1, 4, 4, 7, 28 },
	/* 1/3+2/3  index   2 */
	{  1, 3, 2, 3, 3 },
	/* 1/2+3/4  index   3 */
	{  1, 2, 3, 4, 3 }
};





/*
 *
MSC FINAL: len: 157+588=745 levels:2|2 type: UEP
MSC cells A: 1570, B: 5890
MSC lengths A: 3140, B: 11768
MSC A: 3140bits, B: 11768bits, left: 12
MSC A OUT: 1256bits, B: 4706bits, total: 5962 (745 bytes)
MSC TAIL: 3

Created perm. table A: 3140
Created perm. table B: 11768-12


MSC FINAL: len: 157+588=745 levels:2|2 type: UEP
MSC pA:2 Rp0:0.400000, Rp1:0.000000, N1:1570, N2:5890, L1:1256, L2:4706, rP0:3, rP1:3
MSC cells A: 1570, B: 5890
MSC lengths A: 2512, B: 9412
MSC A: 2512bits, B: 9412bits, left: 2996
MSC A OUT: 1256bits, B: 4706bits, total: 5962 (745 bytes)
MSC TAIL: 2

 */

int drm_process_msc(iq_val *msc_bits, uint8_t *msc_data, mpx_desc_t *mpx, srv_data_t *srv)
{
	//only QAM4 for now...
	//if(msc_mode != 3) return 0;

	int i;
	int8_t mscsDeinter[7460*2];
	unsigned int perm[7460*2];
    int metrics[64];
	int MSCdecoded[7460*2];

	rate_desc_t *rA;

	if(mpx->info.msc_mode != 0)
		rA = &ratesQAM4[mpx->info.prot_A];
	else
		rA = &ratesQAM16[mpx->info.prot_A];
	//rate_desc_t rB = &*rates4[mpx->info.prot_B];
	uint16_t msc_lenA = mpx->info.length_A[0] + mpx->info.length_A[1] + mpx->info.length_A[2] + mpx->info.length_A[3];
	uint16_t msc_lenB = mpx->info.length_B[0] + mpx->info.length_B[1] + mpx->info.length_B[2] + mpx->info.length_B[3];

	fprintf(stderr, "MSC FINAL: len: %d+%d=%d levels:%d|%d type: %s\n",
			msc_lenA, msc_lenB, msc_lenA+msc_lenB, mpx->info.prot_A, mpx->info.prot_B, msc_lenA==0 ? "EEP" : "UEP");

	if(msc_lenA == 0 && msc_lenB  == 0)
		return 0;

	memset(MSCdecoded, 0x00, sizeof(int)*7460*2);


	double Rp0 = (double)rA->RXp0 / (double)rA->RYp0;
	double Rp1 = (double)rA->RXp1 / (double)rA->RYp1;

	int X = msc_lenA;
	int Nmux = 7460;

	int N1 = ceil(8.0 * X / (2.0 * rA->RYlcm * (Rp0 + Rp1))) * rA->RYlcm;
	int N2 = Nmux - N1;


	int L1 = (double) (2.0 * N1 * Rp0 +  2.0 * N1 * Rp1);
	int L2 = rA->RXp0 * floor((2.0*N2 - 12.0) / rA->RYp0)  +  rA->RXp1 * floor((2.0*N2 - 12.0) / rA->RYp1);

	int rP0 = (2*N2 - 12) - rA->RYp0 * floor((2.0*N2 - 12.0) / rA->RYp0);
	int rP1 = (2*N2 - 12) - rA->RYp1 * floor((2.0*N2 - 12.0) / rA->RYp1);
	fprintf(stderr, "MSC pA:%u Rp0:%f, Rp1:%f, N1:%d, N2:%d, L1:%d, L2:%d, rP0:%d, rP1:%d\n", mpx->info.prot_A, Rp0, Rp1, N1, N2, L1, L2, rP0, rP1);


	//int *tp = tail_patterns[rP0];


	int msc_len = (msc_lenA+msc_lenB)*8;

    int uep_a_cells = (8 * msc_lenA  / (2*pp_num[mpx->info.prot_A])) * pp_den[mpx->info.prot_A];
    int uep_b_cells = 7460 - uep_a_cells;
    fprintf(stderr, "MSC cells A: %d, B: %d\n", uep_a_cells, uep_b_cells);

    //int uep_a_len = 2 * uep_a_cells * pp_num[msc_protA] / pp_den[msc_protA];
    //int uep_b_len = pp_num[msc_protB] * ((2 * uep_b_cells - 12) / pp_den[msc_protB]);
    //int L1*2 = 2 * uep_a_cells;
    //int L2*2 = (2 * uep_b_cells - 12);
    fprintf(stderr, "MSC lengths A: %d, B: %d\n", N1*2, N2*2);

#if 0
	L2*2 = 7460*2 - L1*2;//8 * msc_lenB * pp_den[msc_protB] / pp_num[msc_protB];
#else
	//if(L1*2)
	//	L2*2 -= 6*3;
#endif


	//int L1 = pp_num[mpx->info.prot_A] * uep_a_len / pp_den[mpx->info.prot_A];
	//int L2 = pp_num[mpx->info.prot_B] * (uep_b_len / pp_den[mpx->info.prot_B]);
	int tail_bits = (rP0);

	fprintf(stderr, "MSC A: %dbits, B: %dbits, left: %d\n", N1*2, N2*2, 7460*2 - (N1*2+N2*2));
	fprintf(stderr, "MSC A OUT: %dbits, B: %dbits, total: %d (%d bytes)\n", L1, L2, (L1+L2), (L1+L2)/8);
	fprintf(stderr, "MSC TAIL: %d | %d\n", tail_bits, rP0);

	//build new permutation table A if needed...
	if(msc_lenA > 0 && msc_lenA != mpx->msc_len_A) {
		if(!mpx->msc_perm_table_A)
			mpx->msc_perm_table_A = malloc(N1*2*sizeof(uint16_t));
		else
			mpx->msc_perm_table_A=realloc(mpx->msc_perm_table_A, N1*2*sizeof(uint16_t));
		build_permutation(21, N1*2, mpx->msc_perm_table_A);
		fprintf(stderr, "Created perm. table A: %d\n", N1*2);
	}

	//build new permutation table B if needed...
	if(msc_lenB > 0 && msc_lenB != mpx->msc_len_B) {
		if(!mpx->msc_perm_table_B)
			mpx->msc_perm_table_B = malloc((7460*2 - N1*2)*sizeof(uint16_t));
		else
			mpx->msc_perm_table_B=realloc(mpx->msc_perm_table_B, (7460*2 - N1*2)*sizeof(uint16_t));
		build_permutation(21, 7460*2 - N1*2, mpx->msc_perm_table_B);
		fprintf(stderr, "Created perm. table B: %d\n", 7460*2 - N1*2);
	}

	mpx->msc_len_A=msc_lenA;
	mpx->msc_len_B=msc_lenB;

#if 0
	for(i=0;i<L1*2;i++)
		mscsDeinter[mpx->msc_perm_table_A[i]]=msc_bits[i/2][i%2];
	for(i=0;i<7460*2 - L1*2;i++)
		mscsDeinter[L1*2 + mpx->msc_perm_table_B[i]]=msc_bits[(L1*2 + i)/2][(L1*2 + i)%2];
#else
	for(i=0;i<7460*2;i++) {
		if(i<N1*2)
			mscsDeinter[mpx->msc_perm_table_A[i]]=msc_bits[i/2][i%2];
		else
			mscsDeinter[N1*2 + mpx->msc_perm_table_B[i-N1*2]]=msc_bits[i/2][i%2];
	}
#endif
	if(msc_lenA)
		SoftDecisionViterbitDecoder(4, 7, MSCpolynomials, L1, &mscsDeinter[0], &MSCdecoded[0],
				puncturing_patterns[mpx->info.prot_A], metrics, calcMSCmetric);


	SoftDecisionViterbitDecoder(4, 7, MSCpolynomials, L2-tail_bits /*minus l punct.*/, &mscsDeinter[N1*2], &MSCdecoded[L1],
			puncturing_patterns[mpx->info.prot_B], metrics, calcMSCmetric);

	SoftDecisionViterbitDecoder(4, 7, MSCpolynomials, tail_bits /*minus l punct.*/, &mscsDeinter[(N1+N2)*2 - 12 - rP0], &MSCdecoded[L1+L2-tail_bits],
			tail_patterns[rP0], metrics, calcMSCmetric);

	//drm_descramble_int(MSCdecoded, (msc_lenA+msc_lenB)*8);
	drm_prbs(MSCdecoded, msc_len);

	for ( i = 0;  i< msc_len; i++ )
		msc_data[i/8] = msc_data[i/8] << 1 | MSCdecoded[i];

	//feed_aac_frame_data(msc_data, msc_lenA, msc_lenB, (msc_SR == 3) ? 5 : 10);
	//feed_aac_frame_data(msc_data, msc_lenA, msc_lenB, (msc_SR == 3) ? 5 : 10);
	return msc_lenA+msc_lenB;
}

#endif
