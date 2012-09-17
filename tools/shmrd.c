/* Process to collect data over shared memory via region reads, and print
 * the headers of read frames, as well as the registry. Used for shm debug,
 * and shm throughput tests.
 * See include/memmgm.h and ioproc.c for terminology.
 * pep/21Aug12
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"

int main (int argc, char *argv [])
{ ShmType partition, *shm = NULL;
  RawRegionPtrType *regptr = NULL;
  int rd_reg = -1;
  
  // Set all partition parameters to 0.
  bzero (&partition, sizeof (ShmType));

  fprintf (stderr, "# ---- Shared Memory Reader ----\n");
  fprintf (stderr, "# Attaching to shared memory region.\n");

  memcpy (partition.path, "/part0", 16); // Currently hardcoded path and size.

  if (create_partition (&partition) < 0) 
  { fprintf (stderr, "### Error creating partition! Quitting...\n"); }

  shm = &partition;
  regptr = shm->regptr;
  while (!MemDone)
  { // Find a region to read.
    while (1)
    // { rd_reg = find_rd_region (shm, NonBlock);
    { rd_reg = find_rd_region (shm, Block);
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

    // Simulate reading.
    usleep (10);
    region_print_frame_meta (shm->regptr + rd_reg, stdout);
    if (registry_change_region_status (shm->registry, rd_reg, Empty) < 0)
    { fprintf (stderr, "### Unable to change registry status!\n"); } 

    // registry_print (shm, stderr);
  }

  
#if 0
  //  Write out contents
  fprintf (stderr, "%s", (char*)shm.ptr);
  fprintf (stderr, "Sending message to ioproc.c..\n");
  ((char*)shm.ptr)[100] = 0x24;
#endif 

  if (destroy_partition (shm) < 0)
  { fprintf (stderr, "### Error destroying partition! Quitting...\n"); }
  return 0;
}
