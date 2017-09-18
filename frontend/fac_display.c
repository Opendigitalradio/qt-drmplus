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

//Table 51
static const char * languages[] = {
     "No language specified",
     "Arabic",
     "Bengali",
     "Chinese (Mandarin)",
     "Dutch",
     "English",
     "French",
     "German",
     "Hindi",
     "Japanese",
     "Javanese",
     "Korean",
     "Portuguese",
     "Russian",
     "Spanish",
     "Other language"
};


//Table 52
static const char * prog_type[] = {
     "No programme type",
     "News",
     "Current affairs",
     "Information",
     "Sport",
     "Education",
     "Drama",
     "Culture",
     "Science",
     "Varied",
     "Pop Music",
     "Rock Music",
     "Easy Listening Music",
     "Light Classical",
     "Serious Classical",
     "Other Music",
     "Weather/Meteorology",
     "Finance/Business",
     "Children's programmes",
     "Social Affairs",
     "Religion",
     "Phone In",
     "Travel",
     "Leisure",
     "Jazz Music",
     "Country Music",
     "National Music",
     "Oldies Music",
     "Folk Music",
     "Documentary",
     "Not used",
     "Not used - Skip indicator"
};


/**
 * Create a string representation of the FAC.
 *
 * @param f FAC
 *
 * @return A string describtion of the content of the fac.
 */
