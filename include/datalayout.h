/* Header file defining the layouts of various data structures in the system.
 * pep/23Aug12
 */
#ifndef __DATALAYOUT_H__
#define __DATALAYOUT_H__

/*   Terminology:
 *      Basic unit = 4-bit data from a pair of antennas.
 *      1 Frame = Samps2Frame samples of a pair of antennas.
 *      1 Set   = Samps2Frame samples from 20 pairs of antennas.
 *      1 STA   = Sets2STA Sets (8ms @ 37.248 MHz sampling);
 *      1 Region= STA2Region number of STA.
 *      1 Partition = Regions2Part Regions, seen exclusively by a single FX.
 *      1 FFT timeslice = Taps samples.
 */
enum { Antennas    = 40,
       Taps        = 64,
       // Blines      = 40*41/2,
       Blines      = 40*39/2,
       Samps2Frame = 1536,
       Frames2Set  = 20,    // Antennas/2
       Sets2STA    = 97*2,  // STA=8ms@37.248 MHz sampling, 2.48MB per STA.
       STA2Region  = 10,    // ~400ms 
       Regions2Part= 5,     // A Partition is strictly associated with a single
                            // FX process, which mmaps only a single partition
                            // onto its address space.
       TSlices2Set = Samps2Frame/Taps 
     };
    
/****** RAW DATA LAYOUT
 ****** - Each frame consists of 1536B payload, and 32B header.
 ****** - Each byte of a frame holds nibbles from the same sampling tick, from
 ******   2 antennas, thus one frame holds 1536 samples from a pair of ants.
 ****** - 10 Consecutive frames incoming on a single GigE link from the bridge
 ******   card should contain 20 Ph.1 data streams, for the same timestretch.
 ****** - The 10 consecutive frames on the other link should contain the 
 ******   remaining 20 antennas, making up the set of 40 antennas.
 ****** - Frames belonging to the same time have identical ticks, as well as 
 ******   setids. (TBD)
 */

typedef struct
{ unsigned int tick;        // Monotonic counter on sampling clock, identical 
                            // for all frames belonging to a set.
  unsigned short frameid;   // Identifies a particular ADC chip.
  unsigned short setid;     // Identical for all frames belonging to a set.
  int dummy[6]; // 32-byte hdr
} __attribute__((packed)) FrameHdrType;

typedef struct
{ FrameHdrType hdr;
  unsigned char data[Samps2Frame];
} RawFrameType;

typedef struct { char re[16],im[16] ; } 
ByteVectorType __attribute__ ((aligned(16))) ;

typedef struct { short re[8], im[8] ; } 
WordVectorType __attribute__ ((aligned(16))) ;

typedef struct { int re[4], im[4] ; } 
DblWordVectorType __attribute__ ((aligned(16))) ;

/* FFT output format, without header.
 * Output word is a short complex vector consisting of data from 4 FFT blocks.
 * In the current layout, a pair of consecutive channels from the same antenna,
 * from 4 FFT blocks are grouped together in a WordVectorType.
 *   Since a frame has 64*4*6 FFT blocks, and 4 are accomodated in a vector, a 
 * group of 6 WordVectorTypes holds a pair of channels from all 24 FFT blocks.
 *   Following these 6 WordVectorTypes, we have the 6 WordVectorTypes from the 
 * next antenna, and so on for all 40 antennas. Following this, we have the 
 * next channel pair for all FFT blocks in a frame, for all antennas. We have 
 * 16 such groups to cover the 32 channels.
 */

// FFT specific enums 
enum {Chans2Group   = 2,             // Operate channel pair at a time.
      ChanGroups    = Taps/(2*Chans2Group),  // 16 groups for 32 channels.
      Vectors2Frame = 3*Chans2Group};     // NOTE: For a ChanGroup (2chan) only!


// Correlated output format, without header
typedef struct
{ FrameHdrType hdr;
  int corr[Blines*Taps]; // re/im => 2 ints/correlation
} CorrOutType;

#endif // __DATA_LAYOUT_H__
