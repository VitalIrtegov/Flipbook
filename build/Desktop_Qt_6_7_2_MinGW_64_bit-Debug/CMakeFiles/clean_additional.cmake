# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Flipbook_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Flipbook_autogen.dir\\ParseCache.txt"
  "Flipbook_autogen"
  )
endif()
