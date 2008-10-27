/* -*- c++ -*- */
/*
 * Copyright 2008 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the tewavelet of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdexcept>
#include <gr_wavelet_ff.h>
#include <gr_io_signature.h>

#include <stdio.h>

// NB in this version, only Daubechies wavelets
// order is wavelet length, even, 2...20

gr_wavelet_ff_sptr
gr_make_wavelet_ff(int size,
		   int order,
		   bool forward)
{
  return gr_wavelet_ff_sptr(new gr_wavelet_ff(size,
					      order,
					      forward));
}

gr_wavelet_ff::gr_wavelet_ff(int size,
			     int order,
			     bool forward)
  : gr_sync_block("wavelet_ff",
		  gr_make_io_signature(1, 1, size * sizeof(float)),
		  gr_make_io_signature(1, 1, size * sizeof(float))),
    d_size(size),
    d_order(order),
    d_forward(forward)
{
  d_wavelet = gsl_wavelet_alloc(gsl_wavelet_daubechies, d_order);
  if (d_wavelet == NULL)
    throw std::runtime_error("can't allocate wavelet");
  d_workspace = gsl_wavelet_workspace_alloc(d_size);
  if (d_workspace == NULL)
    throw std::runtime_error("can't allocate wavelet workspace");
  d_temp = (double *) malloc(d_size*sizeof(double));
  if (d_workspace == NULL)
    throw std::runtime_error("can't allocate wavelet double conversion temp");
}

gr_wavelet_ff::~gr_wavelet_ff()
{
  gsl_wavelet_free(d_wavelet);
  gsl_wavelet_workspace_free(d_workspace);
  free((char *) d_temp);
}

int
gr_wavelet_ff::work(int noutput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items)
{
  const float *in  = (const float *) input_items[0];
  float       *out = (float *) output_items[0];

  for (int count = 0; count < noutput_items; count++) {
    for (int i = 0; i < d_size; i++)
      d_temp[i] = in[i];
    
    if (d_forward)
      gsl_wavelet_transform_forward(d_wavelet,
				    d_temp,
				    1,
				    d_size,
				    d_workspace);
    else
      gsl_wavelet_transform_inverse(d_wavelet,
				    d_temp,
				    1,
				    d_size,
				    d_workspace);
    
    for (int i = 0; i < d_size; i++)
      out[i] = d_temp[i];

    in  += d_size;
    out += d_size;
  }

  return noutput_items;
}
