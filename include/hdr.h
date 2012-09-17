/*  Header file for all hdr formats for the Gbit DAS system
 * pep/23Apr09
 *
 *  Massively changed header format including bpp, chans etc fields added.
 * The idea being that any type of packet generated should have a consistent
 * header format, be it generated from FPGA processing or from PC processing.
 *
 * As such, all types of packets have the same 8 byte beginning structure, but
 * certain kinds of packets can have header fields extending beyond the 
 * mandatory 8 bytes. This is known only to the programs which can digest those
 * kinds of packets, the rest see only the beginning 8 bytes of header.
 *
 *  The main advantage is that post processing of generated packets can be done
 * by any processing entity, be it FPGA processing PC generated data or vice
 * versa. The use of switches in between PCs and FPGAs makes knowing the sources
 * of data irrelevent.
 * pep/28jun09
 */
#ifndef _HDR_H_
#define _HDR_H_

#include <complex.h>
#include<sys/time.h>
#include <time.h>

#define VERSION "0.2"

//*************** NOTE: Since everything is UDP packets based, sensitive issues
//*************** like changing destination IP addresses/LUT updations should be
//*************** done keeping in mind that packets can be lost in between.
//*************** Seek Confirmation, seek it often!

/* FPGA can send status and data packets only, while PC can sent status, data 
 * and command packets. All pkts have the same header, and are distinguished by
 * the datatype/srcid field. 
 * NOTE: These entries also define the various port numbers to wait on. These
 * are hardcoded in the FPGA code as well.
 */ 
enum {DataPkt=0xd0, StatPkt=0xf0, CmdPkt=0xc0};
enum {DataPort=(DataPkt<<8)+255, StatPort=(StatPkt<<8)+255, 
      CmdPort=(CmdPkt<<8)+255};

/* Types of data packets: 
 * These flow only from the FPGA to the PC.
 * A 4 bit field is available, giving 16 different types of handlable data.
 */
enum {DataUserDef ,            // User defined data type
      DataCnt     ,            // Contents are a 32bit counter, for debug
      DataRaw     ,            // Contents are ADC raw data
      DataRawZip  ,            // Contents are ADC raw compressed data
      DataFFTRaw  ,            // Contents are FFT Complex output
      DataFFTZip  ,            // Contents are FFT Compressed output
      DataFFTPow  ,            // Contents are FFT Power spectra only
      DataSpectCorr,           // Contents are Spectral Correlations
      DataRawStat ,            // Contents hold data statistics only
      DataMeta,                // Contents hold metadata like history or scanhdr
      DataSpectCorrPack,       // Contents are Spectral Correlations with 
                               // minimal header. Size is in units 
      DataTypes};

char DataType[DataTypes][80] = 
   { [DataUserDef ] = "User defined data",
     [DataCnt     ] = "32bit Counter for debug",
     [DataRaw     ] = "ADC Raw Data",
     [DataRawZip  ] = "ADC Raw Data, Compressed in time domain",
     [DataFFTRaw  ] = "ADC data with FFT applied",
     [DataFFTZip  ] = "ADC data with FFT applied, spectrally compressed",
     [DataFFTPow  ] = "ADC data Power spectra",
     [DataSpectCorr] = "Correlated ADC data",
     [DataRawStat ] = "ADC data statistics",
     [DataMeta    ] = "Observation scan related meta data",
     [DataSpectCorrPack] = "Correlated ADC data with minimal header"
   };

/* Bits per pixel: 
 * Kind of data formats recognized, pixel here refers to data from a single 
 * channel. A packet can have multiple channels together.
 * A 4 bit field is available, giving 16 different formats for the 16 different
 * types of handlable data.
 */
enum {Unknownbpp,
      _2bpp, _4bpp, _8bpp,     // Generally used for Raw data formats
      _2b3lev, _3b5lev,
      _8scmplx, _16scmplx, _32fcmplx, // Generally used for FFT/Corr formats
      Statword, 
      BppTypes};

char BppDesc[BppTypes][80] =
   { [Unknownbpp] = "Unknown bits per pixel",
     [_2bpp     ] = "2 bits per pixel",
     [_4bpp     ] = "4 bits per pixel",
     [_8bpp     ] = "8 bits per pixel",
     [_2b3lev   ] = "2 bit, 3 level",
     [_3b5lev   ] = "3 bit, 5 level",
     [_8scmplx  ] = "8 bit signed int Complex",
     [_16scmplx ] = "16 bit signed int Complex",
     [_32fcmplx ] = "32 bit float Complex",
     [Statword  ] = "StatisticWordType"
   };
      
