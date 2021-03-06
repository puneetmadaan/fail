# Find FTDI library
# Defines:
#  FTDI_FOUND
#  FTDI_INCLUDE_DIR
#  FTDI_LIBRARY
#

FIND_PATH(FTDI_INCLUDE_DIR ftdi.h)

FIND_LIBRARY(FTDI_LIBRARY NAMES ftdi
	PATHS /usr/lib /usr/local/lib
	ENV LIBRARY_PATH   # PATH and LIB will also work
	ENV LD_LIBRARY_PATH
)

# handle the QUIETLY and REQUIRED arguments and set FTDI_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FTDI DEFAULT_MSG FTDI_LIBRARY FTDI_INCLUDE_DIR)

MARK_AS_ADVANCED(FTDI_INCLUDE_DIR FTDI_LIBRARY)
unset(FTDI_DIR CACHE)
