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
#include "../build/version.h"

int main (int argc, char *argv [])
{ ShmType partition, *shm = NULL;
  ShmType ctrlpartition;
  RawRegionPtrType *regptr = NULL;
  CtrlPartitionInfoType  *ctrlinfo = NULL;
  int rd_reg = -1, i = 0, ctrloff = -1;
  
  fprintf (stderr, "# ---- %s: Shared memory reader and frame header printer ----\n", argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  // Attach to control partition: Assume it is already populated by writer.
  // NOTE: put in action for when the writer has not been run first.
  bzero (&ctrlpartition, sizeof (ShmType));
  fprintf (stderr, "# Attaching to shared memory CONTROL area.\n");
  memcpy (ctrlpartition.path, "/part0", 16); // Control area has hardcoded path.
  ctrlpartition.size = sizeof (CtrlPartitionInfoType);

  if (attach_partition (&ctrlpartition) < 0) 
  { fprintf (stderr, "### Error creating CONTROL partition! Quitting...\n"); }

  // Figure out which partition to attach to, update control region.
  ctrlinfo = (CtrlPartitionInfoType*) (ctrlpartition.ptr);
  fprintf (stderr, " Control Partition information.\n");
  for (i=0; i<ctrlinfo->rinfo[0].total_partitions; i++)
  { fprintf (stderr, "--> Information for partition %d\n", i);
    fprintf (stderr, "   -- Total partitions    :%d\n", 
             ctrlinfo->rinfo[i].total_partitions);
    fprintf (stderr, "   -- Port name           :%s\n", 
             ctrlinfo->rinfo[i].portname);
    fprintf (stderr, "   -- Partition path      :%s\n", 
             ctrlinfo->rinfo[i].partition_path);

    fprintf (stderr, "\n   -- Partition claimed   :%d\n", 
             ctrlinfo->rinfo[i].claimed);
    fprintf (stderr, "   -- Output linkname     :%s\n\n", 
             ctrlinfo->rinfo[i].linkname);
    if (ctrlinfo->rinfo[i].claimed == 0)
    { sem_wait (&ctrlinfo->sem); // Blocked wait on control region access.
      ctrlinfo->rinfo[i].claimed = 1;
      ctrlinfo->currently_active ++;
      sem_post (&ctrlinfo->sem);    // Unlock semaphore
      strncpy (ctrlinfo->rinfo[i].linkname, "127.0.0.1", 16);
      // strncpy (outlink.linkname, ctrlinfo->rinfo[i].linkname, 16);
      // strncpy (outlink.portname, ctrlinfo->rinfo[i].portname, 16);
      // NOTE: This is the ONLY variable which stores offset in ctrl region! 
      // Guard carefully!!
      ctrloff = i; 
      fprintf (stderr, "--> Claiming partition %d.\n", ctrloff);
      break;
    }
  }

  fprintf (stderr, "# Attaching to shared memory region.\n");
  // memcpy (partition.path, "/part0", 16); // Currently hardcoded path and size.
  memcpy (partition.path, ctrlinfo->rinfo[ctrloff].partition_path, 16); 

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