/* Channels or baselines (depends on datatype) in a given packet.  
 * A 4 bit field is available, giving upto 16 combinations of channels
 */
enum { _2chan,  _10chan,  _20chan,  _30chan, _40chan, _12chan, _24chan, _36chan,
      _48chan, _60chan, _72chan, _84chan, _96chan, _66chan, _132chan, _264chan,
      ChanTypes};

//enum { _2chan,  _4chan,  _6chan,  _10chan, _12chan, _16chan, _24chan, _40chan,
//      _48chan, _60chan, _72chan, _84chan, _96chan, _66chan, _132chan, _264chan,
//      ChanTypes};

int NChans [ChanTypes] = 
   { [_2chan ] = 2 ,
     [_10chan] = 10 ,
     [_20chan] = 20 ,
     [_30chan] = 30 ,
     [_40chan] = 40,
     [_12chan] = 12,
     [_24chan] = 24,
     [_36chan] = 36,
     [_48chan] = 48,
     [_60chan] = 60,
     [_72chan] = 72,
     [_84chan] = 84,
     [_96chan] = 96,
     [_66chan] = 66,
     [_132chan] = 132,
     [_264chan] = 264
   };

char ChanDesc[ChanTypes][80] =
   { [_2chan ] = "2 Channels",
     [_10chan] = "10 Channels",
     [_20chan] = "20 Channels",
     [_30chan] = "30 Channels",
     [_40chan] = "40 Channels",
     [_12chan] = "12 Channels",
     [_24chan] = "24 Channels", 
     [_36chan] = "36 Channels",
     [_48chan] = "48 Channels",
     [_60chan] = "60 Channels",
     [_72chan] = "72 Channels",
     [_84chan] = "84 Channels",
     [_96chan] = "96 Channels",
     [_66chan] = "66 Channels",
     [_132chan] = "132 Channels",
     [_264chan] = "264 Channels"
   };

enum {_2mods, _10mods, _12mods, _16mods, _22mods, _24mods, _40mods, _48mods, 
      BlineTypes};
int NBlines[BlineTypes] = 
   { [_2mods   ] = 3,          // Two modules = 1 baseline only + selfs
     [_10mods  ] = 10*11/2,      // n.(n+1)/2, includes selfs
     [_12mods  ] = 12*13/2,    // n.(n+1)/2, includes selfs
     [_16mods  ] = 16*17/2,
     [_22mods  ] = 22*23/2,
     [_24mods  ] = 24*25/2,
     [_40mods  ] = 40*41/2,
     [_48mods  ] = 48*49/2
   };

char BlineDesc[BlineTypes][80] = 
   { [_2mods  ] = "2 modules, 1 baseline",
     [_10mods ] = "10 modules, 55 baselines",
     [_16mods ] = "16 modules, 136 baselines",
     [_22mods ] = "22 modules, 253 baselines",
     [_40mods ] = "40 streams, 820 baselines"
   };

/* Types of status packets: 
 * These flow only from the FPGA to the PC.
 */
enum {SSubIAA = 1,             // I am alive status packet
      SSubStart  ,             // Tx in start state
      SSubStop   ,             // Tx in stop state
      SSubInvalOp,             // Command invalid
      SSubCmdDone,             // Command successfully executed
      SSubLUT    ,             // Contains current LUT entries as data
      SSubMon    ,             // This is a monitor packet
      StatusTypes};

/* Types of Command packets: 
 * These flow only from PC to FPGA
 */
enum {CSubReset    ,   // Assert internal reset. All counters reset
      CSubSendAYA  ,   // Send an Are you alive packet to FPGA
      CSubStart    ,   // Start FPGA application 
      CSubStop     ,   // Stop FPGA application
      CSubUpdateHdr,   // Update internal IP hdrs
      CSubUpdateLUT,   // payload has LUT updation data, see pktsub
      CSubSendLUT  ,   // Send current LUT entries to PC
      CSubDoDbg    ,   // Ask FPGA for a 32bit cntr at specified rate
      CSubFlushFIFO,   // Flush all FIFOs/buffers. TSCs are unaffected.
      CSubGPIO     ,   // Manipulate the 32 bit GPIO lines on FPGA
      CSubChanSel  ,   // Choose from upto 16 channel inputs
      CSubInit     ,   // Send an init command to data sources
      CSubSetDelay ,   // Set a delay for given channel
      CSubSetTS    ,   // Set the TimeStamp counter to the given value
      CSub2ch8bpp  ,   // Choose a 2ch, 8bpp continuous stream mode from eth0
      CSubUserDef  ,   // Look at pktsub for decode.NOTE: Always keep this last!
      CmdTypes};       

