/* Program to check the fftw_shuffle () function, by feeding it a known pattern.
 * pep/12Sep12
 */

#include <stdio.h>
#include "../include/corrops.h"
#include "../build/version.h"

int main (int argc, char *argv [])
{ FftType *finfo;
  
  fprintf (stderr, "# ---- %s: Test FFT shuffle output ----\n", argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  init_fftw (&finfo);

  shuffle_fill_pattern (finfo);
  fprintf (stderr, "\n\n# ---- Filled data pattern Shuffled FFT output ----\n");
  print_setoutput_file (finfo, stderr);
  shuffle_fft (finfo);

  fprintf (stderr, "\n\n# ---- Shuffled FFT output ----\n");
  print_fftout_file (finfo, stderr);

  deinit_fftw (finfo);
  return 0;
}
