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

int FACpolynomials[] = { 0x5b, 0x79, 0x65, 0x5b };
int FACpunctuation[] = { 1, 1, 1, 1, -1 }; // 1/4


int calcFACmetric(int n, int8_t *symbol, int *encoderoutput) {
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

#if 0
int charCmp(char *in1, char *in2, int length) {
	int i, res=0;

	for (i=0; i<length; i++)
		if (in1[i] != in2[i]) res=-1;

	return res;
}
#endif


int drm_process_fac(int8_t *fac_deinterleaved, uint8_t *fac_data)
{

	int FACdecoded[244];
	memset(FACdecoded, 0x00, sizeof(int)*244);

	SoftDecisionViterbitDecoder(4, 7, FACpolynomials, 108 + 8, fac_deinterleaved, FACdecoded,
	                                FACpunctuation, NULL, calcFACmetric);

	//drm_descramble_int(FACdecoded, 108 + 8);
	drm_prbs(FACdecoded, 108 + 8);

	int i;
#if 0
	char FACchar[108+8];
	char FACchar2[108+4+8];
	for (i=0; i<108; i++) {
		FACchar[i]=(char) FACdecoded[i];
//		if(i<100)
//			FACchar2[i]=0;
//		else
		FACchar2[i]=(char) FACdecoded[i];
	}
	FACchar2[108]=0;FACchar2[109]=0;FACchar2[110]=0;FACchar2[111]=0;
	for (i=108; i<108+8; i++) {
		FACchar[i]=(char) FACdecoded[i];
		FACchar2[i+4]=(char) FACdecoded[i];
	}
#endif

	//NEW!!!!
	uint8_t fac_data_raw[15] = { 0 };
	for ( i = 0;  i< 112+8; i++ ) {
		if(i<108)
			fac_data_raw[i/8] = fac_data_raw[i/8] << 1 | FACdecoded[i];
		else if(i<112)
			fac_data_raw[i/8] = fac_data_raw[i/8] << 1;
		else
			fac_data_raw[i/8] = fac_data_raw[i/8] << 1 | FACdecoded[i-4];
	}

	//width=8 poly=0x1d init=0xff refin=false refout=false xorout=0xff check=0x4b name="CRC-8/SAE-J1850"
	//G 8 (x) = x 8 + x 4 + x 3 + x 2 + 1
	//CRC-8-SAE J1850   x8 + x4 + x3 + x2 + 1     0x1D or 0xB8

	uint8_t crc_calc = crc8_fac(0x00, fac_data_raw, 14);
	//fprintf(stderr, "\ncrc_rx=%02x crc_ok=%02x\n", fac_data_raw[14], crc_calc);

	if(fac_data && fac_data_raw[14] == crc_calc )
		memcpy(fac_data, fac_data_raw, 15);

	//print_bytes(0, (char *) fac_data_raw, 15);
	return (fac_data_raw[14] == crc_calc);

#if 0
	//G 8 (x) = x 8 + x 4 + x 3 + x 2 + 1
	char crcPolyFac[9]={1,0,1,1,1,0,0,0,1};
	char crcPolyFac2[]={0,0,0,1,1,1,0};
	char crc[8], crc2[8];
	drm_calc_crc(crc, FACchar2, crcPolyFac, 112, 8);
	int crc1v = drm_calc_crc8(crc2, FACchar2, crcPolyFac2, 112+8);
	int crc2v = drm_calc_crc2(crc2, FACchar2, crcPolyFac2, 112+8, 8);
	char crcOk=0;

	fprintf(stderr, "\ncrc2v=%d, crc_rx=%02x crc_ok=%02x crc_ok2=%02x\n", crc2v, fac_data_raw[14], crc_ok, crc_ok2);

	if (0==charCmp(crc,FACchar+108,8))
	{
		printf("\nCRC ok!\n");
		crcOk=1;

//printf("Calculated CRC:%d%d%d%d%d%d%d%d \n",crc[0],crc[1],crc[2],crc[3],crc[4],crc[5],crc[6],crc[7]);
	} else {
		printf("\nCRC check failed!\n");
		crcOk=0;
		//printf("Calculated CRC:%d%d%d%d%d%d%d%d was:%d%d%d%d%d%d%d%d \n",crc[0],crc[1],crc[2],crc[3],crc[4],crc[5],crc[6],crc[7], facsDeinter[64],facsDeinter[64+1],facsDeinter[64+2],facsDeinter[64+3],facsDeinter[64+4],facsDeinter[64+5],facsDeinter[64+6],facsDeinter[64+7]);
		printf("Calculated CRC:%d%d%d%d%d%d%d%d was:%d%d%d%d%d%d%d%d can be:%d%d%d%d%d%d%d%d \n",
				crc[0],crc[1],crc[2],crc[3],crc[4],crc[5],crc[6],crc[7],
				FACdecoded[108],FACdecoded[108+1],FACdecoded[108+2],FACdecoded[108+3],FACdecoded[108+4],FACdecoded[108+5],FACdecoded[108+6],FACdecoded[108+7],
				crc2[0],crc2[1],crc2[2],crc2[3],crc2[4],crc2[5],crc2[6],crc2[7]
				);
	}
	return crcOk;
#endif
}






int drm_fill_services_from_fac(dense_fac_t *f, srv_data_t *s, mpx_info_t *info)
{
	srv_data_t *s0 = &s[f->short_id0];
	srv_data_t *s1 = &s[f->short_id1];

	uint32_t s_id = f->service_id0_20 << 20 | f->service_id0_12 << 12 | f->service_id0_4 << 4 | f->service_id0_0;
	if(s_id != s0->serviceId) {
		s0->flags=0;
		fprintf(stderr, "new service ID[%u]: %u != %u (%s)\n", f->short_id0, s_id, s0->serviceId, f->audio_data1 ? "D" : "A");
	}
	s0->serviceId = s_id;
	s0->shortId = f->short_id0;
	s0->audioCaFlag = f->audio_ca0;
	s0->language = f->lang0_3 << 3 | f->lang0_0;
	s0->audioDataFlag = f->audio_data0;
	s0->serviceDesc = f->service_desc0_1 << 1 | f->service_desc0_0;
	s0->dataCaFlag = f->data_ca0;

	if(f->short_id0 != f->short_id1) {
		s_id = f->service_id1_16 << 16 | f->service_id1_8 << 8 | f->service_id1_0;
		if(s_id != s1->serviceId) {
			s1->flags=0;
			fprintf(stderr, "new service ID[%u]: %u != %u (%s)\n", f->short_id1, s_id, s1->serviceId, f->audio_data1 ? "D" : "A");
		}
		s1->serviceId = s_id;
		s1->shortId = f->short_id1;
		s1->audioCaFlag = f->audio_ca1;
		s1->language = f->lang1;
		s1->audioDataFlag = f->audio_data1;
		s1->serviceDesc = f->service_desc1;
		s1->dataCaFlag = f->data_ca1;
	}

    uint8_t a = f->services >> 2 & 0x03;
    uint8_t d = f->services      & 0x03;

    a = a == 0 && d == 0 ? 4 : a;
    d = a == 3 && d == 3 ? d : d;

    if(a != info->service_num_a || d != info->service_num_d) {
    	info->service_num_a=a;
    	info->service_num_d=d;
    	return 1;
    } else {
    	return 0;
    }

//	fprintf(stderr, "SIDs[%d|%d]: %06x | %06x\n",  f->short_id0, f->short_id1, s0->serviceId, s1->serviceId);
}