/* These flags indicate whether additional information is needed to construct
 * a particular command packet.
 */
int CmdNeedSubType[CmdTypes] = 
   { [CSubReset]    = 0, 
     [CSubSendAYA]  = 0,
     [CSubStart]    = 0,
     [CSubStop]     = 0,
     [CSubUpdateHdr]= 1,
     [CSubUpdateLUT]= 1,
     [CSubSendLUT]  = 0,
     [CSubDoDbg]    = 0,
     [CSubFlushFIFO]= 0,
     [CSubGPIO]     = 1, 
     [CSubChanSel]  = 1,
     [CSubInit]     = 0,
     [CSubSetDelay] = 1,
     [CSubSetTS]    = 1, 
     [CSub2ch8bpp] = 1,
     [CSubUserDef]  = 0
   };

// Command names used by users to communicate with us.
enum {MaxCmdNameSize=16, MaxCmdExplainSize=80};
char CmdName[CmdTypes][MaxCmdNameSize] = 
   { [CSubReset]    = "Reset", 
     [CSubSendAYA]  = "AYA",
     [CSubStart]    = "Start",
     [CSubStop]     = "Stop",
     [CSubUpdateHdr]= "Updatehdr",
     [CSubUpdateLUT]= "UpdateLUT",
     [CSubSendLUT]  = "SendLUT",
     [CSubDoDbg]    = "DoDebug",
     [CSubFlushFIFO]= "Flushfifo",
     [CSubGPIO]     = "GPIO",
     [CSubChanSel]  = "ChanSel",
     [CSubInit]     = "Init",
     [CSubSetDelay] = "SetDelay",
     [CSubSetTS]    = "SetTS",
     [CSub2ch8bpp] = "2ch8bpp",
     [CSubUserDef]  = "UserDef"
    }; 

// Command explanations
char CmdExplain[CmdTypes][MaxCmdExplainSize] =
  {  [CSubReset]    = "Reset all counters & dest addrs, continue in prev state",
     [CSubSendAYA]  = "Send AreYouAlive pkt to FPGA,Should get an IAA pkt.",
     [CSubStart]    = "Send a Start command to FPGA application.",
     [CSubStop ]    = "Send a Stop command to FPGA application.",
     [CSubUpdateHdr]= "Update MAC+IP+UDP+DAS hdrs, for specifying new dest.",
     [CSubUpdateLUT]= "Update a specific LUT, App dependent.UNIMPLEMENTED.",
     [CSubSendLUT]  = "Request a specific LUT, App dependent.UNIMPLEMENTED.",
     [CSubDoDbg]    = "Request FPGA to send a 32bit cntr at specified rate.",
     [CSubFlushFIFO]= "Flush all internal buffers/fifo.UNIMPLEMENTED.",
     [CSubGPIO]     = "Set GPIO lines. To query GPIO, send an AYA cmd.",
     [CSubChanSel]  = "Select any combination from upto 16 channels",
     [CSubInit]     = "Init:send external sync & reset FIFOs of all data srcs",
     [CSubSetDelay] = "Set a delay for a chosen ADC channel",
     [CSubSetTS]    = "Reset the 32 bit Time stamp counter to the given value",
     [CSub2ch8bpp]  = "Set ADC1's 2 channel, 8bits/pixel mode on eth0 of DAS",
     [CSubUserDef]  = "User defined command."
  };

enum {DataLen=1024};
// Old header: kept for sentimental reasons
typedef struct
{ unsigned char type;          // Magic number, one of PktType
  unsigned char words;         // Packet length in 64bit words
  unsigned short seq;          // 16 bit pkt counter
  unsigned int tick;           // 32 bit counter running on FPGA's clock
} HdrType;

