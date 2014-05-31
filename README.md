Welcome to HIntLib!
===================

HIntLib is a C++ library for high-dimensional numerical integration.

All the documentation on HIntLib is contained in the manual, which can be
found as manual.pdf in the same directory as README.

Appendix A of the manual deals with building and installing HIntLib.

The official website can be found at http://mint.sbg.ac.at/HIntLib/

Specific instructions for Debian-based distributions
===================

To install this development version from this repository, use the following chain:

 - `./autogen.sh`
 - `./configure --enable-static --prefix=/usr
 - `make -j 4`
 - `sudo checkinstall` (at this point make sure that the name and the version are detected automatically; if not, edit them to be `hintlib` and `0.0.13-github`, correspondingly)
 - `sudo ldconfig`
 
To install with MPI enabled, add the appropriate flag:
 - `./configure --enable-static --enable-shared MPI_HEADER_PATH=/path/to/mpi.h --prefix=/usr
 
For instance, with these packages installed

`sudo apt-get install libcr-dev mpich2 mpich2-doc checkinstall`

the correct path will be `MPI_HEADER_PATH=/usr/lib/mpich2/include`.
