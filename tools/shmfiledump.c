/* Process to collect data over shared memory and dump regenerated raw packets 
 * to disk. Apart from allowing operations on rawdata, it also allows checking 
 * of shared memory interaction between writer and reader by intercomparing the 
 * generated raw data packets.
 *   OTHER IDEAS:
 *      - Maybe we can specify in advance data corresponding to which times 
 *        needs to be dumped, or to dump periodically, or specify the number
 *        of regions to dump.
 *      - Dumped filename can be automatically generated based on specified 
 *        parameters.
 * pep/22Aug12
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "../include/memmgmt.h"
#include "../build/version.h"

int main (int argc, char *argv [])
{ ShmType partition, *shm = NULL;
  RawRegionPtrType *regptr = NULL;
  int i = 0, j = 0, rd_reg = -1;
  FILE *fraw = NULL;
  char rawfname[32] = "dump.raw";
  int dumpregions = 1;
  
  fprintf (stderr, "# ---- %s: Dump shared memory contents to disk ----\n", 
           argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  if ((fraw=fopen (rawfname, "wb")) == NULL)
  { perror ("fopen"); return -1; }

  // Set all partition parameters to 0.
  bzero (&partition, sizeof (ShmType));

  fprintf (stderr, "# Attaching to shared memory region.\n");
  memcpy (partition.path, "/part0", 16); // Currently hardcoded path and size.

  if (create_partition (&partition) < 0) 
  { fprintf (stderr, "### Error creating partition! Quitting...\n"); }

  shm = &partition;
  regptr = shm->regptr;
  for (i=0; i<dumpregions; i++)
  { // Find a region to read.
    while (1)
    // { rd_reg = find_rd_region (shm, NonBlock);
    { rd_reg = find_rd_region (shm, NonBlock);
      if (rd_reg != -1)
      { fprintf (stderr, "<-- Region %d Readable. Status: %s\n", rd_reg,
                 RegionStatus[shm->registry->region[rd_reg].status]);
        break;
      }
      else
      { fprintf (stderr, "No Readable region found in non-blocking search.\n");
        registry_print (shm, stderr);
        sleep (1);
        if (MemDone == 1) break;
      }
    }

    // Start dumping data to disk.
    // NOTE: Can either indicate region to be empty on reading, or can leave it 
    // with status Full, in  order to allow a coupled process to operate on the
    // data. This requires us to maintain a record of which regions have been 
    // already read, in order to avoid writing the same data to disk twice.
    // CURRENTLY UNIMPLEMENTED.
    region_write_to_file (shm->regptr + rd_reg, fraw);
    if (registry_change_region_status (shm->registry, rd_reg, Empty) < 0)
    { fprintf (stderr, "### Unable to change registry status!\n"); } 

    // registry_print (shm, stderr);
  }
  fprintf (stderr, "# %d Regions dumped to file %s.\n", dumpregions, rawfname);

  if (destroy_partition (shm) < 0)
  { fprintf (stderr, "### Error destroying partition! Quitting...\n"); }
  return 0;
}
