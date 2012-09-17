/* Program to apply a C-based correlator on data from a shared memory region.
 * pep/23Aug12
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"
#include "../include/corrops.h"

int main (int argc, char *argv [])
{ ShmType partition, *shm = NULL;
  RawRegionPtrType *regptr = NULL;
  FftType *fftinfo = NULL;
  CorrOutType corr;
  int i = 0, j = 0, rd_reg = -1;
  FILE *fftwordfile = NULL;
  unsigned int setsize = Samps2Frame * Frames2Set; // Bytes.
  unsigned int stasize = setsize * Sets2STA;       // Bytes.

  if ((fftwordfile=fopen ("set_fft_wordvector.bin", "wb")) == NULL)
  { perror ("fopen"); return -1; }

  // Get FFTW initialized
  if (init_fftw (&fftinfo) < 0)
  { fprintf (stderr, "init_fftw failed!\n"); return -1; }

  // Set all partition parameters to 0.
  bzero (&partition, sizeof (ShmType));

  fprintf (stderr, "# ---- FFTW Based Correlator ----\n");
  fprintf (stderr, "# Attaching to shared memory region.\n");

  memcpy (partition.path, "/part0", 16); // Currently hardcoded path and size.

  if (create_partition (&partition) < 0) 
  { fprintf (stderr, "### Error creating partition! Quitting...\n"); }

  shm = &partition;
  regptr = shm->regptr;
  while (!MemDone)
  { // Find a region to read.
    while (1)
    { // rd_reg = find_rd_region (shm, NonBlock);
      rd_reg = find_rd_region (shm, Block);
      if (rd_reg != -1)
      { fprintf (stderr, "<-- Region %d Readable. Status: %s\n", rd_reg,
                 RegionStatus[shm->registry->region[rd_reg].status]);
        break;
      }
      else
      { fprintf (stderr, "No Readable region found in non-blocking search.\n");
        sleep (1);
        if (MemDone == 1) break;
      }
    }

    // Found a valid region! Carry out correlation of every STA making up the 
    // region. 1 Region has STA2Region * Sets2STA sets. Each set has 
    // Frames2Set frames. FFT and correlation currently occurs on a single set.
    // for (i=0; i<STA2Region; i++) // Process every STA within region.
    { 
      unsigned char *set = shm->regptr[rd_reg].data + i*stasize; 

      // for (j=0; j<Sets2STA; j+=Frames2Set) // Process every set within STA.
      { fftw_set_fft (set + j*setsize, Frames2Set, fftinfo); // FFT of one set.

        // fftw_set_print_fftout_file (fftinfo, stderr); // ASCII dump of a set.
        // fftw_set_dump_file (fftinfo, fftwordfile);    // BLOB dump to file.

        xmac_set (fftinfo, &corr);         // XMAC within a set, output in 
                                           // corr.
        // xmac_set_print_reim_file (&corr, stdout);// print for debug.
        xmac_set_print_ampph_file (&corr, stdout, 1);// print for debug.
      }
    }
  }
  
  if (destroy_partition (shm) < 0)
  { fprintf (stderr, "### Error destroying partition! Quitting...\n"); }

  if (deinit_fftw (fftinfo) < 0)
  { fprintf (stderr, "deinit_fftw failed!\n"); }

  if (fftwordfile) fclose (fftwordfile);
  return 0;
}
