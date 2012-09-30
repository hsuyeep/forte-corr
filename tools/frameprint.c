/* Program to print the contents of frames read from file.
 * pep/23Aug12
 */

#include "../include/datalayout.h"
#include "../build/version.h"
#include <stdio.h>
#include <errno.h>

int main (int argc, char *argv [])
{ FILE *fraw = NULL;
  RawFrameType frame;
  int i = 0, got = 0;

  if (argc < 2)
  { fprintf (stderr, "Usage: %s rawfile.dat\n", argv[0]); return -1; }

  fprintf (stderr, "# ---- %s: Raw data print from file ----\n", argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  if ((fraw=fopen (argv[1], "rb")) == NULL)
  { perror ("fopen"); return -1; }

  // while (!feof (fraw))
  for (i=0; i<Frames2Set; i++)
  {  if ((got=fread (&frame, 1, sizeof (RawFrameType), fraw)) < 
          sizeof (RawFrameType))
     { fprintf (stderr, "Short read!\n"); break; }
  
     fprintf (stderr, "# Hdr: tick: %8d set: %5d frame: %8d.\n", frame.hdr.tick,
              frame.hdr.setid, frame.hdr.frameid);

     frame_print_contents (&frame, stderr);
  }

  if (fraw) fclose (fraw);

  return 0;
}
