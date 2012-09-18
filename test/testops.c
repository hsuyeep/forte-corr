/* Routines for carrying out testing of the ORT software correlator.
 * pep/18Sep12
 */

#include <stdio.h>
#include <math.h>
#include "../include/datalayout.h"
#include "../include/testops.h"


// Generates a Jaehne vector of max size 512 samples.
// The input is broadband.
int gen_frame_data_perant_jaehne (RawFrameType *frame, int antpair)
{ if (frame == NULL) return -1;

  int i = 0, j = 0, taps = 64, comp = 0; // Tuned for 64 tap FFT.
  float samp = 0;
  unsigned char nib_val = 0;
 
  // samp[n] = mag * sin ((0.5*pi*n*n)/len), 0<=n<len
  // fprintf (stderr, "# Antpair %d.\n",  antpair);
  for (j=0; j<Samps2Frame/Taps; j++)
  { for (i=0; i<Taps; i++)
    { samp = ((sin ((0.5*M_PI*i*i)/Taps) + 1)*8 - 1); // Remain between 0-15
      nib_val = ((int) samp)&15;

      #if 0
      // frame->data[i] = nib_val;      // Place in LSB nibble
         frame->data[i] = nib_val * 16; // Place in MSB nibble
      #endif 

      // Place in both nibbles, with a shift corresponding to the antenna
      // pair.
      frame->data[j*Taps + i + antpair] = (nib_val + nib_val*16) & 255;

      #if 0
      // Debug 
      fprintf (stderr, "%8.3f %3d %3d\n", samp, frame->data[i]&15, 
             ((frame->data[i]>>4)&15));
      #endif 
    }
  }
  // fprintf (stderr, "\n\n");

  return 0;
}

// Generates a data frame containing 1536 samples from a single antenna pair.
// The input is a sinusoidal sum containing as many spectral components as the
// antenna pair number.
int gen_frame_data_perant_sinusoid (RawFrameType *frame, int antpair)
{ if (frame == NULL) return -1;
  int i = 0, taps = 64, comp = 0; // Tuned for 64 tap FFT.
  float samp = 0;
  unsigned char nib_val = 0;
 
  // fprintf (stderr, "# Antpair %d.\n",  antpair);
  for (i=0; i<Samps2Frame; i++)
  { // Sum of spectral components in each stream.
    // for (samp=0, comp=1; comp < antpair; comp++) 
    //   samp += sin (comp*2*M_PI*(i+1)/taps);

    // Each antenna has a different frequency component.
    samp = (sin (antpair * 2*M_PI*i/taps) + 1.1)*8 - 1; // Remain between 0-15
                                         
    nib_val = ((int) samp)&15;
    // frame->data[i] = nib_val;      // Place in LSB nibble
#if 0
    frame->data[i] = nib_val * 16; // Place in MSB nibble
#endif 

    // Place in both nibbles
    frame->data[i] = (nib_val + nib_val*16) & 255;
#if 0
    // Debug 
    fprintf (stderr, "%8.3f %3d %3d\n", samp, frame->data[i]&15, 
             ((frame->data[i]>>4)&15));
#endif 
  }
  // fprintf (stderr, "\n\n");
  
  return 0;
}
