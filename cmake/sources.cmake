# === sourcefiles *.{c,h}

set(IOPROC_SOURCES
	core/ioproc.c
	core/memmgmt.c
#	core/netmgmt.c
	include/datalayout.h
	include/memmgmt.h
	include/netmgmt.h
)

set(CFX_SOURCES
	core/cfx.c 
	core/corrops.c
	core/memmgmt.c
	core/netmgmt.c
	include/corrops.h
	include/datalayout.h
	include/memmgmt.h
 )

#	include/ffthdr.h
#	core/fftw_shuffle.c
#	core/filerawsta.c
#	core/frameops.c
#	core/frameprint.c
#	core/netrawsta.c
#	core/shmfiledump.c
#	core/shmrd.c
#	core/xmac_set.c
