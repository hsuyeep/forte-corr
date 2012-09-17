/* Program to generate sets of data worth a single STA, in the raw data format.
 * pep/21Aug12
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "../include/memmgmt.h"
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

int main (int argc, char *argv[])
{ RawFrameType frame;
  FrameHdrType *hdr = &frame.hdr;
  FILE *fp = NULL;
  unsigned int framecnt = 0;

  int fd = open ("/dev/random", O_RDONLY);
  if (fd < 0) 
  { perror ("open random"); return -1; }

  if ((fp=fopen ("testraw.dat", "wb")) == NULL)
  { perror ("fopen"); return -1; }

  int i = 0, j = 0;

  for (i=0; i<6; i++) hdr->dummy[i] = i;
  for (j=0; j<Sets2STA; j++)
  { for (i=0; i<Frames2Set; i++)
    { hdr->tick = framecnt; 
      hdr->setid = i;

      // For generating a sinusoid of an increased frequency for each antenna.
      // gen_frame_data_perant_sinusoid (&frame, i); 

      // For generating a broadband source per antenna, delayed by one sample.
      gen_frame_data_perant_jaehne (&frame, i); 

      // For random data.
      // read (fd, &frame.data, Samps2Frame); 

      fwrite (&frame, 1, sizeof (RawFrameType), fp);
      framecnt++;
    }
    // fprintf (stderr, ".");
    fprintf (stderr, "%5d %5d\n.", hdr->tick, hdr->setid);
  }
  
  fprintf (stderr, "# Wrote %d frames\n", framecnt);

  close (fd);
  if (fp) fclose (fp);
  return 0;
}
