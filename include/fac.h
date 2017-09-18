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
 *
 *
 *  borrowed from embedded-drm-radio
 */

#ifndef FAC_H_
#define FAC_H_


typedef struct dense_fac_s {
     //Channel parameters
     // byte 1
     uint8_t depth         : 1; /**< Interleaver depth. LSB */
     uint8_t occ           : 3; /**< Spectrum occupancy */
     uint8_t rm_flag       : 1; /**< Robustness mode flag. */
     uint8_t id            : 2; /**< FAC ID (Offset in superframe) */
     uint8_t base          : 1; /**< Enhancment flag. MSB */

     //byte 2
     uint8_t reconf_idx_2  : 1; /**< Reconfiguration index bit 2. LSB */
     uint8_t services      : 4; /**< Number of audio and data  services*/
     uint8_t sdc_mode      : 1; /**< SDC mode */
     uint8_t msc_mode      : 2; /**< MSC mode. MSB */

     //byte 3
     uint8_t service_id0_20 : 4; /**< Service ID bit 23-20. LSB */
     uint8_t rfu           : 1; /**< Reserved. Should be 0. */
     uint8_t toggle_flag   : 1; /**< Toggle flag: setted to 0 in frame 0,2 and to 1 in frame 1,3. */
     uint8_t reconf_idx_0  : 2; /**< Refconfiguration index bit 1,0. MSB */

     //byte 4 and 5
     uint8_t service_id0_12; /**< Service ID bit 19-12. */
     uint8_t service_id0_4; /**< Service ID bit 11-4. */

     //byte 6
     uint8_t lang0_3        : 1; /**< Language bit 3. LSB */
     uint8_t audio_ca0      : 1; /**< Conditional Access.*/
     uint8_t short_id0      : 2; /**< Short id */
     uint8_t service_id0_0  : 4; /**< Service ID bit 3-0. MSB */


     //byte 7
     uint8_t service_desc0_1: 4; /**< Service descriptor. Bit 5-2. LSB */
     uint8_t audio_data0    : 1; /**< Audio or data? */
     uint8_t lang0_0        : 3; /**< Language. Bit 2-0. MSB */

     //byte 8
     uint8_t rfa0           : 6; /**< Reserved. LSB */
     uint8_t data_ca0       : 1; /**< Conditional Access */
     uint8_t service_desc0_0: 1; /**< Service descriptor bit 0. MSB */

     //byte 9,10,11
     uint8_t service_id1_16; /**< Service ID1 bit 16-24. */
     uint8_t service_id1_8; /**< Service ID1. bit 8-15.*/
     uint8_t service_id1_0; /**< Service ID1. bit 0-7. */


     //byte 12
     uint8_t audio_data1    : 1; /**< Audio or data? */
     uint8_t lang1          : 4; /**< Language */
     uint8_t audio_ca1      : 1; /**< Conditional Access.*/
     uint8_t short_id1      : 2; /**< Short id */

     //byte 13
     uint8_t rfa1_4         : 2; /**< Reserved. LSB */
     uint8_t data_ca1       : 1; /**< Conditional Access */
     uint8_t service_desc1  : 5; /**< Service descriptor. Bit 5-2. LSB */

     //byte 14
     uint8_t offset         : 4; /**< Reserved */
     uint8_t rfa1_0         : 4; /**< Reserved. LSB */

     //byte 15
     uint8_t crc; /**< CRC value. Post inverted. */
} __attribute__((__packed__)) dense_fac_t;


#endif /* FAC_H_ */
