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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drmplus_internal.h"

uint8_t crc8_fac(uint8_t l_crc, const void *lp_data, unsigned l_nb)
{
    const uint8_t* data = (const uint8_t*)lp_data;
    while (l_nb--)
        l_crc = crc8tab[l_crc ^ *(data++)];
    return (l_crc);
}

uint16_t crc16_sdc(uint16_t l_crc, const void *lp_data, unsigned l_nb)
{
    const uint8_t* data = (const uint8_t*)lp_data;
    while (l_nb--)
    	l_crc = (l_crc << 8) ^ crc16tab[((l_crc >> 8) ^ *data++) & 0xff];

    return (l_crc);
}

int build_permutation(char t, size_t xin, uint16_t *perm)
{
     unsigned long long int i,s,q;

     if ( perm == NULL)
          return -1;

     if ( xin == 0)
          return 0;

     //s = 2^(ceil(log2(xin)))
     for ( i = sizeof(xin)*8-1; i >= 0; i--){
          //fprintf(stderr, "0x%08x & 0x%08d\n", 1<<i, xin);
          if ( 1<<i & xin ){
               s = 1 << (i+1);
               break;
          }
     }

     q = s/4 - 1;
     //fprintf(stderr, "s = %d, q = %d t = %d xin = %d\n", s, q, t, xin);

     perm[0] = 0;
     for (i = 1; i < xin; i++){
          perm[i] = (t * perm[i-1] + q)%s;

          while ( perm[i] >= xin ){
               perm[i] = (t*perm[i]+q)%s;
          }

     }

     return 0;
}

void print_bytes(char *bytes, int len) {
    int i;
    int count;
    int done = 0;

    while (len > done) {
		if (len-done > 32){
			count = 32;
		} else {
			count = len-done;
		}

		fprintf(stderr, "\t\t\t\t");

		for (i=0; i<count; i++) {
	    	fprintf(stderr, "%02x ", (int)((unsigned char)bytes[done+i]));
		}

		for (; i<32; i++) {
	    	fprintf(stderr, "   ");
		}


		fprintf(stderr, "\t\"");

        for (i=0; i<count; i++) {
	    	fprintf(stderr, "%c", isprint(bytes[done+i]) ? bytes[done+i] : '.');
        }

        for (; i<32; i++) {
	    	fprintf(stderr, " ");
		}

		fprintf(stderr, "\"\n");
    	done += count;
    }
}
