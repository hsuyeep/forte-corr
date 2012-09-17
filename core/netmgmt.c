/* Library functions for network management.
 * pep/22Aug12
 */

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../include/netmgmt.h"
#include "../include/memmgmt.h"

/* Signal handler, e.g., for SIGINT. Assures proper closing of network 
 * interfaces, still to be done by the main thread.
 */
int NetDone = 0; // NOTE: Global defined here, only declared in header, in order
                 // to prevent multiple definition errors.
void net_sig_hdlr (int dummy)
{ fprintf (stderr, "mem_sig_hdlr: Terminating memory partitions.\n");
  NetDone = 1;

  return;
}

int init_data_link (LinkInfoType *linfo)
{ struct addrinfo hints, *p;
  int rv = 0;

  if (linfo == NULL) return -1;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  sprintf (linfo->portname, "%d", DataSrv);
  if ((rv=getaddrinfo (linfo->linkname, linfo->portname, &hints, 
                      &linfo->servinfo)) != 0) 
  { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  for(p = linfo->servinfo; p != NULL; p = p->ai_next) 
  { if ((linfo->sock = socket(p->ai_family, p->ai_socktype,
         p->ai_protocol)) == -1) 
    { perror("socket");
      continue;
    }
    break;
  }

  return 0; 
}


/* Function to bind onto the socket specified in linfo
 */
int bind_data_link (LinkInfoType *linfo)
{ if (linfo == NULL) return -1;
  int opt = 1;
  struct addrinfo *p;

  for(p = linfo->servinfo; p != NULL; p = p->ai_next) 
  { if (setsockopt (linfo->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) 
        < 0)
    { perror ("setsockopt"); }  // Not bad enough to quit..
    if (bind(linfo->sock, p->ai_addr, p->ai_addrlen) == -1) 
    { close(linfo->sock);
      perror("bind");
      continue;
    }
    break;
  }
  fprintf (stderr, "# Bound on data port %s.\n", 
           linfo->portname);

  return 0;
}

// Tear down the ethernet connection.
int close_data_link (LinkInfoType *linfo)
{ if (linfo == NULL) return -1;
  freeaddrinfo(linfo->servinfo);
  return 0;
}


/* Function to receive packets from a pair of GigE links and construct 
 * a set out of them consisting of frames from all antennas.
 * NOTE: Currently, the function simply receives consecutive frames
 * from two links, and places them in the same order in the variable set.
 */
int recv_link_pair (LinkInfoType *linfo0, LinkInfoType *linfo1, 
                    unsigned char *set)
{ if (linfo0 == NULL || linfo1 == NULL || set == NULL) return -1;
  int recvbytes = 0;
  int i = 0;
  RawFrameType *frame = (RawFrameType*) set;

  for (i=0; i<Frames2Set/2; i++)
  { if ((recvbytes=recvfrom (linfo0->sock, frame+i, sizeof(RawFrameType), 0, NULL, NULL)) 
         == -1) 
    { perror("recvfrom"); break; }
  
    if ((recvbytes=recvfrom (linfo1->sock, frame+i+Frames2Set/2, sizeof(RawFrameType), 0, 
        NULL, NULL)) == -1) 
    { perror("recvfrom"); break; }
  }
  // print_frame_meta ((RawFrameType*)set, stderr);
  // print_frame_meta ((RawFrameType*)(set+Frames2Set/2*sizeof(RawFrameType)), stderr);
  
  return 0;
}
