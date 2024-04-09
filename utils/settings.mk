# Libraries
LIBS=commons pthread readline m

# Custom libraries' paths
STATIC_LIBPATHS=utils/src/utils

# Compiler flags
CDEBUG=-g -Wall -DDEBUG -fdiagnostics-color=always
CRELEASE=-O3 -Wall -DNDEBUG -fcommon
