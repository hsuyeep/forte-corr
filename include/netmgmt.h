/* Header file for network management operations.
 * pep/22Aug12
 */
#ifndef __NETMGMT_H__
#define __NETMGMT_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

extern int NetDone;
// UDP Ports for accessing various services.
enum {DataSrv=0xd0ff, StatSrv=0xf0ff, CmdSrv= 0xc0ff };

typedef struct StLinkInfoType
{ char linkname[16];     // Name as in /etc/hosts, or IP addr as string.
  char portname[16];     // Also a string!
  int sock;              // socketid afer link setup.
  struct addrinfo *servinfo; // memory is allocated by getaddrinfo ().
} LinkInfoType;

// Function prototypes
void net_sig_hdlr (int dummy);
int bind_data_link  (LinkInfoType *);
int init_data_link  (LinkInfoType *);
int close_data_link (LinkInfoType *);
int recv_link_pair (LinkInfoType *, LinkInfoType *, unsigned char *);
#endif // __NETMGMT_H__
