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


#ifndef GRAPHS_CALLBACKS_H_
#define GRAPHS_CALLBACKS_H_

void spectrum_update(void *priv_data, void *spectrum_p, int callbackType, void *additionalInfo);
void iq_update(void *priv_data, void *iq_p, int callbackType, void *additionalInfo);
void equalizer_update(void *priv_data, void *eq_p, int callbackType, void *additionalInfo);


#endif /* GRAPHS_CALLBACKS_H_ */
