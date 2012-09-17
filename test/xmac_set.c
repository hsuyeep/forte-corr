/* Test jig for xmac_set () testing.
 * pep/12Sep12
 */

#include "../include/corrops.h"
#include <strings.h>
#include <stdio.h>

int main ()
{ FftType finfo; 
  CorrOutType corr;

  bzero (&finfo, sizeof (FftType));

  xmac_fill_pattern (&finfo);
  fprintf (stderr, "# ---- XMAC input ----\n");
  fftw_set_print_fftout_file (&finfo, stderr);

  xmac_set (&finfo, &corr);

  fprintf (stderr, "# ---- XMAC output ----\n");
  xmac_set_print_reim_file (&corr, stderr);
  return ;
}
