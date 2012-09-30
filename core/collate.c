/* Process to collate processed data over shared memory.
 * pep/25Sep12
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"
#include "../include/netmgmt.h"
#include "../build/version.h"

enum {MaxReaders=Partitions, MaxPackets=8};
int main (int argc, char *argv [])
{ ShmType ctrlpartition;
  CtrlPartitionInfoType *ctrlinfo = NULL;
  LinkInfoType *rdlink = NULL;
  int i = 0, j = 0, k = 0, nreaders = 0, recvbytes = 0;
  CorrOutType rxcorr[MaxReaders][MaxPackets];

  fprintf (stderr, "# ---- %s: Processed Output Collator ----\n", argv[0]);
  fprintf (stderr, "# ---- %s ----\n\n", HUMAN_NAME);

  // Attach to control partition: Assume it is already populated by writer.
  // NOTE: put in action for when the writer has not been run first.
  bzero (&ctrlpartition, sizeof (ShmType));
  fprintf (stderr, "# Attaching to shared memory CONTROL area.\n");
  memcpy (ctrlpartition.path, "/part0", 16); // Control area has hardcoded path.
  ctrlpartition.size = sizeof (CtrlPartitionInfoType);

  if (attach_partition (&ctrlpartition) < 0) 
  { fprintf (stderr, "### Error creating CONTROL partition! Quitting...\n"); }

  ctrlinfo = (CtrlPartitionInfoType*) (ctrlpartition.ptr);
  // Determine number of readers to collate
  nreaders = ctrlinfo->currently_active;

  fprintf (stderr, "# Found %d currently active reader processes.\n", nreaders);
  rdlink = (LinkInfoType*) calloc (nreaders, sizeof (LinkInfoType));
  // TODO: Also create memory for receiving packets.

  for (i=0; i<nreaders; i++)
  { fprintf (stderr, "# Creating network link for reader %d on port %s.\n", 
             i, ctrlinfo->rinfo[i].portname);
    // TODO: Can pass a linkinfotype directly in readerinfotype?
    strncpy (rdlink[i].linkname, ctrlinfo->rinfo[i].linkname, 16);
    strncpy (rdlink[i].portname, ctrlinfo->rinfo[i].portname, 16);
    if (init_data_link (rdlink + i) < 0) 
    { fprintf (stderr, "## Unable to create link on link %s!\n", 
               rdlink[i].linkname);
      return -1; 
    }
    if (bind_data_link (rdlink + i) < 0) 
    { fprintf (stderr, "## Unable to bind onto link %s!\n", 
               rdlink[i].linkname);
      return -1; 
    }
  }

  // Collate incoming packets by timestamp values.
  for (k=0; k<100; k++)
  { for (j=0; j<MaxPackets; j++)
    { for (i=0; i<nreaders; i++)
      { if ((recvbytes=recvfrom (rdlink[i].sock, &rxcorr[i][j], 
          	sizeof (CorrOutType), 0, NULL, NULL)) == -1) 
        { perror("recvfrom"); break; }

        fprintf (stderr, "rdlink %d, pkt%d: tick: %d, frame:%d, set:%d.\n", 
                 i, j, rxcorr[i][j].hdr.tick, rxcorr[i][j].hdr.frameid, 
                 rxcorr[i][j].hdr.setid);
      }
    }
  }

  // Process collated packets.
  
  // Close network connections
  for (i=0; i<nreaders; i++)
  { if (close_data_link (rdlink + i) < 0)
    { fprintf (stderr, "### Error in closing data link %s!\n", 
               rdlink[i].linkname);
    }
  }

  // Tear down ctrl. partition.
  if (destroy_ctrl_partition (&ctrlpartition) < 0)
  { fprintf (stderr, "### Error destroying CONTROL partition! Quitting...\n"); }

  return 0;
}
