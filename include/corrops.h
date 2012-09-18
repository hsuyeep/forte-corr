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
inline int shuffle_fft_antpair (FftType *);
inline int print_shuffle_fft_antpair (FftType *, FILE *);
inline int shuffle_fft (FftType *);
inline int shuffle_fill_pattern (FftType *);
inline int retrieve_pair_fft (fftwf_complex *, short *, int );
// int fftw_fft (RawRegionPtrType *, int , FftType *);
inline int fftw_frame (unsigned char *, FftType *, short *);
inline int fftw_set_fft (unsigned char *, int , FftType *);
int dump_setoutput_file (FftType *, FILE *);
int print_setoutput_file (FftType *, FILE *);
int print_fftout_file (FftType *, FILE *);
inline int xmac_fill_pattern (FftType *);
int xmac_set (FftType *, CorrOutType *);
int print_xmac_reim_file (CorrOutType *, FILE *);
int print_xmac_ampph_file (CorrOutType *, FILE *, int );
int dump_xmac_reim_file (CorrOutType *, FILE *);
int gen_bin_fname (char *, int len);

#endif // __CORROPS_H__
