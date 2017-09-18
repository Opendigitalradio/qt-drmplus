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
#include "fac.h"
#include "midiviterbi.h"

#include <string.h>

int SDCpolynomials[] = { 0x5b, 0x79, 0x65, 0x5b };
int SDCpunctuation1_4[] = { 1, 1, 1, 1, -1 }; // 1/4
int SDCpunctuation1_2[] = { 1, 1, 0, 0, -1 }; // 1/2

int calcSDCmetric(int n, int8_t *symbol, int *encoderoutput) {
    int i, d, dtot;
    dtot = 0;
    for(i = 0; i < n; i++)
    {
        d = symbol[i] - (128-256*encoderoutput[i]);
        if (d > 0)
            dtot += d;
        else
            dtot -= d;
    }
    return dtot;
}


int drm_process_sdc(int8_t *sdc_deinterleaved, uint8_t *sdc_data, uint8_t sdc_mode)
{

    int metrics[64];
	int SDCdecoded[936];
	int i;

	//memset(SDCdecoded, 0x00, sizeof(int)*936);

	int sdc_data_bytes = sdc_mode ? 55 : 113;

	SoftDecisionViterbitDecoder(4, 7, SDCpolynomials, sdc_data_bytes*8 + 4 + 16, sdc_deinterleaved, SDCdecoded,
	                                sdc_mode ? SDCpunctuation1_4 :  SDCpunctuation1_2, NULL, calcSDCmetric);

	drm_prbs(SDCdecoded, sdc_data_bytes*8 + 4 + 16);

	sdc_data[0]=0;
	for ( i = 4;  i<(sdc_data_bytes+3)*8; i++ ){
	     sdc_data[i/8] = sdc_data[i/8] << 1 | SDCdecoded[i-4];
	}

	uint16_t crc_calc = crc16_sdc(0x0000, sdc_data, sdc_data_bytes+1);
	uint16_t crc_val = sdc_data[sdc_data_bytes+1] << 8 | sdc_data[sdc_data_bytes+2];

	if(crc_calc == crc_val) {
		//fprintf(stderr, "SDC CRC ok!\n");
		//print_bytes(0, sdc_data, sdc_data_bytes+3);
		return 1;
	} else {
		fprintf(stderr, "SDC CRC error! Calculated CRC: 0x%04x != %04x\n", crc_val, crc_calc);
		//print_bytes(0, sdc_data, sdc_data_bytes+3);
		return 0;
	}
}




static void MjdToDate (long Mjd, long *Year, long *Month, long *Day)
{
    long J, C, Y, M;

    J = Mjd + 2400001 + 68569;
    C = 4 * J / 146097;
    J = J - (146097 * C + 3) / 4;
    Y = 4000 * (J + 1) / 1461001;
    J = J - 1461 * Y / 4 + 31;
    M = 80 * J / 2447;
    *Day = J - 2447 * M / 80;
    J = M / 11;
    *Month = M + 2 - (12 * J);
    *Year = 100 * (C - 49) + Y + J;
}

