# This library is deprecated!
# Please use [libadm](https://github.com/IRT-Open-Source/libadm) in any future implementations.

--------------------------------------------------------------------------------
## Purpose

Audio Definition Model handling library

## Dependencies

bbcat-base - `git@git0.rd.bbc.co.uk:aplibs/bbcat-base.git`

bbcat-dsp - `git@git0.rd.bbc.co.uk:aplibs/bbcat-dsp.git`

jsoncpp - https://github.com/open-source-parsers/jsoncpp

## Files
```
autogen.sh - simple autotools script
configure.ac - configure configuration for autotools
CMakeLists.txt - CMake configuration
COPYING - information on copying this library
debian/ - Debian control and version information
doxygen.am - Doxygen automake file
doxygen.cfg - Doxygen configuration
m4 - folder for autotools
Makefile.am - makefile for automake
README - this file
src/ - source folder containing C/C++ source and header files

src/ADMData.cpp                         | Base class for ADM data handlers
src/ADMData.h                           |

src/ADMObjects.cpp                      | Individual component ADM objects
src/ADMObjects.h                        |

src/ADMSubObjects.cpp                   | Objects representing complex ADM elements
src/ADMSubObjects.h                     |

src/CMakeLists.txt                      | CMake configuration for source files

src/Makefile.am                         | Makefile for automake

src/ADMXMLGenerator.cpp                 | A simple XML generator for ADM data
src/ADMXMLGenerator.h                   |

src/AudioObjectCursor.h                 | A cursor that tracks channel parameter changes over time

src/AudioObjectParameters.cpp           | A set of parameters describing a channel's audio attributes (such as position, size, diffuseness, gain and importance)
src/AudioObjectParameters.h             |

src/XMLValue.cpp                        | A simple XML value/attribute/value with attribute class
src/XMLValue.h                          |

src/register.cpp                        | Registration function (see below)
```
--------------------------------------------------------------------------------
## Initialising the Library (IMPORTANT!)

Compilers are clever and try to remove unused code whilst linking applications.
However, sometimes they are too clever and remove some code that *appears* not
to be used but is.  Uses of the SelfRegisteringParametricObject class may
particularly suffer from this as it appears as though nothing is using the
code.

To stop this, each library includes a register.cpp file which explicitly calls a
set of initilisation and registration functions. This file also calls the
registration functions of any libraries it is dependant upon.

For this to work, any application *must* call the registration function of most
dependant library it uses.

For example, for an application using only this library:
```
#include <bbcat-adm/register.h>

using namespace bbcat;

int main(int argc, char *argv[])
{
  // ensure libraries are set up
  bbcat_register_bbcat_adm();
}
```

register.cpp is included in repo but will be updated by the script
bbcat-common/findregisterfunctions.sh if autotools is used.  The CMake process
is also capable of autogenerating this file if the original in the source
directory does not exist.

--------------------------------------------------------------------------------

## Building on Windows (Visual Studio)

Follow the installation instructions for bbcat-base (https://github.com/bbcrd/bbcat-base)

Use git-bash to change to the directory where the libraries are to be cloned to

Clone source code, if necessary:
`git clone git@github.com:bbcrd/bbcat-adm.git`
```
cd bbcat-adm
mkdir build
cd build
cmake -G "Visual Studio 14 2015 Win64" .. && cmake --build . --target INSTALL --config Release
```
(or whatever version of Visual Studio you are using)

Notes on Windows builds

As there is no standardised directories for cmake files, libraries, etc. the build *assumes* that:
1. Library includes, libs and shared files will be stored in c:\local\bbcat
2. CMake configuration files will be stored in c:\local\cmake

--------------------------------------------------------------------------------

## Building on Mac and Linux

There are two build mechanisms supported: autotools and cmake

autotools:

`./autogen.sh && configure && make && sudo make install`

cmake:

`mkdir build ; cd build ; cmake .. && make && sudo make install`

--------------------------------------------------------------------------------

##Building without cmake or autotools

Using the libraries with other build environments is possible, simply throw the
files into the build environment BUT certain defines must be set to enable
features.

bbcat-adm uses the following defines:
```
ENABLE_JSON=1 		 - enables json support
OLD_JSON_CPP  		 - define if old version of jsoncpp (version 0.6.0, for example) used
_FILE_OFFSET_BITS=64 - enable 64-bit file operations
INSTALL_PREFIX=...   - locations of installation (e.g. /usr/local, c:/local, etc)
USE_PTHREADS         - define if using pthreads rather than std::thread
```
--------------------------------------------------------------------------------
