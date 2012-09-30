/* Routines for memory management of incoming raw data, and subsequent
 * feeding to FX processes.
 * pep/21Aug12
 */

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../include/memmgmt.h"

/* Signal handler, e.g., for SIGINT. Assures proper teardown of memory regions,
 * although this still needs to be done by the main thread.
 */
int MemDone = 0; // NOTE: Global defined here, only declared in header, in order
                 // to prevent multiple definition errors.
void mem_sig_hdlr (int dummy)
{ fprintf (stderr, "mem_sig_hdlr: Terminating memory partitions.\n");
  MemDone = 1;

  return;
}

int attach_partition (ShmType *shm)
{ if (shm == NULL) return -1;

  if ((shm->fd=shm_open (shm->path, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0)
  { perror ("attach_partition"); return -1;}
  ftruncate (shm->fd, shm->size);

  // NOTE: MAP_ANONYMOUS or MAP_HUGEPAGES?
  if ((shm->ptr = mmap (NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, 
                        shm->fd, 0)) == MAP_FAILED)
  { perror ("attach_partition():"); return -1; }

  return 0;
}

/* Create a single, control Shared memory region, to be shared by all readers.
 */
enum {ProcDataSrv=0xe0ff};
int create_ctrl_partition (ShmType *shm)
{ if (shm == NULL) return -1;
  CtrlPartitionInfoType *partinfo = NULL;
  int i = 0; 

  if (shm->path[0] == 0)
  { fprintf (stderr, "### create_part(): No shm->path found! Quitting...\n");
    return -1;
  }

  shm->size = sizeof (CtrlPartitionInfoType);
  shm->regions = 1; // Only a single region is created.

  attach_partition (shm);
  partinfo = (CtrlPartitionInfoType*) shm->ptr;

  // No registry being created.
  shm->registry = NULL;
  shm->curr_reg = 0;

  // Partition registry Initialization: Initialize all region parameters.
  // Create semaphore for access control on registry.
  // initial value = 1, shared between processes
  if (sem_init (&partinfo->sem, 1, 1) < 0) 
  { perror ("create_partition():"); }

  partinfo->currently_active = 0;
  // Fill in writer specific information.
  for (i=0; i<Partitions; i++)
  { partinfo->rinfo[i].total_partitions = Partitions;
    sprintf (partinfo->rinfo[i].partition_path, "/part%d", i+1);
    sprintf (partinfo->rinfo[i].portname, "%d", ProcDataSrv + i+1);
  }

  return 0; 
}

/* Create a single Shared memory partition, with parameters provided in ShmType.
 * Fills other fields in ShmType. Also initializes the region registry per 
 * partition. NOTE: This partition is meant for data exchange.
 */
int create_partition (ShmType *shm)
{ if (shm == NULL) return -1;
  int i = 0; 

  // Determine sizes for each component of a region.
  unsigned int regiondatsize = STA2Region * (Sets2STA*Frames2Set*Samps2Frame);
  unsigned int regionhdrsize = STA2Region * Sets2STA * Frames2Set *
                                sizeof (FrameHdrType);

  // The size of the registry of all regions, to be accessed by both writer 
  // and reader.
  unsigned int registrysize = sizeof (PartRegistryType) + 
               Regions2Part * sizeof (RawRegionType);
  unsigned int partitionsize = registrysize + 
               Regions2Part*(regiondatsize+regionhdrsize);

  if (shm->path[0] == 0)
  { fprintf (stderr, "### create_part(): No shm->path found! Quitting...\n");
    return -1;
  }
  fprintf (stderr, "# create_part():Region: %5.3fMB, Regions: %d, Registry: %dB, Partition: %4.0fMB.\n",
           (regiondatsize + regionhdrsize)/1048576., Regions2Part, 
           registrysize, partitionsize/1048576.);

  shm->size = partitionsize;
  shm->regions = Regions2Part;

  if ((shm->fd=shm_open (shm->path, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0)
  { perror ("create_partition"); return -1;}
  ftruncate (shm->fd, shm->size);

  // NOTE: MAP_ANONYMOUS or MAP_HUGEPAGES?
  if ((shm->ptr = mmap (NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, 
                        shm->fd, 0)) == MAP_FAILED)
  { perror ("create_partition():"); return -1; }
  
  // Create registry of RawRegionTypes for this partition.
  shm->registry = (PartRegistryType*)shm->ptr; 
  shm->curr_reg = shm->registry->region;

  // Partition registry Initialization: Initialize all region parameters.
  // Create semaphore for access control on registry.
  // initial value = 1, shared between processes
  if (sem_init (&shm->registry->sem, 1, 1) < 0) 
  { perror ("create_partition():"); }

  for (i=0; i<shm->regions; i++)
  { shm->registry->region[i].validsta = 0;
    shm->registry->region[i].lock   = 0;
    shm->registry->region[i].status = Empty;
    // Singleton registry->region[i].data->region[i].framemeta
    shm->regptr[i].data   =  shm->ptr + registrysize + 
                             i * (regiondatsize+regionhdrsize);
    shm->regptr[i].framemeta = 
             (FrameHdrType*) (shm->regptr[i].data + regiondatsize);
  } 

  // Associate signal handler
  if (signal (SIGINT, mem_sig_hdlr) == SIG_ERR)
  { fprintf (stderr, "create_partition: Unable to assign signal handler!\n");
  }
#if 0
  // Write test of partition
  for (i=0; i<Regions2Part; i++)
  { fprintf (stderr, "writing to region %d.\n", i);
    for (j=0; j<regiondatsize; j++)
      ((char*)shm->registry->region[i].data)[i] = i&255;
     // bzero (shm->registry->region[i].data, regiondatsize);
  }
#endif 

  return 0; 
}

int destroy_ctrl_partition (ShmType *shm)
{ if (shm == NULL) return -1;
  CtrlPartitionInfoType *partinfo = (CtrlPartitionInfoType*) shm->ptr;

  if (sem_destroy (&partinfo->sem) < 0)
  { perror ("destroy_partition():"); }

  if (munmap (shm->ptr, shm->size) < 0) perror ("destroy_partition()");
  if (shm_unlink (shm->path) < 0) perror ("destroy_partition()");
  return 0; 
}

int destroy_partition (ShmType *shm)
{ if (shm == NULL) return -1;

  if (sem_destroy (&shm->registry->sem) < 0)
  { perror ("destroy_partition():"); }

  if (munmap (shm->ptr, shm->size) < 0) perror ("destroy_partition()");
  if (shm_unlink (shm->path) < 0) perror ("destroy_partition()");
  return 0; 
}

/* Prints the current status of the registry of the specified partition.
 */
int registry_print (ShmType *shm, FILE *fp)
{ if (shm == NULL) return -1;
  PartRegistryType *reg = shm->registry;
  int i = 0;

  for (i=0; i<Regions2Part; i++)
  { fprintf (fp, "Region: %d Status: %s Data addr: %p, Meta addr: %p\n", 
             i, RegionStatus[reg->region[i].status], shm->regptr[i].data, 
             shm->regptr[i].framemeta); 
  }
  return 0;
}


/* Function to receive Frames2Set frames over two GigE links, create a single
 * set, and move it into the specified memory address.
 */
// int recv_set (LinkInfoType *netinfo, char *dest)
// { return 0;
// }

/* Function to dump the contents of the specified region to the specified file.
 * The raw data packets are regenerated by combining raw data with corresponding
 * frame headers.
 */
int region_write_to_file (RawRegionPtrType *reg, FILE *fp)
{ if (reg == NULL || fp == NULL) return -1;
  int i = 0;
  RawFrameType frame;
  
  for (i=0; i<Frames2Set*Sets2STA*STA2Region; i++)
  { memcpy (&frame.hdr, reg->framemeta + i, sizeof (FrameHdrType));
    memcpy (&frame.data, reg->data + i*Samps2Frame, Samps2Frame);
    fwrite (&frame, 1, sizeof (RawFrameType), fp);
  }

  return 0;
}

/* Debug function for filling an STA within a region with
 * data read from a file. File is assumed to contain at least a single STA worth
 * of data. staoff is the offset in STA units within the region.
 */
int region_fill_STA_file (RawRegionPtrType *reg, int staoff, FILE *fp)
{ if (reg == NULL || fp == NULL) return -1;
  int i = 0;
  static int setid = 0;
  unsigned int stadatsize = Samps2Frame*Frames2Set*Sets2STA; //Bytes.
  RawFrameType frame; // Currently a single frame is read in...

  // Read an STA, frame at a time from file, which is assumed to have 1STA worth
  // of data. May need a function to generate an STA coherently..
  for (i=0; i<Frames2Set*Sets2STA; i++)
  { fread (&frame, 1, sizeof (RawFrameType), fp); 
    frame.hdr.setid = setid;
    // NOTE: Using Samps2Frame as the size of the frame payload. Should have a
    // proper size somewhere...
    memcpy (reg->data + staoff*stadatsize + i*Samps2Frame, 
            &frame.data, Samps2Frame);
    // NOTE: reg->framemeta is a pointer of type FrameHdrType!
    memcpy (reg->framemeta + staoff*Frames2Set*Sets2STA + i, &frame.hdr, 
            sizeof (FrameHdrType));
  }
  setid ++;

  return 0;
}

/* Function to fill an STA worth data within the specified region from the 
 * links in linfo0, linfo1
 */
int region_fill_STA_net (RawRegionPtrType *reg, int staoff, 
              struct StLinkInfoType *link0, struct StLinkInfoType *link1, 
              unsigned char *set)
{ if (reg == NULL || link0 == NULL || link1 == NULL || set == NULL) return -1;
  int i = 0, j = 0;
  RawFrameType *frame = (RawFrameType*) set;
  unsigned int stadatsize = Samps2Frame*Frames2Set*Sets2STA; //Bytes.
  unsigned int stahdrsize = Frames2Set*Sets2STA; //Bytes.
  unsigned int setdatsize = Samps2Frame*Frames2Set;
  
  for (i=0; i<Sets2STA; i++)
  { if (recv_link_pair (link0, link1, set) < 0)
    { fprintf (stderr, "region_fill_STA_net: recv_link_pair error!\n"); }

    // Assured STA frame available, now strip headers and write to region
    for (j=0; j<Frames2Set; j++)
    { memcpy (reg->data + staoff*stadatsize + i*setdatsize + j*Samps2Frame, 
              &frame[j].data, Samps2Frame);
      // NOTE: reg->framemeta is a pointer of type FrameHdrType!
      memcpy (reg->framemeta + staoff*stahdrsize + i*Frames2Set + j, 
              &frame[j].hdr, sizeof (FrameHdrType));
    }
  }
  return 0;
}


/* Function to print the meta header values of all frames within a region,
 * specified in reg.
 */
int region_print_frame_meta (RawRegionPtrType *reg, FILE *fp)
{ int i = 0, j = 0;
  FrameHdrType *hdr = NULL; 

  for (i=0; i<STA2Region; i++)
  { fprintf (fp, "# STA: %3d\n", i);
    hdr = reg->framemeta + i*Frames2Set*Sets2STA;
    // For printing all frame hdrs
    for (j=0; j<Sets2STA*Frames2Set; j++) 
    // For printing only the first few frame hdr in an STA.
    // for (j=0; j<3; j++)
      fprintf (fp, "%3d %5d %8d %8d\n", j, hdr[j].setid, hdr[j].frameid, 
               hdr[j].tick);
    
  }
  return 0;
}

/* Function to change the status parameters of the specified region in the 
 * registry.
 */
int registry_change_region_status (PartRegistryType *regis, int reg, int status)
{ if (regis == NULL) return -1;
  sem_wait (&regis->sem); // Blocked wait on registry access.
  regis->region[reg].status = status;
  sem_post (&regis->sem); // Unlock the registry.
  return 0;
}

/* Function to find a region to write into, by interrogating the registry.
 * Once a region is found, its status is set to stale, its lock is asserted,
 * and writing into that region begins, at the STA offset specified. After
 * writing a region worth of STAs, the registry is updated with this region
 * being marked as the latest.
 *   Function returns the region offset within the partition which should be 
 * used for writing new data. If block is 1, function blocks till a new region
 * is ready for writing into, else, it returns -1 to indicate all regions being
 * full. 
 */
int find_wr_region (ShmType *shm, int block)
{ int i = 0, found = 0;

  if (block == NonBlock) // NON BLOCKING CASE
  { // Scan through registry for a region with status EMPTY
    for (i=0; i<Regions2Part; i++)
    { if (shm->registry->region[i].status == Empty)
      { // Found an empty region: Mark status as WRITING, return.
        registry_change_region_status (shm->registry, i, Writing);
        found = 1; break;
      }
    }
    if (found) return i;
    else return -1; // No empty region found, return -1;
  }
  else // BLOCKING CASE, for all other values of block.
  {  do
     { for (i=0; i<Regions2Part; i++)
       { if (shm->registry->region[i].status == Empty)
         { // Found an empty region: Mark status as WRITING, return.
           registry_change_region_status (shm->registry, i, Writing);
           found = 1; break;
         }
       }
       usleep (100);
       if (MemDone == 1) {i = -1; break;}
     } while (!found);
    return i;
  }
}

/* Function similar to find_wr_region (), this is called by the data consumer
 * to figure out which region to process next. Should be called when the 
 * producer indicates that data is ready, via registry->avial > 0;
 *   NOTE: This function should be called explicitly, inspite of the latest 
 * region available for reading being indicated in registry->avail. This 
 * explicit call allows the reader to lag behind the writer, and choose between
 * multiple Full regions.
 *
 */
int find_rd_region (ShmType *shm, int block)
{ int i = 0, found = 0;

  if (block == NonBlock) // NON BLOCKING CASE
  { // Scan through registry for a region with status FULL 
    for (i=0; i<Regions2Part; i++)
    { if (shm->registry->region[i].status == Full)
      { // Found an empty region: Mark status as READING, return.
        registry_change_region_status (shm->registry, i, Reading);
        found = 1; break; // NOTE: Update for handling multiple full regions!
      }
    }
    if (found) return i;
    else return -1; // No full regions found, return -1;
  }
  else // BLOCKING case for all other values of block.
  {  do
     { for (i=0; i<Regions2Part; i++)
       { if (shm->registry->region[i].status == Full)
         { // Found an empty region: Mark status as Reading, return.
           registry_change_region_status (shm->registry, i, Reading);
           found = 1; break;
         }
       }
       usleep (100);
       if (MemDone == 1) {i = -1; break;} // indicate no region was found.
     } while (!found);
    return i;
  }
}