// Reject packets larger than MaxPktSize, which is all we can accomodate in the
// 12 bits allotted to words..
enum {MaxPktSize = 4096*8};    // Bytes, based on num. of bits alloted to words
typedef struct
{ // unsigned int flags    :2; // More fragments/intermediate frag
  // unsigned int srcid    :6; // Unique srcid for each data source
  unsigned char srcid;
  unsigned int  datatype:4;    // Kind of data in pkt, one of (above) DataTypes
  unsigned int  bits2pix:4;    // How many bits for each word
  unsigned int  chans   :4;    // Number of independent data streams in this pkt
  unsigned int  words   :12;   // Size of the packet in words of 64 bits each
  unsigned int  tick;           // 32bit timestamp on write strb,unique for each src
} __attribute__((packed)) ADCHdrType;

typedef struct
{ unsigned int seq   :12;   // Layer specific sequence number for this pkt
  unsigned int id    :4;    // Intermediate layer source id
} __attribute__ ((packed)) LayerHdrType;

typedef struct
{ // unsigned int flags    :2; // More fragments/intermediate frag
  // unsigned int srcid    :6; // Unique srcid for each data source
  unsigned char srcid;
  unsigned int  datatype:4;    // Kind of data in pkt, one of (above) DataTypes
  unsigned int  bits2pix:4;    // How many bits for each word
  unsigned int  chans   :4;    // Number of independent data streams in this pkt
  unsigned int  words   :12;   // Size of the packet in words of 64 bits each
  unsigned int  tick;          // 32bit timestamp on write strb,unique for each src
  LayerHdrType lay[3];         // Layer specific headers
  unsigned char grpid;
  unsigned char dummy;
} __attribute__((aligned (16), packed)) DataHdrType; 

/*  // Ultimately, DatahdrType should be defined as follows
typedef struct
{ ADCHdrType adchdr;
  LayerHdrType l3, l2, l1;
  unsigned char grpid;
  unsigned char dummy;
} DataHdrType;
*/ 
  
typedef struct
{ DataHdrType hdr;
  unsigned char data[0];
} DataPktType;

typedef struct
{ HdrType hdr;
  unsigned char data[0];
} CmdPktType;

/* Layout of a status packet. This has fields which are manipulatable by
 * the App for app specific status. Also has common status fields.
 */
typedef struct
{ unsigned char mac[6];
  unsigned char ip [4];
  unsigned short csum;
} FPGADstType;

/* Every status packet has at the very least this much information
 */
typedef struct
{ unsigned char FPGAVER;       // Indicates the version of the FPGA code
  unsigned char ability_state; // The advertised ability of FPGA code. Should be
                               // one among DSub* above (4bit). LSB 4 bits are 
                               // current state
  unsigned short appdef;       // FPGA app specific field.
  unsigned int GPIOlines;      // The current value of FPGA internal GPIO lines 
  FPGADstType destaddr[4];     // Internal current destinations.
  unsigned char data[0];       // Contains things like LUTs, current 
                               // destinations etc.
                               // Size should be calculated based on type of 
                               // status packet as well as size reported in the 
                               // UDP header.
} StatPktType;

typedef struct
{ unsigned char srcid;
  unsigned int  datatype: 4;
  unsigned int  bits2pix: 4;
  unsigned int  chans   : 4;
  unsigned int  words   :12;   // NOTE: size of pkt including hdr in 8byte units
  unsigned int  tick;
  unsigned int  taps;
  float         f_samp;        // Sampling frequency in MHz
} SpectHdrType;

typedef struct
{ SpectHdrType hdr;
  float pspect[0];
} PowPktType;

typedef struct
{ SpectHdrType hdr;
  _Complex float spect[0];
} SpectPktType;

typedef struct
{ unsigned char srcid;
  unsigned int  datatype: 4;
  unsigned int  bits2pix: 4;
  unsigned int  chans   : 4;   // interpreted as the number of blines in pkt
  unsigned int  words   :12;
  unsigned int  tick;
  unsigned short taps;         // NOTE Max. taps
  unsigned short blines;       // NOTE Max. baselines per packet
  float         f_samp;
} CorrHdrType;

// NOTE: All cross power spectra generated by spectcorr prior to 03Feb10 does 
// not have the tick field in the BlineWordType structure, so needs to be 
// regenerated to have a look at it using showcorr and others.
typedef struct                 // Represents data from one baseline, cross spect
{ char a0;                     // The first module/antenna
  char astar1;                 // The second module/antenna, sig (a0.a1*)
  short flag;                  // To hold flagging information
  unsigned int tick;           // Holds the timestamp, useful for reorganizing
                               // data on a per baseline basis.
  _Complex float cspect[0];    // The complex Cross spectra for this baseline
} BlineWordType;