char * describe_fac(dense_fac_t *f){

     int tmp;
     char *new, *old;
     char *t = NULL;

     asprintf_noret(&old,"%s","_____________oo00 FAC 00oo_____________\n");

     if ( f->base == 0)
          t = "Base layer";
     else
          t = "Enhanced layer";

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "Base",  f->base, t);
     free(old);
     old = new;


     t = NULL;
     if ( f->id == 0 )
          t = "First FAC block. AFS invalid";
     else if ( f->id == 1)
          t = "Second FAC block.";
     else if ( f->id == 2)
          t = "Third FAC block.";
     else if ( f->id == 3)
          t = "First FAC block. AFS valid";
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "ID",  f->id, t);
     free(old);
     old = new;


     t = f->rm_flag == 0 ? "DAB" : "DAB+";
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "Robustness Mode",
                    f->rm_flag, t);
     free(old);
     old = new;

     switch ( f->occ ){
     case 0:
          t = "100 ";
          break;
     default:
          t = "Illegal Value";
          break;
     }
     asprintf_noret(&new, "%s%-20s (%2hhx): %s kHz\n", old, "Spectrum",
                    f->occ, t);
     free(old);
     old = new;

     //depth
     t = f->depth == 0 ? "long" : "short";
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old, "Interl. Depth",
                    f->depth, t);
     free(old);
     old = new;

     switch ( f->msc_mode ) {
     case 0:
          t = "16, No hierarchical";
          break;
     case 1:
     case 2:
          t = "Illegal Value";
          break;
     case 3:
          t= "4, No hierarchical";
          break;
     default:
          t = "not recognized";
          break;
     }

     asprintf_noret(&new, "%s%-20s (%2hhd): QAM %s\n", old,"MSC Mode",
                    f->msc_mode, t);
     free(old);
     old = new;


     t = f->sdc_mode == 0 ? "1/2" : "1/4";
     asprintf_noret(&new, "%s%-20s (%2hhd): QAM 4 code rate: %s\n", old, "SDC Mode",
                    f->sdc_mode, t);
     free(old);
     old =new;

     //Note we do not mark reserved patterns.
     uint8_t a = f->services >> 2 & 0x03;
     uint8_t d = f->services      & 0x03;

     asprintf_noret(&new, "%s%-20s (%2hhd): %d Audio. %d Data.\n", old,
                    "Services", f->services,
                    a == 0 && d == 0 ? 4 : a,
                    a == 3 && d == 3 ? d : d);
     free(old);
     old = new;

     tmp = f->reconf_idx_2 << 2 | f->reconf_idx_0;
     if ( tmp == 0 )
          asprintf_noret(&new, "%s%-20s (%2hhd): Not signalled\n", old,
                         "Reconfiguration", tmp);
     else
          asprintf_noret(&new, "%s%-20s (%2hhd): In %d superframes\n",
                         old,  "Reconfiguration", tmp, tmp);
     free(old);
     old = new;

     t = f->rfu == 0 ? "Ok!" : "Should be zero";
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Reserved",
                    f->rfu, t);
     free(old);
     old = new;

     t = f->toggle_flag == 0 ? "Not signalled" : "Signalled";
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Toggle Flag",
    		 f->toggle_flag, t);
     free(old);
     old = new;

     asprintf_noret(&new,"%s_____________oo00 SERVICE %d 00oo_____________\n", old, f->short_id0);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (  ): 0x%01X%02X%02X%01X\n", old,
                    "Service Id",
                    f->service_id0_20, f->service_id0_12, f->service_id0_4,
                    f->service_id0_0);
     free(old);
     old = new;


     asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old,"Short ID",
                    f->short_id0, f->short_id0);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Audio CA",
                    f->audio_ca0, f->audio_ca0 ? "is used" : "is not used");
     free(old);
     old = new;

     tmp = f->lang0_3 << 3 | f->lang0_0;
     asprintf_noret(&new,"%s%-20s (%2hhd): %s\n", old,"Language", tmp,
                    languages[tmp]);
     free(old);
     old = new;

     asprintf_noret(&new,"%s%-20s (%2hhd): %s\n", old,"AD Flag",
                    f->audio_data0,
                    f->audio_data0 ? "Data" : "Audio");
     free(old);
     old = new;

     tmp = f->service_desc0_1 << 1 |f-> service_desc0_0;
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Programme Type",
                    tmp, prog_type[tmp]);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Data CA",
                    f->data_ca0, f->data_ca0 ? "used" : "not used");
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Reserved", f->rfa0,
                    f->rfa0 ? "Should be zero": "Ok!");
     free(old);
     old = new;

   if(f->short_id0 != f->short_id1) {
     asprintf_noret(&new,"%s_____________oo00 SERVICE %d 00oo_____________\n", old, f->short_id1);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (  ): 0x%02X%02X%02X\n", old,
                    "Service Id",
                    f->service_id1_16,f->service_id1_8,f->service_id1_0);
     free(old);
     old = new;


     asprintf_noret(&new, "%s%-20s (%2hhd): %d\n", old,"Short ID",
                    f->short_id1, f->short_id1);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Audio CA",
                    f->audio_ca1, f->audio_ca1 ? "is used" : "is not used");
     free(old);
     old = new;

     asprintf_noret(&new,"%s%-20s (%2hhd): %s\n", old,"Language", f->lang1,
                    languages[f->lang1]);
     free(old);
     old = new;

     asprintf_noret(&new,"%s%-20s (%2hhd): %s\n", old,"AD Flag",
                    f->audio_data1,
                    f->audio_data1 ? "Data" : "Audio");
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Programme Type",
                    f->service_desc1, prog_type[f->service_desc1]);
     free(old);
     old = new;

     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Data CA",
                    f->data_ca1, f->data_ca1 ? "used" : "not used");
     free(old);
     old = new;

     tmp= f->rfa1_4<<4|f->rfa1_0;
     asprintf_noret(&new, "%s%-20s (%2hhd): %s\n", old,"Reserved", tmp,
    		 	 	tmp ? "Should be zero": "Ok!");
     free(old);
     old = new;
   }

     //is checked by library
     //tmp = crc8sae_j1850_byte(0x00, (uint8_t *)f, sizeof(dense_fac_t) - 1);
     tmp=f->crc;

     asprintf_noret(&new, "%s%-20s (%2hhx): 0x%02x %s\n\n",
                    old, "CRC Sum",  f->crc,
                    tmp,
                    f->crc == tmp ? "Ok!":
                    "Error");
     free(old);

     return new;
}









void drm_show_fac(dense_fac_t *f)
{
    char* fac = describe_fac(f);
    fprintf(stderr, "%s",fac );
    free(fac);
}
