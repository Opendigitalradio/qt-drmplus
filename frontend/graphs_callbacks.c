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


#include <string.h>
#include <math.h>

#include "drmplus.h"
#include "gnuplot_i.h"

#ifndef fft_complex
typedef double fft_complex[2];
#endif

gnuplot_ctrl *sp_plot = NULL;
void spectrum_update(void *priv_data, void *spectrum_p, int callbackType, void *additionalInfo)
{
	double *spectrum = (double *) spectrum_p;
	if(!spectrum || callbackType != SIGSP_CALLBACK)
		return;

    if(!sp_plot) {
    	sp_plot = gnuplot_init();
    	gnuplot_setstyle(sp_plot, "lines") ;
    } else {
    	gnuplot_resetplot(sp_plot) ;
    }

    gnuplot_plot_x(sp_plot, spectrum, 432, "Spectrum");
    usleep(250000);

//	fprintf(stderr, "spectrum callback\n");
}



gnuplot_ctrl *iq_plot = NULL;
void iq_update(void *priv_data, void *iq_p, int callbackType, void *additionalInfo)
{
	fft_complex *iq_data = (fft_complex *) iq_p;
	uint8_t *markers = (uint8_t *) additionalInfo;
	//NOTE: markers[213] containing symbol ID number!
	if(!iq_data || !markers || callbackType != SIGIQ_CALLBACK)
		return;

    if(!iq_plot) {
    	iq_plot = gnuplot_init();
    	//gnuplot_setstyle(iq_plot, "dots") ;
    } else {
    	gnuplot_resetplot(iq_plot) ;
    }

    double data_i[213];
    double data_q[213];

    double facsdc_i[213];
    double facsdc_q[213];

    double pil_i[213];
    double pil_q[213];

    double pil2x_i[2];
    double pil2x_q[2];

    int i;

    int num_data = 0;
    int num_facsdc = 0;
    int num_pil = 0;
    int num_pil2x = 0;

    for(i=0;i<213;i++) {

    	switch(markers[i]) {
    	case CELL_TYPE_PIL:
    	case CELL_TYPE_AFS:
    		pil_i[num_pil] = iq_data[i][0];
    		pil_q[num_pil] = iq_data[i][1];
    		num_pil++;
    		break;
    	case CELL_TYPE_PIL2X:
    		pil2x_i[num_pil2x] = iq_data[i][0];
    		pil2x_q[num_pil2x] = iq_data[i][1];
    		num_pil2x++;
    		break;
    	case CELL_TYPE_FAC:
    	case CELL_TYPE_SDC:
    		facsdc_i[num_facsdc] = iq_data[i][0];
    		facsdc_q[num_facsdc] = iq_data[i][1];
    		num_facsdc++;
    		break;
    	default:
        	data_i[num_data] = iq_data[i][0];
        	data_q[num_data] = iq_data[i][1];
        	num_data++;
    		break;
    	}
    }
    //hack to display
    if(!num_facsdc) {facsdc_i[num_facsdc]=0;facsdc_q[num_facsdc]=0;num_facsdc++;}
    if(!num_pil2x) {pil2x_i[num_pil2x]=0;pil2x_q[num_pil2x]=0;num_pil2x++;}

    if(num_data)
    	gnuplot_plot_xy(iq_plot, data_i, data_q, num_data, "MSC");
    if(num_pil)
    	gnuplot_plot_xy(iq_plot, pil_i, pil_q, num_pil, "pil");
    if(num_pil2x)
    	gnuplot_plot_xy(iq_plot, pil2x_i, pil2x_q, num_pil2x, "pil2x");
    if(num_facsdc)
    	gnuplot_plot_xy(iq_plot, facsdc_i, facsdc_q, num_facsdc, "FAC");
    usleep(250000);

}


gnuplot_ctrl *equalizer_plot = NULL;
void equalizer_update(void *priv_data, void *eq_p, int callbackType, void *additionalInfo)
{
	fft_complex *eq_data = (fft_complex *) eq_p;
	if(!eq_data|| callbackType != SIGEQ_CALLBACK)
		return;

    if(!equalizer_plot) {
    	equalizer_plot = gnuplot_init();
    	gnuplot_setstyle(equalizer_plot, "lines") ;
    } else {
    	gnuplot_resetplot(equalizer_plot) ;
    }

    double energies[213];
    double phases[213];

    int i;
    for(i=0;i<213;i++) {
    	energies[i]=sqrt(eq_data[i][0]*eq_data[i][0] + eq_data[i][1]*eq_data[i][1]);
    	phases[i] = atan2(eq_data[i][1], eq_data[i][0]) * energies[0] + energies[0];
    }
    gnuplot_plot_x(equalizer_plot, energies, 213, "energy");
    gnuplot_plot_x(equalizer_plot, phases, 213, "phase");
    usleep(250000);

}
