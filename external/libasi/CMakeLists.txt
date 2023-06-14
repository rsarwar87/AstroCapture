cmake_minimum_required (VERSION 3.0)
project (libasi)

# Using ASI Camera SDK Version 1.29 updated on 2023-05-03
# Using ASI EFW SDK Version 1.7 updated on 2021-05-17
# Using ASI ST4 SDK Version 1.0 updated on 2018-07-23
# Using ASI EAF SDK Version 1.6 updated on 2023-03-16

set (ASICAM_VERSION "1.29")
set (ASICAM_SOVERSION "1")

set (ASIEFW_VERSION "1.7")
set (ASIEFW_SOVERSION "1")

set (ASIST4_VERSION "1.0")
set (ASIST4_SOVERSION "1")

set (ASIEAF_VERSION "1.6")
set (ASIEAF_SOVERSION "1")
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake_module/")
include (GNUInstallDirs)
include (InstallImported)


add_library (ASICamera  STATIC IMPORTED)

set_target_properties (ASICamera  PROPERTIES VERSION ${ASICAM_VERSION} SOVERSION ${ASICAM_SOVERSION})

if (UNIX AND NOT WIN32)

  if (CMAKE_SYSTEM_PROCESSOR MATCHES "armv+")
    set_property (TARGET ASICamera  PROPERTY IMPORTED_LOCATION "armv6/libASICamera2.bin")
  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set_property (TARGET ASICamera  PROPERTY IMPORTED_LOCATION "armv8/libASICamera2.bin")
  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set_property (TARGET ASICamera  PROPERTY IMPORTED_LOCATION "x64/libASICamera2.bin")
  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86")
    set_property (TARGET ASICamera  PROPERTY IMPORTED_LOCATION "x86/libASICamera2.bin")
  endif ()

endif ()
#
install (
  FILES ASICamera2.h 
  DESTINATION include/libasi
)

# Install library
install_imported (
  TARGETS ASICamera 
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