typedef struct
{ CorrHdrType hdr;
  BlineWordType *bline;        
} CorrPktType;

// In the packed CPS storage structure, the CorrHdrType object remains, but the
// 12bit words field of the mandatory initial MetaDataType is set to all 1s 
// (0xfff), which implies that the packet size cannot fit in the words field, 
// but is available in the tick field.
//  This larger packet allows a single output packet to be formed without 
//  splitting, and also the placement of CPS data without per baseline headers. 
typedef struct                 // Represents data from one baseline, cross spect
{ char a0;                     // The first module/antenna (-1 => flagged bline)
  char astar1;                 // The second module/antenna, sig (a0.a1*)
} PackBlineHdrType;

// In this layout, all baseline data is clumped together, while baseline infor
// mation is at the packet beginning.
typedef struct
{ CorrHdrType hdr;
  PackBlineHdrType *bline; 
  // _Complex float cspect[0];  // NOTE: The CPS follows thi
} PackCorrPktType;

typedef struct
{ unsigned char srcid;
  unsigned int  datatype: 4;
  unsigned int  bits2pix: 4;
  unsigned int  chans   : 4;
  unsigned int  words   :12;   // NOTE: sizeof pkt (with hdr) in 8byte units
  unsigned int  tick;
  LayerHdrType lay[3];
  unsigned char grpid;
  unsigned char dummy;

  unsigned int  samps;
  float f_samp;                // NOTE: All headers should be 8 byte aligned
} __attribute__((aligned (16), packed)) StatisticHdrType;

enum {HistBins=32};
typedef struct
{ float mean, var;
  unsigned int hist[HistBins];
} StatisticWordType;

typedef struct
{ StatisticHdrType hdr;
  StatisticWordType word[0];
} StatisticPktType; 

// Timing related: NOTE: ORDER OF hw/sw matters!!
typedef struct
{ unsigned int hw, sw; } Tick; // Could not come up with anything better...
typedef union
{ Tick t;
  unsigned long long abscnt;   // Software and Hardware absolute cntr.
} LongTick;

// Fragmentation related masks
enum {MaxFrags=128, FragMask=0xC0, FirstFrag=0x80, IntermedFrag=0xC0, 
      LastFrag=0x40, SeqShift=16, FragSeqMask=0xffff};

// Information specific to the field being observed
typedef struct
{ char name[32];
  float ra, dec;               // src coordinates in J2000
} SrcInfoType;

// Information specific to the observer for this scan
typedef struct 
{ char name[32];
  char projcode[16];
  struct timeval time;         // Timestamp at beginning of scan
} ObsInfoType;

enum {MaxElements=264};
enum {Tracking, Transit, ObsModes};
char ObsModeDesc[ObsModes][80] = 
   { [Tracking] = "Tracking",
     [Transit ] = "Transit"
   };
enum {FullMod, HalfMod, Comb4Way, ArrModes};
char ArrModeDesc[ArrModes][80] = 
   { [FullMod] = "Full Module",
     [HalfMod] = "Half Module",
     [Comb4Way] = "4Way Comb."
   };

float SensorSpace[ArrModes] = 
   { [FullMod] = 23.0, 
     [HalfMod] = 11.5,
     [Comb4Way] = 1.9167
   };

#define DEG2RAD M_PI/180.
// Information specific to the ORT configuration during the observation
#define C 299792458            //m/s, speed of light

#if 0 // NOTE: subloph.c still needs this layout!
// NOTE: This is the way existing code at ORT lays out modules; generally, 
/pkts2meta = / everything is referenced to module N01 (delay calc., phasing ... )
enum {N01, N02, N03, N04, N05, N06, N07, N08, N09, N10, N11, S01, S02, S03, 
      S04, S05, S06, S07, S08, S09, S10, S11, Modules};
char Module_Names [Modules][80] = { "N01", "N02", "N03", "N04", "N05", "N06", 
                                    "N07", "N08", "N09", "N10", "N11", "S01", 
                                    "S02", "S03", "S04", "S05", "S06", "S07",
                                    "S08", "S09", "S10", "S11"};
#endif 

