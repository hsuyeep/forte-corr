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
#include "../include/testops.h"


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
      gen_frame_data_perant_jaehne (&frame, i); 
      // read (fd, &frame.data, Samps2Frame); // For random data.
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
