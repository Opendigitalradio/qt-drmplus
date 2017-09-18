/*
 *    Copyright (C) 2014 .. 2017
 *    Sergiy (piratfm@gmail.com)
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *
 * This file is part of the QT-DRM+ distribution.
 * (https://github.com/Opendigitalradio/qt-drmplus).
 * Copyright (c) 2017 OpenDigitalRadio.
 *.
 * This program is free software: you can redistribute it and/or modify..
 * it under the terms of the GNU General Public License as published by..
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but.
 * WITHOUT ANY WARRANTY; without even the implied warranty of.
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU.
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



//
//	Common definitions and includes for
//	the DRM+ decoder

#ifndef	__DRM_CONSTANTS__
#define	__DRM_CONSTANTS__
#include	<QString>
#include	<math.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<complex>
#include	<limits>
#include	<cstring>
#include	<unistd.h>

//#if defined QWT_VERSION && ((QWT_VERSION >> 8) > 0x0601)
//#define	QT_STATIC_CONST
//#endif

#ifndef	__FREEBSD__
#include	<malloc.h>
#endif

#ifdef	__MINGW32__
//#include	"iostream.h"
#include	"windows.h"
#else
#ifndef	__FREEBSD__
#include	"alloca.h"
#endif
#include	"dlfcn.h"
typedef	void	*HINSTANCE;
#endif

typedef	float	DSPFLOAT;
typedef	std::complex<DSPFLOAT> DSPCOMPLEX;

using namespace std;
//
#define	Hz(x)		(x)
#define	Khz(x)		(x * 1000)
#define	KHz(x)		(x * 1000)
#define	kHz(x)		(x * 1000)
#define	Mhz(x)		(Khz (x) * 1000)
#define	MHz(x)		(KHz (x) * 1000)
#define	mHz(x)		(kHz (x) * 1000)

static inline
bool	isIndeterminate (DSPFLOAT x) {
	return x != x;
}

static inline
bool	isInfinite (double x) {
	return x == numeric_limits<DSPFLOAT>::infinity ();
}

static inline
DSPCOMPLEX cmul (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) * y, imag (x) * y);
}

static inline
DSPCOMPLEX cdiv (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) / y, imag (x) / y);
}

static inline
float	get_db (DSPFLOAT x) {
	return 20 * log10 ((x + 0.005) / (float)(256));
}
//
static	inline
DSPFLOAT	PI_Constrain (DSPFLOAT val) {
	if (0 <= val && val < 2 * M_PI)
	   return val;
	if (val >= 2 * M_PI)
	   return fmod (val, 2 * M_PI);
//	apparently val < 0
	if (val > - 2 * M_PI)
	   return val + 2 * M_PI;
	return 2 * M_PI - fmod (- val, 2 * M_PI);
}
/*
 */

#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

static inline
float	jan_abs (DSPCOMPLEX z) {
float	re	= real (z);
float	im	= imag (z);
	if (re < 0) re = - re;
	if (im < 0) im = - im;
	return re + im;
}

#endif //