// Half module layout
enum {N11N, N11S, N10N, N10S, N09N, N09S, N08N, N08S, N07N, N07S, N06N, N06S, 
      N05N, N05S, N04N, N04S, N03N, N03S, N02N, N02S, N01N, N01S, S01N, S01S, 
      S02N, S02S, S03N, S03S, S04N, S04S, S05N, S05S, S06N, S06S, S07N, S07S, 
      S08N, S08S, S09N, S09S, S10N, S10S, S11N, S11S, UnconnectedHM, 
      HalfModules};
char HalfModule_Names [HalfModules][80] =
     { [N11N]="N11N", [N11S]="N11S", [N10N]="N10N", [N10S]="N10S",
       [N09N]="N09N", [N09S]="N09S", [N08N]="N08N", [N08S]="N08S", 
       [N07N]="N07N", [N07S]="N07S", [N06N]="N06N", [N06S]="N06S", 
       [N05N]="N05N", [N05S]="N05S", [N04N]="N04N", [N04S]="N04S", 
       [N03N]="N03N", [N03S]="N03S", [N02N]="N02N", [N02S]="N02S", 
       [N01N]="N01N", [N01S]="N01S", [S01N]="S01N", [S01S]="S01S", 
       [S02N]="S02N", [S02S]="S02S", [S03N]="S03N", [S03S]="S03S", 
       [S04N]="S04N", [S04S]="S04S", [S05N]="S05N", [S05S]="S05S",
       [S06N]="S06N", [S06S]="S06S", [S07N]="S07N", [S07S]="S07S", 
       [S08N]="S08N", [S08S]="S08S", [S09N]="S09N", [S09S]="S09S",
       [S10N]="S10N", [S10S]="S10S", [S11N]="S11N", [S11S]="S11S",
       [UnconnectedHM]="UnconnectedHM"};

// Full module layout
enum {N11, N10, N09, N08, N07, N06, N05, N04, N03, N02, N01, S01, S02, S03, 
      S04, S05, S06, S07, S08, S09, S10, S11, UnconnectedFM, Modules};
char Module_Names [Modules][80] = 
     { [N11]="N11", [N10]="N10", [N09]="N09", [N08]="N08", [N07]="N07",
       [N06]="N06", [N05]="N05", [N04]="N04", [N03]="N03", [N02]="N02", 
       [N01]="N01", [S01]="S01", [S02]="S02", [S03]="S03", [S04]="S04", 
       [S05]="S05", [S06]="S06", [S07]="S07", [S08]="S08", [S09]="S09", 
       [S10]="S10", [S11]="S11", [UnconnectedFM]="UnconnectedFM"};

// NOTE: This structure is filled by reading the fields made available 
// over the network, along with the ORT Antenna position
enum {Time2Meta=5};            // Time after which a MetaPkt should be inserted
                               // into the recorded packet stream
typedef struct 
{ int lsthr, lstmin, lstsec;   // Current LST,should match localtime recorded
  int mjd;                     // Modified Julian date 
} AstroTimeType;

typedef struct
{ int hr, min, sec;            // Current Hour Angle of ORT antenna
} ORTPosType;

// The only time varying structure, which records all dynamic information
typedef struct
{ struct tm t_gps;             // GPS time
  AstroTimeType t_ast;         // Records LST/MJD
  ORTPosType pos;              // Records the curent Hour Angle position of ORT
  struct timeval t_pc;         // Acquisition PC's time
} ObsTimeType;

typedef struct
{ float rf, rfbw;              // Recording RF center freq. and bandwidth
  int obsmode;                 // Tracking or transit
  int arrmode;                 // Fullmod/Halfmod/4Waycomb. output, ...
  int elements; 
  char mod2chan [MaxElements]; // Map of sensor elements to bitstream position
                               // mod2chan[i] = j => At column i, module j with
                               // name Module_Names[j] is connected.
} ORTInfoType;

typedef struct
{ float f_samp;                // Sampling clk used in MHz.
  unsigned char FPGAVER;       // Fields from StatPktType
  unsigned char ability_state; 
  unsigned short appdef;
  FPGADstType destaddr[4];     // Destinations to which this DAS was sending
} DASInfoType; 

typedef struct
{ char recprog[24];            // The recording program details (28 for align)
  unsigned int pkts2scan;      // How many packets before the next scan starts
  short links2rec;             // The total GigE links in a rec. session
  short selflink;              // This MetaHdrType goes with rec. of Which link
} RecInfoType;

