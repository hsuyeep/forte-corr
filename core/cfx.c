/* Program to apply a C-based correlator on data from a shared memory region.
 * pep/23Aug12
 *
 * Updated program to attach to a control partition being created by ioproc.c 
 * for exchanging general purpose information. Also writing out correlated 
 * packets to network. The output can be collected by a collator process.
 * pep/26Sep12
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "../include/datalayout.h"
#include "../include/memmgmt.h"
#include "../include/netmgmt.h"
#include "../include/corrops.h"
#include "../build/version.h"

#define WRITE2FILE 0
#define WRITE2NET 1

int main (int argc, char *argv [])
{ ShmType partition, *shm = NULL;
  ShmType ctrlpartition;
  CtrlPartitionInfoType  *ctrlinfo = NULL;
  RawRegionPtrType *regptr = NULL;
  FftType *fftinfo = NULL;
  CorrOutType corr;
  LinkInfoType outlink;
  int sentbytes = 0;
  int i = 0, j = 0, rd_reg = -1;
#if  WRITE2FILE
  FILE *fbin = NULL;
  char fname[16] = {0,};
#endif 
  unsigned int setsize = Samps2Frame * Frames2Set; // Bytes.
  unsigned int stasize = setsize * Sets2STA;       // Bytes.

  // NOTE: Offset into control area for region claimed by this process.
  int ctrloff = -1; 

  fprintf (stderr, "# ---- %s: FFTW Based Correlator ----\n", argv[0]);
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
      strncpy (outlink.linkname, ctrlinfo->rinfo[i].linkname, 16);
      strncpy (outlink.portname, ctrlinfo->rinfo[i].portname, 16);
      // NOTE: This is the ONLY variable which stores offset in ctrl region! 
      // Guard carefully!!
      ctrloff = i; 
      fprintf (stderr, "--> Claiming partition %d.\n", ctrloff);
      break;
    }
  }

#if WRITE2FILE
  gen_bin_fname (fname, 16); // NOTE: Maybe should come from ctrl region?
  fprintf (stderr, "# Writing correlated packets to file %s\n", fname);
  if ((fbin=fopen (fname, "wb")) == NULL)
  { perror ("fopen"); return -1; }
#elif WRITE2NET
  fprintf (stderr, "# Setting up output connection to hosts %s.\n",
           outlink.linkname);
  if (init_data_link (&outlink) < 0)
  { fprintf (stderr, "# Unable to send datagrams to %s.\n", outlink.linkname); }
#endif

  // Get FFTW initialized
  if (init_fftw (&fftinfo) < 0)
  { fprintf (stderr, "init_fftw failed!\n"); return -1; }

  // Set all partition parameters to 0.
  bzero (&partition, sizeof (ShmType));

  fprintf (stderr, "# Attaching to shared memory region.\n");

  // memcpy (partition.path, "/part0", 16); //Currently hardcoded path and size.
  memcpy (partition.path, ctrlinfo->rinfo[ctrloff].partition_path, 16); 

  if (create_partition (&partition) < 0) 
  { fprintf (stderr, "### Error creating partition! Quitting...\n"); }

  shm = &partition;
  regptr = shm->regptr;
  while (!MemDone)
  { // Find a region to read.
    while (1)
    { // rd_reg = find_rd_region (shm, NonBlock);
      rd_reg = find_rd_region (shm, Block);
      if (rd_reg != -1)
      { fprintf (stderr, "<-- %s: Region %d Readable. Status: %s\n", 
                 ctrlinfo->rinfo[ctrloff].partition_path, rd_reg,
                 RegionStatus[shm->registry->region[rd_reg].status]);
        break;
      }
      else
      { fprintf (stderr, "No Readable region found in non-blocking search.\n");
        sleep (1);
        if (MemDone == 1) break;
      }
    }

    fprintf (stderr, "Packet Tick: %d, Frameid: %d, setid: %d\n", 
             shm->regptr[rd_reg].framemeta->tick, 
             shm->regptr[rd_reg].framemeta->frameid, 
             shm->regptr[rd_reg].framemeta->setid
            );
    fprintf (stderr, "Corr Tick: %d, Frameid: %d, setid: %d\n", corr.hdr.tick,
             corr.hdr.frameid, corr.hdr.setid);

    // Found a valid region! Carry out correlation of every STA making up the 
    // region. 1 Region has STA2Region * Sets2STA sets. Each set has 
    // Frames2Set frames. FFT and correlation currently occurs on a single set.
    for (i=0; i<STA2Region; i++) // Process every STA within region.
    { unsigned char *set = shm->regptr[rd_reg].data + i*stasize; 

      for (j=0; j<Sets2STA; j+=Frames2Set) // Process every set within STA.
      { fftw_set_fft (set + j*setsize, Frames2Set, fftinfo); // FFT of one set.
        // print_fftout_file (fftinfo, stderr);           // ASCII dump of set.
        // dump_setoutput_file (fftinfo, fftwordfile);    // BLOB dump to file.

        xmac_set (fftinfo, &corr);             // XMAC within a set, output in 
                                               // corr.
        // Fill in header information. NOTE: The meta data of the first 
        // raw data frame is currently the meta data for the correlated packet.
        memcpy (&corr.hdr, shm->regptr[rd_reg].framemeta,sizeof (FrameHdrType));
      #if WRITE2FILE
        dump_xmac_reim_file (&corr, fbin);          // Dump contents to disk
      #elif WRITE2NET
        {  if ((sentbytes = sendto (outlink.sock, &corr, 1024, //sizeof(CorrOutType), 
            0, outlink.servinfo->ai_addr, outlink.servinfo->ai_addrlen)) == -1) 
          { perror("sendto outlink"); }
        }
      #else 
        // print_xmac_reim_file (&corr, stdout);    // print for debug.
        print_xmac_ampph_file (&corr, stdout, 1);// print for debug.
      #endif
      }
    }

    // Finished operating on memory region, set registry entry as empty.
    if (registry_change_region_status (shm->registry, rd_reg, Empty) < 0)
    { fprintf (stderr, "### Unable to change registry status!\n"); } 

    fprintf (stderr, "<--    Region %d Processed. Status: %s\n", rd_reg,
                 RegionStatus[shm->registry->region[rd_reg].status]);
  }
  
  // Destroy partitions.
  if (destroy_partition (shm) < 0)
  { fprintf (stderr, "### Error destroying partition! Quitting...\n"); }
  if (destroy_ctrl_partition (&ctrlpartition) < 0)
  { fprintf (stderr, "### Error destroying CONTROL partition! Quitting...\n"); }

  if (deinit_fftw (fftinfo) < 0)
  { fprintf (stderr, "deinit_fftw failed!\n"); }
#if WRITE2FILE
  if (fbin) fclose (fbin);
#elif WRITE2NET
  if (close_data_link (&outlink) < 0)
  { fprintf (stderr, "# Unable to close datalink %s.\n", outlink.linkname); }
#endif

  return 0;
}
