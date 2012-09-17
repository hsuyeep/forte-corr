static unsigned char AcqVersion = 0x04 ; // initial release, 4-bit samples

enum {DataSrv=0xd0ff, StatSrv=0xf0ff, CmdSrv= 0xc0ff };

enum {MaxFrameBytes = 255*16};
typedef struct
{ unsigned char card,scan ;
 unsigned char srcid   ;
 unsigned char version ; // = AcqVersion
 unsigned char obsmode ; // 0x05, normal 5 streams per set
 unsigned char framesize ; // in 16B (128-bit) units
 unsigned int RefTime  ;    // 32bit Time Offset in seconds
} __attribute__((packed)) AcqHdrType;   // 64 bits


typedef struct
{ unsigned int id    :4;    // Intermediate layer source id
 unsigned int seq   :12;   // Layer specific sequence number for this pkt
} __attribute__ ((packed)) LayerHdrType; //16 bits


typedef struct { unsigned char cmd, status ;
                unsigned int seq ; // same for all 5 frames in a set
                unsigned int frame_id : 4 ; // each set cycles thru
0,1,2,3,4
                unsigned int tick0 : 12 ; // maximum of 4k samples in a
frame
              } FrameTagType ;

typedef struct
{ LayerHdrType host, pooler, bridge ; // 6B static, defined by controller
 AcqHdrType acqhdr; // 10 B  static, defined by controller
 FrameTagType tag ; // 8B  , to update dynamically
 FrameTagType tag_copy ; // 8 B , copy of previous 8 bytes
} __attribute__ ((packed)) MetaDataType ; //32 B total

/* FPGA :  info[127 downto 0] : from control card - static, frame-independent
*         tag[63 downto 48] : cmd, status - static, fpga generated at
command
*         tag[47 downto 16] : frame_seq, updated once every 5 frames
*         tag[15 downto 12] : frame_id (cycles thru 0,1,2,3,4 ,
0,1,2,3,4, ....
*         tag[11 downto 0] : clock (12 lsb) at first frame-strobe from a
chip
*
*         FrameHeader[255 downto 0] <= info & tag & tag
*/
