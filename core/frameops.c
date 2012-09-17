/* Functions to operate on frames of data, which can be raw or processed.
 * pep/23Aug12
 */

#include <stdio.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"


int print_frame_meta (RawFrameType *frame, FILE *fp)
{ if (frame == NULL) return -1;
  fprintf (fp, "Frame tick: %d, frame: %d, setid: %d.\n", frame->hdr.tick,
           frame->hdr.frameid, frame->hdr.setid); 
  return 0;
}

int frame_print_contents (RawFrameType *frame, FILE *fp)
{ if (frame == NULL || fp == NULL) return -1;
  int i = 0;
 
  for (i=0; i<Samps2Frame; i++) // Assuming 4-bit per sample.
    fprintf (fp, "%2d %2d\n", (frame->data[i]>>4)&15, frame->data[i]&15);

  fprintf (fp, "\n\n");
  
  return 0; 
}

int list_missing_frames ()
{
  return 0;
}

int check_frame_data_integrity ()
{
  return 0;
}
