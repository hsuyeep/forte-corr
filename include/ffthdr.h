#ifndef _FFT_HDR_H_
#define _FFT_HDR_H_

typedef struct { char re[16],im[16] ; } 
ByteVectorType __attribute__ ((aligned(16))) ;

typedef struct { short re[8], im[8] ; } 
WordVectorType __attribute__ ((aligned(16))) ;

// Function declarations for help in FFT debugging
int print_xmac_simd_hadd_output (short *corrout, int chans, int nblines);
int print_do_fft64sse_output (void *outvec, int chans, int antquads);
int print_f64multi_output (WordVectorType *buf1, WordVectorType *buf2, FILE *fp);

int init_data (ByteVectorType *raw, WordVectorType *F);
int init_pulse (ByteVectorType *raw);
int print_raw_bytevector (ByteVectorType *raw, FILE *fp);
#endif // __FFT_HDR_H_
