/* Test jig for xmac_set () testing.
 * pep/12Sep12
 */

#include "../include/corrops.h"
#include "../build/version.h"
#include <strings.h>
#include <stdio.h>

int main (int argc, char *argv [])
{ FftType finfo; 
  CorrOutType corr;

  bzero (&finfo, sizeof (FftType));

  fprintf (stderr, "# ---- %s: XMAC test ----\n", argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  xmac_fill_pattern (&finfo);
  fprintf (stderr, "# ---- XMAC input ----\n");
  print_fftout_file (&finfo, stderr);

  xmac_set (&finfo, &corr);

  fprintf (stderr, "# ---- XMAC output ----\n");
  print_xmac_reim_file (&corr, stderr);
  return ;
}
