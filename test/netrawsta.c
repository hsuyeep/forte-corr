/* Program to generate sets of data worth corresponging to 40 antenna
 * elements, and send 20 antenna elements over a single GigE link.
 *   The program can be run on a separate node to simulate the bridge card
 * GigE interfaces. NOTE that two GigE links are used to send all data.
 * pep/22Aug12
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include "../include/datalayout.h"
#include "../include/netmgmt.h"
#include "../include/memmgmt.h"

int Done = 0;
void sig_hdlr (int dummy)
{ fprintf (stderr, "netrawsta: Caught signal.\n"); Done = 1; }

int main (int argc, char *argv[])
{ unsigned int timetick = 0; // Masquerades as the hardware tick.
  unsigned int framecnt = 0;
  short setid = 0;
  int i = 0, j = 0;
  int sentbytes = 0;
  RawFrameType *set = NULL;
  LinkInfoType link0, link1;
  struct timeval start, end;
  float elapse = 0;
  int nset = 0;

  int fd = open ("/dev/random", O_RDONLY);
  if (fd < 0) 
  { perror ("open random"); return -1; }

  // Open the two outgoing ethernet links. NOTE: If a link does not work,
  // data corresponding to that link is not sent.
  sprintf (link0.linkname, "%s", "192.168.1.221");
  sprintf (link1.linkname, "%s", "192.168.1.222");

  fprintf (stderr, "# Setting up connections to hosts %s, %s.\n",
           link0.linkname, link1.linkname);
  if (init_data_link (&link0) < 0)
  { fprintf (stderr, "# Unable to send datagrams to %s.\n", link0.linkname); }
  if (init_data_link (&link1) < 0)
  { fprintf (stderr, "# Unable to send datagrams to %s.\n", link0.linkname); }

  // Create memory for a full set of frames
  if ((set=(RawFrameType*) calloc (Frames2Set, Samps2Frame)) == NULL)
  { perror ("calloc"); return -1; }

  signal (SIGINT, sig_hdlr);

  gettimeofday (&start, NULL);
  while (!Done)
  { // Generate appropriate headers for all frames
    // and fill frames with data.
    for (i=0; i<Frames2Set; i++)
    { for (j=0; j<6; j++) set[i].hdr.dummy[j] = j;
      set[i].hdr.tick = timetick;
      set[i].hdr.frameid = i;
      set[i].hdr.setid = setid;
      // read (fd, &set[i].data, Samps2Frame); // Random payload
    }
    timetick++;
    setid++;
  
    // Send out appropriate half sets on individual links.
    for (i=0; i<Frames2Set/2; i++)
    { if ((sentbytes = sendto (link0.sock, &set[i], sizeof(RawFrameType), 
           0, link0.servinfo->ai_addr, link0.servinfo->ai_addrlen)) == -1) 
      { perror("sendto link0"); }
  
      if ((sentbytes = sendto (link1.sock, &set[i+Frames2Set/2], sizeof(RawFrameType), 
           0, link1.servinfo->ai_addr, link1.servinfo->ai_addrlen)) == -1) 
      { perror("sendto link1"); }
    }
    // print_frame_meta (set, stderr);
    // print_frame_meta (set+Frames2Set/2, stderr);
    nset++;
    usleep (100);
    if (nset % 100 == 99) fprintf (stderr, ".");
  }
  gettimeofday (&end, NULL);
  elapse = (end.tv_sec+end.tv_usec/1e6) - (start.tv_sec+start.tv_usec/1e6);
  fprintf (stdout, "Generated %8d sets. (Total %lf MB)\n", 
           nset, (nset*(Samps2Frame*Frames2Set)/1048576.)); 
  fprintf (stdout, "Total time taken: %8.3f secs.\n", elapse);
  fprintf (stdout, "Datarate: %8.4lf MBps, Setrate: %8.4lf sets per sec.\n", 
         (double)(nset*((Samps2Frame*Frames2Set)/1048576.))/elapse, 
         (double)(nset)/elapse); 

  if (set) free (set);
  close (fd);
  return 0;
}