int drm_fill_services_from_sdc(uint8_t *s, uint8_t len, srv_data_t *srv, mpx_desc_t *mpx)
{
	int i;

	mpx->sdc_num = s[0];

    uint8_t e_pos = 1;
 	while(e_pos < (len-2)) { //minus crc
 		uint8_t e_len =  s[e_pos] >> 1;
 		uint8_t e_ver =  s[e_pos] & 0x01;
 		uint8_t e_type =  s[e_pos+1] >> 4;
 		uint8_t *e_data = &s[e_pos+2];

#if 0
 		if(e_len) {
			fprintf(stderr, "e[%d] len:%d ver:%d, data:", e_type, e_len, e_ver);
			for (i=0;i<e_len;i++) {
				fprintf(stderr, " %02x", e_data[i]);
			}
			fprintf(stderr, "\n");
 		}
#endif

 		switch(e_type) {
 		case 0: {
 			if(e_len < 3) break;
 			mpx->info.prot_A = (s[e_pos+1] >> 2)  & 0x03;
 			mpx->info.prot_B = s[e_pos+1] & 0x03;
 			//fprintf(stderr, "prot_A=%d, prot_B=%d: ", mpx->prot_A, mpx->prot_B);
 			mpx->info.length_A[0]=0;mpx->info.length_A[1]=0;mpx->info.length_A[2]=0;mpx->info.length_A[3]=0;
 			mpx->info.length_B[0]=0;mpx->info.length_B[1]=0;mpx->info.length_B[2]=0;mpx->info.length_B[3]=0;
		     for(i=0;i<e_len;i+=3) {
 				uint32_t streamDesc = ((uint32_t)e_data[i]) << 16 | ((uint16_t)e_data[i+1]) << 8 | (e_data[i+2]);
 				//fprintf(stderr, "%d|%d ", streamDesc >> 12, streamDesc & 0x0FFF);
 				mpx->info.length_A[i/3] += streamDesc >> 12;
 				mpx->info.length_B[i/3] += streamDesc & 0x0FFF;
 			}
		    //fprintf(stderr, "\n");
		    //fprintf(stderr, "total: %d|%d\n", mpx->length_A, mpx->length_B);
 			break;
 		}
 		case 1: {
 			if(!e_len || e_len > 64) break;
 			uint8_t shortId = (s[e_pos+1] >> 2)  & 0x03;
 			memcpy(srv[shortId].label, (char*)e_data, e_len);
 			if(e_len < 64)
 				srv[shortId].label[e_len] = '\0';
			srv[shortId].flags |= 0x02;
 			break;
 		}
 		case 8: {
 			if(e_len < 3) break;
 			uint32_t ts =  ((uint32_t)s[e_pos+1] & 0x0F) << 24 | ((uint32_t)e_data[0]) << 16 | ((uint16_t)e_data[1]) << 8 | (e_data[2]);
 			long Year, Month, Day;
 			MjdToDate (ts >> 11, &mpx->info.Year, &mpx->info.Month, &mpx->info.Day);
 			mpx->info.Hours = (ts >> 6) & 0x1F;
 			mpx->info.Minutes = ts & 0x3F;
 			break;
 		}
 		case 9: {
 			if(e_len < 2) break;
 			uint8_t shortId = (s[e_pos+1] >> 2)  & 0x03;

			srv[shortId].streamId = (s[e_pos+1])  & 0x03;
			srv[shortId].audioCoding = e_data[0] >> 6;
			srv[shortId].sbrFlag = (e_data[0] >> 5) & 0x01;
			srv[shortId].audioMode = (e_data[0] >> 3) & 0x03;
			srv[shortId].audioSamplingRate = e_data[0] & 0x07;
			srv[shortId].textFlag = (e_data[1] >> 7) & 0x01;
			srv[shortId].enhancementFlag = (e_data[1] >> 6) & 0x01;
			srv[shortId].coderField = e_data[1] & 0x1F;

 			if(e_len > 2) {
 				srv[shortId].extradataLen=e_len-2;
 				for(i=2;i<e_len;i++) {
 					//e_data[i]
 					srv[shortId].extradata[i-2] =e_data[i];
 				}
 			}
			srv[shortId].flags |= 0x01;
 			break;
 		}
 		case 10: {
 			//don't care about next FAC...
 			break;
 		}
 		case 12: {
 			uint8_t shortId = (s[e_pos+1] >> 2)  & 0x03;
 			if(e_len > 4) {
 				srv[shortId].language2[0] = e_data[0];
 				srv[shortId].language2[1] = e_data[1];
 				srv[shortId].language2[2] = e_data[2];
 				srv[shortId].country[0] = e_data[3];
 				srv[shortId].country[1] = e_data[4];
 			}

 			break;
 		}
 		default:
 			break;
 		}

 		e_pos+=e_len+2;
 	}

	//fprintf(stderr, "drm_fill_services_from_sdc\n");
	for (i=0;i<mpx->info.service_num_a+mpx->info.service_num_d;i++)
		if(!srv[i].audioDataFlag && (srv[i].flags & 0x03)!=0x03 ||
		    srv[i].audioDataFlag && (srv[i].flags & 0x03)!=0x02) break;

	if(i<mpx->info.service_num_a+mpx->info.service_num_d)
		return 0;

	for (i=0;i<mpx->info.service_num_a+mpx->info.service_num_d;i++)
		if((srv[i].flags & 0x04)!=0x04) break;

	if(i==mpx->info.service_num_a+mpx->info.service_num_d)
		return 0;

	for (i=0;i<mpx->info.service_num_a+mpx->info.service_num_d;i++)
		srv[i].flags|=0x04;

	return 1;
}
