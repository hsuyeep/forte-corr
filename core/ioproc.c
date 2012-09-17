/* Process to create memory partitions of shared memory, and handle filling them
 * with raw data incoming over GigE.
 * pep/21Aug12
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"
#include "../include/netmgmt.h"

#define FILEREAD 1

int init_filedata (FILE **fp)
{ // Debug related
  char fname[32] = "testraw.dat";
  if ((*fp=fopen (fname, "rb")) == NULL)
  { perror ("fopen"); return -1; }
  // TODO: Check if enough data in test file is available for a region.
  return 0;
}

int init_net_data (struct StLinkInfoType *link0, struct StLinkInfoType *link1, unsigned char **set)
{ if (link0 == NULL || link1 == NULL) return -1;
  int i = 0, recvbytes = 0;
  RawFrameType frame;

  sprintf (link0->linkname, "%s", "192.168.1.221"); // names of data receivers.
  sprintf (link1->linkname, "%s", "192.168.1.222");

  if (init_data_link (link0) < 0) return -1; 
  if (init_data_link (link1) < 0) return -1; 

  if (bind_data_link (link0) < 0) return -1;
  if (bind_data_link (link1) < 0) return -1;

  if ((*set=calloc (Samps2Frame, Frames2Set)) == NULL)
  { perror ("calloc"); return -1; }

  // Debug
  for (i=0; i<100; i++)
  { if ((recvbytes=recvfrom (link0->sock, &frame, sizeof(RawFrameType), 0, NULL, NULL)) 
         == -1) 
    { perror("recvfrom"); break; }

    fprintf (stderr, "link0, pkt%d: tick: %d, frame:%d, set:%d.\n", 
             i, frame.hdr.tick, frame.hdr.frameid, frame.hdr.setid);
  
    if ((recvbytes=recvfrom (link1->sock, &frame, sizeof(RawFrameType), 0, 
        NULL, NULL)) == -1) 
    { perror("recvfrom"); break; }

    fprintf (stderr, "link1, pkt%d: tick: %d, frame:%d, set:%d.\n", 
             i, frame.hdr.tick, frame.hdr.frameid, frame.hdr.setid);
  }
   
  return 0;
}

int main (int argc, char *argv [])
{ ShmType partition[Partitions], *shm = NULL;
  RawRegionPtrType *regptr = NULL;
  int wr_reg = -1;
  int i = 0, j = 0;
  FILE *fp = NULL;           // Used for collecting data read from file.
  struct StLinkInfoType link0, link1; // Used for collecting data read from GigE.
  unsigned char *set = NULL; // External memory for collection of set.

  // Set all partition parameters to 0.
  bzero (partition, Partitions*sizeof (ShmType));

  fprintf (stderr, "# ---- IO handler ----\n");
  fprintf (stderr, "# Creating shared memory Partitions.\n");

  for (i=0; i<Partitions; i++)
  { sprintf (partition[i].path, "/part%d", i);
    if (create_partition (partition + i) < 0) 
    { fprintf (stderr, "### Error creating partition %d! Quitting...\n", i);
      for (j=0; j<i; j++) destroy_partition (partition + j);
      return -1;
    }
  }
 
/*  // For Debug
  for (i=0; i<Partitions; i++)
  { fprintf (stderr, " --- Partition %d\n", i);
    registry_print (partition+i, stderr);
  }
*/
 
#if FILEREAD 
  if (init_filedata (&fp) < 0)
  { fprintf (stderr, " Unable to initialize file related structures!\n"); 
    // Tear down partitions
    for (i=0; i<Partitions; i++)
     if (destroy_partition (partition+i) < 0)
    { fprintf (stderr, "### Error destroying partition %d! Quitting...\n", i); }
    return -1;
  }
#else
  if (init_net_data (&link0, &link1, &set) < 0)
  { fprintf (stderr, " Unable to initialize network related structures!\n"); 
    // Tear down partitions
    for (i=0; i<Partitions; i++)
     if (destroy_partition (partition+i) < 0)
    { fprintf (stderr, "### Error destroying partition %d! Quitting...\n", i); }
    return -1;
  }
#endif


  shm = partition;
  while (!MemDone)
  { while (1)
    { wr_reg = find_wr_region (shm, NonBlock);
      if (wr_reg != -1)
      { fprintf (stderr, "--> Region %d Writable. Status: %s\n", wr_reg,
                 RegionStatus[shm->registry->region[wr_reg].status]);
        break;
      }
      else
      { fprintf (stderr, "No Writable region found in non-blocking search.\n");
        // registry_print (shm, stderr);
        sleep (1);
        if (MemDone == 1) break;
      }
    }
    if (MemDone == 1) break;

    // Fill a given region with data from file, which is assumed to hold a 
    // single STA worth of data.
    shm = partition;
    regptr = shm->regptr;
    for (j=0; j<STA2Region; j++)
#if FILEREAD
     region_fill_STA_file (regptr + wr_reg, j, fp); // Use with file
    rewind (fp);
#else
       region_fill_STA_net (regptr + wr_reg, j, &link0, &link1, set);
#endif 
    
    // print all hdrs in region (only for debug!).
    // region_print_frame_meta (regptr + wr_reg, stderr); 

    if (registry_change_region_status (shm->registry, wr_reg, Full) < 0)
    { fprintf (stderr, "### Unable to change registry status!\n"); } 
    // After filling the region, the appropriate FX proc. is signalled.
    shm->registry->avail_reg = wr_reg;
    fprintf (stderr, "-->    Region %d written. Status: %s\n", wr_reg,
                 RegionStatus[shm->registry->region[wr_reg].status]);

    // registry_print (shm, stderr);
  }

#if FILEREAD
  if (fp) fclose (fp);
#else
  // Close network connections
  if (close_data_link (&link0) < 0)
  { fprintf (stderr, "### Error in closing data link %s!\n", link0.linkname);}
  if (close_data_link (&link1) < 0)
  { fprintf (stderr, "### Error in closing data link %s!\n", link1.linkname);}
#endif 

  if (set) free (set);

  // Print final memory statistics.
  // TODO

  // Tear down partitions
  for (i=0; i<Partitions; i++)
   if (destroy_partition (partition+i) < 0)
  { fprintf (stderr, "### Error destroying partition %d! Quitting...\n", i); }

  return 0;
}
