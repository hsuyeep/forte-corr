                    F . O . R . T . E    C . O . R . R
                    ----------------------------------
- forte-corr: Software based correlator for the Phase-1 ORT project.
- Initial commit: pep/17Sep12

- DEPENDENCIES: 
  - fftw3, compiled for float operations (use ./configure --enable-float)
    Tested with fftw-3.3.2.
  - To set the location of the FFT libraries in cmake: 
    mkdir build; cd build; cmake ../; 
    ccmake ../
    <Press 't' for advanced mode, then set path to fftw include and libraries 
     in variables FFTW_INCLUDE_DIRS and FFTW_LIBRARIES>


- BUILD INSTRUCTIONS:
  - The build is based on cmake (>2.6).
  - cd forte-corr; mkdir build;
  - [build]$ cmake ../
  - make
  - NOTE: To compile for debugging: cmake ../ -DCMAKE_BUILD_TYPE=Debug
                           release: cmake ../ -DCMAKE_BUILD_TYPE=Release
 
- The binaries are built in bin/.

- INSTRUCTIONS FOR RUNNING:
  - The receiver for data streaming over 2 GigE links is --> ioproc. It creates
    multiple shared memory regions, and maintains a registry of regions as they
    are filled.
  - The reader of this memory is -->cfx, which also implements a C-based 
    correlator. The output of this program can be either ASCII text of the 
    correlations, or a binary dump to disk.

- OUTPUT :
  - The ioproc process prints region write statistics on stderr.
  - The cfx process prints region read statistics on stderr, and generates 
    a binary file with filename corresponding to the time of run 
    (hhmmss_ddMonyy).

- TESTING:
  - For testing, ioproc can be compiled with USE_FILE=1, and then it looks for
    a raw data file "testraw.dat" from which to fill a set worth of data, and 
    subsequently fill the entire memory buffer available. This allows 
    initialization of memory regions with known patterns, for debugging.
    NOTE: The program test/filerawsta.c can be used to generate different
    kinds of patterns of raw data, suitably formatted.

  - For testing the GigE portion, the program test/netrawsta.c can be used. This
    currently creates raw data frames filled with random data, and sends data
    from 20 half modules (half the telescope) on one GigE link, and the other
    20 half modules on another GigE link.

- CONFIGURATION:
  - The number and size of the memory partitions created by ioproc can be 
    currently set in the header file include/datalayout.h: 
     STA2Region (currently 10) and Regions2Part (currently 5).