enum {SRCNAME, SRCRA, SRCDEC, OBSNAME, PROJCODE, ARRMODE, RF, RFBW, OBSMODE, 
      HOSTNAME, DAS2PILLAR, MOD2CHANMAP, SERVER, FSAMP, PKTS2SCAN, FILEPRE, 
LINKS2REC, LINKINFO, REFPILLAR, PILLARDEL, PILLARPH,
      CONFKEYWORDS};
char ConfKeyWords[CONFKEYWORDS][32] = 
     { [SRCNAME] = "SRCNAME",   [SRCRA]    = "SRCRA", [SRCDEC] = "SRCDEC",
       [OBSNAME] = "OBSNAME",   [PROJCODE] = "PROJCODE",
       [ARRMODE] = "ARRMODE",   [RF]       = "RF",    [RFBW]   = "RFBW", 
       [OBSMODE] = "OBSMODE",   [HOSTNAME] = "HOSTNAME", [DAS2PILLAR] = "DAS2PILLAR",
       [MOD2CHANMAP] = "MOD2CHANMAP", [SERVER] = "SERVER", [FSAMP]     = "FSAMP", 
       [PKTS2SCAN] = "PKTS2SCAN",     [FILEPRE]   = "FILEPRE", 
       [LINKS2REC] = "LINKS2REC",     [LINKINFO]  = "LINKINFO", 
       [REFPILLAR] = "REFPILLAR",     [PILLARDEL] = "PILLARDEL", 
       [PILLARPH]  = "PILLARPH"};

enum {CALSRC, CALTS, OBSTS, ANT, BLINE, NBLSOL, CALKEYWORDS};
char CalKeyWords[CALKEYWORDS][32] = 
     { [CALSRC] = "CALSRC", [CALTS] = "CALTS", [OBSTS] = "OBSTS", [ANT] = "ANT",
       [BLINE] = "BLINE", [NBLSOL] = "NBLSOL"};

// Metainformation Header
typedef struct
{ unsigned char srcid;         // First 8 bytes are common to all headers
  unsigned int  datatype: 4;
  unsigned int  bits2pix: 4;
  unsigned int  chans   : 4;
  unsigned int  words   :12;   // NOTE: size of pkt including hdr in 8byte units
  unsigned int  tick;
  int pkts2meta;               // Data packets to the next MetaPktType
  SrcInfoType src;             // Source information
  ObsInfoType obs;             // Observer information
  ORTInfoType ort;             // Telescope information
  DASInfoType das;             // Data acquisition system information
  RecInfoType rec;             // Recording system information
  ObsTimeType t_obs;           // All dynamical information available here
} MetaPktType;

char Month[][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// enum {N08, N03, S03, S08, Ph1Stations};
enum {Box0, Box1, Box2, Box3, V5DasBoxes};
char BoxNames [V5DasBoxes][6] = 
   {[Box0] = "A.272", [Box1] = "A.273", [Box2] = "A.274", [Box3] = "A.275"};
char Box2Srcid [V5DasBoxes] = 
   // NOTE: Actual srcids are these, but due to segmentation by the bridge card
   // they lose the MSB 3 bits, to become these.
   // {[Box0] = 0x50, [Box1] = 0x51, [Box2] = 0x52, [Box3] = 0x53};
   {[Box0] = 0x10, [Box1] = 0x11, [Box2] = 0x12, [Box3] = 0x13};

// This is the default 12-Chan DAS box assignment, and can be changed by
// specifying a different mapping in das.conf
unsigned char Box2Station [V5DasBoxes] = 
   {[Box0]=S03, [Box1]=N03, [Box2]=N08, [Box3]=S08};

// Structure to hold the full second's worth of raw and correlated data
// This forms a single output record written to file.
typedef struct 
{ int sec;                  // Second value since beginning of session 
  int samps2sec;            // Actual number of samples per record
  LongTick tick;            // 64bit TSC value of initial sample in the second
  struct timeval t;         // Computed PC time for this one-sec data stretch
  int miss_samp;            // The number of missing samples in this 1sec.
  int samps2blk;            
  int blks2sec; 
  int size;                 // Size of this record in bytes
  int chan;                 // Record holds a pair of channels, specify fsk chan
  unsigned char raw[0];// Actual extracted raw data
} FSKRecType;

/*
char Ph1StationNames [][Ph1Stations] =  
   {[N08] = "N08", [N03] = "N03", 
    [S03] = "S03", [S08] = "S08"};
*/
#endif // _HDR_H
