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


#ifndef DRMPLUSDEMOD_H_
#define DRMPLUSDEMOD_H_

#include "drmplus.h"
#include "fac.h"

void asprintf_noret(char **strp, const char *fmt, ...);

void drm_show_fac(dense_fac_t *f);
void drm_show_sdc(uint8_t *f, int size);

#endif /* DRMPLUSDEMOD_H_ */
