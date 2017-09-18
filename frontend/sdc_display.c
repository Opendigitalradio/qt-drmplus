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


#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdarg.h>
#include <stdint.h>
#include <drmplusdemod.h>



const char audioCodingStr[4][40]={"AAC","reserved","reserved","xHE-AAC"};
const char audioModeAacStr[4][50]={"mono","parametric stereo", "stereo", "reserved"};
const char audioSamplingRateStrAAC[8][40]={"reserved","12 kHz","reserved","24 kHz","reserved","48 kHz","reserved","reserved"};
const char audioSamplingRateStrXHEAC[8][40]={"9.6 kHz","12 kHz","16 kHz","19.2 kHz","24 kHz","32 kHz","38.4 kHz","48 kHz"};
const char protLevels[4][4] = {"1/4", "1/3", "2/5", "1/2"};


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

/**
 * Create a string representation of the SDC.
 *
 * @param s SDC
 * @param size SDC size
 *
 * @return A string describtion of the content of the sdc.
 */
char * describe_sdc(uint8_t *s, int size)
{

     int tmp, i;
     char *new, *old;
     char *t = NULL;
     asprintf_noret(&old,"%s","_____________oo00 SDC 00oo_____________\n");

     asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "AFS Index", s[0], s[0]);
     free(old);
     old = new;

    uint8_t e_pos = 1;
 	while(e_pos < (size-2)) { //minus crc
 		uint8_t e_len =  s[e_pos] >> 1;
 		uint8_t e_ver =  s[e_pos] & 0x01;
 		uint8_t e_type =  s[e_pos+1] >> 4;
 		uint8_t *e_data = &s[e_pos+2];


 		switch(e_type) {
 		case 0: {
 			if(e_len < 3) break;
 			uint8_t in_partAProtection = (s[e_pos+1] >> 2)  & 0x03;
 			uint8_t in_partBProtection = s[e_pos+1] & 0x03;

		    asprintf_noret(&new,"%s_____________oo00 Multiplex description 00oo_____________\n", old);
		    free(old);
		    old = new;

		     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,
		                    "Prot. lvl A", in_partAProtection, protLevels[in_partAProtection]);
		     free(old);
		     old = new;

		     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,
		                    "Prot. lvl B", in_partBProtection, protLevels[in_partBProtection]);
		     free(old);
		     old = new;

		     for(i=0;i<e_len;i+=3) {
 				uint32_t streamDesc = ((uint32_t)e_data[i]) << 16 | ((uint16_t)e_data[i+1]) << 8 | (e_data[i+2]);

 			     asprintf_noret(&new, "%s%-20s (%2hhd): A:%u B:%u\n", old,
 			                    "Stream",  i/3, streamDesc >> 12, streamDesc & 0x0FFF);
 			     free(old);
 			     old = new;
 			}
 			break;
 		}
 		case 1: {
 			if(!e_len) break;
 			uint8_t shortId = (s[e_pos+1] >> 2)  & 0x03;

		    asprintf_noret(&new,"%s_____________oo00 Label data 00oo_____________\n", old);
		    free(old);
		    old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %.*s\n", old,
		                    "Stream",  shortId, e_len, (char*)e_data);
		    free(old);
		    old = new;
 			break;
 		}
 		case 8: {
 			asprintf_noret(&new,"%s_____________oo00 Timing data 00oo_____________\n", old);
 			free(old);
 			old = new;
 			if(e_len < 3) break;
 			uint32_t ts =  ((uint32_t)s[e_pos+1] & 0x0F) << 24 | ((uint32_t)e_data[0]) << 16 | ((uint16_t)e_data[1]) << 8 | (e_data[2]);
 			long Year, Month, Day;
 			MjdToDate (ts >> 11, &Year, &Month, &Day);
		    asprintf_noret(&new, "%s%-20s (  ): %02u-%02u-%04u %02u:%02u\n", old,
		                    "Time/Date",  Day, Month, Year, (ts >> 6) & 0x1F, ts & 0x3F);
 			free(old);
 			old = new;
 			break;
 		}
 		case 9: {
 			if(e_len < 2) break;
 			uint8_t shortId = (s[e_pos+1] >> 2)  & 0x03;
 			uint8_t streamId = (s[e_pos+1])  & 0x03;
 			uint8_t audioCoding = e_data[0] >> 6;
 			uint8_t sbrFlag = (e_data[0] >> 5) & 0x01;
 			uint8_t audioMode = (e_data[0] >> 3) & 0x03;
 			uint8_t audioSamplingRate = e_data[0] & 0x07;
 			uint8_t textFlag = (e_data[1] >> 7) & 0x01;
 			uint8_t enhancementFlag = (e_data[1] >> 6) & 0x01;
 			uint8_t coderField = e_data[1] & 0x1F;

 			asprintf_noret(&new,"%s_____________oo00 Audio Info 00oo_____________\n", old);
 			free(old);
 			old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "Short Id",  shortId, shortId);
 			free(old);
 			old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "Stream Id",  streamId, streamId);
 			free(old);
 			old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "Audio coding", audioCoding,audioCodingStr[audioCoding]);
 			free(old);
 			old = new;

 			asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "SBR flag", sbrFlag,sbrFlag);
 			free(old);
 			old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "AAC Audio mode", audioMode,audioModeAacStr[audioMode]);
 			free(old);
 			old = new;

		    asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "Audio SR",
		    		audioSamplingRate,audioCoding==3 ? audioSamplingRateStrXHEAC[audioSamplingRate] : audioSamplingRateStrAAC[audioSamplingRate]);
 			free(old);
 			old = new;

 			asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "Text flag", textFlag,textFlag);
 			free(old);
 			old = new;

 			asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "Enhanced flag", enhancementFlag,enhancementFlag);
 			free(old);
 			old = new;

 			asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old, "Coder Field", coderField,coderField);
 			free(old);
 			old = new;

 			if(e_len > 2) {
 	 			asprintf_noret(&new, "%s%-20s (%2hhd): ", old, "Codec Data", e_len-2);
 	 			free(old);
 	 			old = new;

 				for(i=2;i<e_len;i++) {
 					asprintf_noret(&new, "%s%02x", old, e_data[i]);
	 	 			free(old);
	 	 			old = new;
 				}
				asprintf_noret(&new, "%s\n", old);
 	 			free(old);
 	 			old = new;
 			}
 			break;
 		}
 		case 10: {
 			//don't care about next FAC...
 			break;
 		}
 		default:
 			break;
 		}

 		e_pos+=e_len+2;
 	}


     //is checked by library
     tmp=s[size-2] << 8 | s[size-1];

     asprintf_noret(&new, "%s%-18s (%04x): 0x%04x %s\n\n",
                    old, "CRC Sum",  tmp,
                    tmp,
                    "Ok!");
     free(old);

     return new;
}









void drm_show_sdc(uint8_t *f, int size)
{
    char* sdc = describe_sdc(f, size);
    fprintf(stderr, "%s",sdc );
    free(sdc);
}
