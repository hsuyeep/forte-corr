/* Header file for structures and functions specific to carrying out FFT and 
 * XMAC.
 * pep/23Aug12
 */

#ifndef __CORROPS_H__
#define __CORROPS_H__

#include <fftw3.h>
#include "../include/datalayout.h"

// Datastructure to hold FFT parameters.
typedef struct
{ int taps;
  int words2pkt;
  int blks2acc;
  float dnu;
  fftwf_complex *in, *out;
  short *setoutput;
  fftwf_plan p;
  WordVectorType fftout[Antennas*Vectors2Frame*ChanGroups];
} FftType;

// Function prototypes
int init_fftw   (FftType **);
int deinit_fftw (FftType *);
inline int fftw_shuffle (FftType *);
inline int shuffle_fill_pattern (FftType *);
inline int fftw_retrieve_pair (fftwf_complex *, short *, int );
// int fftw_fft (RawRegionPtrType *, int , FftType *);
inline int fftw_frame (unsigned char *, FftType *, short *);
inline int fftw_set_fft (unsigned char *, int , FftType *);
int fftw_set_print_fftout_file (FftType *, FILE *);
int fftw_set_dump_file (FftType *, FILE *);
int xmac_set (FftType *, CorrOutType *);
int xmac_set_print_file (CorrOutType *, FILE *);
int fftw_set_print_fftout_file (FftType *, FILE *);

#endif // __CORROPS_H__
