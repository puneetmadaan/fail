set(PLUGIN_NAME signalgenerator)

set(MY_PLUGIN_SRCS
  SignalGenerator.cc
  SignalGenerator.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
